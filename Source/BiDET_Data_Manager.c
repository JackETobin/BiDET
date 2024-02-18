/*
*	DATA STRUCTURE IN MEM: BOTTOM -> TOP
*	----DATA BOTTOM----
*
*	-----KEY NAME------
*	----NAME LENGTH----	<-- (uint32_bd)
*	-----DATA SIZE----- 	<--DATA PTR IN KEY (bd_buffType)
*	----DATA ITSELF----
*
*	-----DATA TOP------
*/

#include "BiDET.h"

#define BD_METAINFOSIZE bd_nameBuffSize + bd_dataBuffSize
#define BD_KEYSIZE sizeof(key_bd)

#define BD_DATAMINLEN 1
#define BD_NAMEMINLEN 1
#define BD_DATAMINSIZE BD_METAINFOSIZE + BD_NAMEMINLEN + BD_DATAMINLEN

static stash_bd* Stash;

stash_bd**
BiDET_Retrieve_Stash()
{ return &Stash; }


static inline uint64_bd
Hash(char* keyName, bd_nameBuffType* pKeyLen)
{
	bd_nameBuffType keyLen = 0;
	uint64_bd hashVal = 1;
	while (*keyName != '\0')
	{
		hashVal += *keyName * (*keyName - *(keyName + 1));
		keyName++; keyLen++;
	}
	*pKeyLen = keyLen + 1;
	return hashVal;
}


static inline key_bd*
Border_Index(uint64_bd hash)
{
	key_bd* pKey = Stash->lastKey;
	return (hash < pKey->hash) ? (pKey + Stash->numKeys - 1) : pKey;
}

static inline key_bd*
Binary_Index(uint64_bd hash)
{
	int64_bd keyAdjust = Stash->numKeys >> 1;
	key_bd* pKey = Stash->lastKey + keyAdjust;
	int32_bd positionCheck = (keyAdjust > 1) *
							 ((hash < (pKey + 1)->hash) -
							 (hash >= (pKey - 1)->hash));
	while (positionCheck)
	{
		keyAdjust >>= (keyAdjust > 1);
		pKey += positionCheck * keyAdjust;
		positionCheck = (hash < (pKey + 1)->hash) -
						(hash >= (pKey - 1)->hash);
	}
	return pKey;
}

static key_bd* (*Hash_Type[2])(uint64_bd) =
{ Binary_Index, Border_Index };

static inline key_bd*
Index(uint64_bd hash)
{
	uint32_bd border = (hash <= (Stash->lastKey + Stash->numKeys - 1)->hash ||
						hash >= (Stash->lastKey)->hash);
	return Hash_Type[border](hash);
}


static void
Insert_Key(key_bd* pTarget, void* nextEntry, uint64_bd hash)
{
	Stash->lastKey--;
	Stash->numKeys++;

	key_bd* pKey = Stash->lastKey;
	pTarget -= (pTarget->hash < hash);
	while (pKey < pTarget)
	{
		*pKey = *(pKey + 1);
		pKey++;
	}
	*pTarget = (key_bd){ hash, nextEntry };
	return;
}


static inline void
Remove_Key(key_bd* pTarget, key_bd* pLimit)
{
	while (pTarget > pLimit)
	{
		*pTarget = *(pTarget - 1);
		pTarget--;
	}
	*pTarget = (key_bd){ 0 };
}


static inline int32_bd
Name_Check(char* inKeyName, char* inStoreName)
{
	inStoreName -= bd_nameBuffSize;
	bd_nameBuffType storedLen = *(bd_nameBuffType*)inStoreName;
	inStoreName -= storedLen;
	uint32_bd checkLen = 0;
	while (*inKeyName == *inStoreName)
	{
		inKeyName++; inStoreName++;
		checkLen++;
	}
	return (checkLen == storedLen);
}


static inline uint32_bd
Check_Duplicate_Key_Name(char* keyName, key_bd** pTarget, const uint64_bd hash)
{
	key_bd* pKey = *pTarget;
	pKey += (pKey->hash != hash);

	uint32_bd duplicate = 0;
	do {
		duplicate = Name_Check(keyName, pKey->data);
		pKey += (!duplicate);
	} while (!duplicate && pKey->hash == hash);

	*pTarget = pKey;
	return duplicate;
}


static inline uint32_bd
Check_Unique_Hash(char* keyName, key_bd** pTarget, const uint64_bd hash)
{
	key_bd* pKey = *pTarget;
	uint32_bd duplicate = (hash == pKey->hash ||
						   hash == (pKey + 1)->hash);
	return duplicate ? Check_Duplicate_Key_Name(keyName, pTarget, hash) : 0;
}


static inline void
Store_Data(void** ppTarget, char* dataIn, uint64_bd dataSize)
{
	char* entry = *(char**)ppTarget;
	*((bd_dataBuffType*)entry) = dataSize;
	entry += bd_dataBuffSize;

	while (dataSize--)
		*entry++ = *dataIn++;

	*ppTarget = (void*)entry;
	return;
}


static inline void
Store_Key_Name(char* keyName, void** ppTarget)
{
	uint32_bd len = 0;
	char* pReturn = (char*)*ppTarget;
	
	do {
		*pReturn++ = *keyName;
		len++;
	} while (*keyName++ != '\0');

	*(bd_nameBuffType*)pReturn = len;
	pReturn += bd_nameBuffSize;
	*ppTarget = (void*)pReturn;
	return;
}


static inline bd_dataBuffType
Void_Shrink(key_bd* pVoidKey, uint64_bd shiftSize, bd_dataBuffType newSize)
{
	pVoidKey->data += shiftSize;
	*(bd_dataBuffType*)pVoidKey->data = newSize;
	return 0;
}


static inline bd_dataBuffType
Void_Fill(key_bd* pVoidKey, bd_dataBuffType sizeRemaining)
{
	Remove_Key(pVoidKey, Stash->lastKey);
	return sizeRemaining;
}


static inline key_bd*
Check_Void(uint64_bd totalSize)
{
	key_bd* pVoidKey = Stash->lastKey;
	uint32_bd spaceFound = 0;
	while (pVoidKey->hash == BD_HASHMAX && !spaceFound)
	{
		spaceFound = (*(bd_dataBuffType*)(pVoidKey->data) >= totalSize);
		pVoidKey -= (!spaceFound);
	}
	return (spaceFound) ? pVoidKey : BD_NULL;
}


static inline void*
Consolidate_Void(key_bd* pVoidKey, uint64_bd sizeRequired, bd_dataBuffType* dataSize)
{
	void* writeTarget = pVoidKey->data;
	bd_dataBuffType sizeDiff = *(bd_dataBuffType*)(pVoidKey->data - sizeRequired);
	*dataSize += (sizeDiff < BD_DATAMINSIZE) ?
		Void_Fill(pVoidKey, sizeDiff) : Void_Shrink(pVoidKey, sizeRequired, sizeDiff);
	return writeTarget;
}


static inline void
Commit_Store
(char*			keyName,		key_bd*				pKey,
uint64_bd		sizeRequired,	uint64_bd			hash,
void*			dataIn,			bd_dataBuffType		dataSize)
{
	key_bd* pVoidKey = Check_Void(sizeRequired);
	void** writeTarget = (pVoidKey) ?
		&(void*) { Consolidate_Void(pVoidKey, sizeRequired, &dataSize) } :
		&Stash->nextEntry;

	Store_Key_Name(keyName, writeTarget);
	Insert_Key(pKey, *writeTarget, hash);
	Store_Data(writeTarget, dataIn, dataSize);
	return;
}


static inline uint64_bd Size_Remaining()
{ return (void*)Stash->lastKey - Stash->nextEntry; }

static inline void
BiDET_Store(char* keyName, void* dataIn, uint64_bd* dataSize, void** callID)
{
	bd_nameBuffType keyNameLen = 0;
	uint64_bd hash = Hash(keyName, &keyNameLen);
	uint64_bd sizeRequired = keyNameLen + BD_METAINFOSIZE + *dataSize + BD_KEYSIZE;
	uint32_bd inadequateSize = (Size_Remaining() < sizeRequired);

	key_bd* pKey = Index(hash);
	uint32_bd duplicate = Check_Unique_Hash(keyName, &pKey, hash) * (!inadequateSize);

	ERR_CATCH((duplicate * DUPLICATEKY) + (inadequateSize * SPACENEEDED), callID) ?
		Commit_Store(keyName, pKey, sizeRequired, hash, dataIn, (bd_dataBuffType)*dataSize) : 0;
	return;
}


static inline uint32_bd
Retrieve_Key(char* keyName, key_bd** ppKey)
{
	bd_nameBuffType keyLen = 0;
	uint64_bd hash = Hash(keyName, &keyLen);
	*ppKey = Index(hash);
	uint32_bd unique = !Check_Unique_Hash(keyName, ppKey, hash);
	return unique;
}



static inline void
Retrieve_Data(const key_bd* pKey, void* dataOut, const uint64_bd containerSize, void* callID[])
{
	bd_dataBuffType dataSize = *(bd_dataBuffType*)pKey->data;
	dataSize = (ERR_CATCH((dataSize < containerSize) * CONTAINERMISSMATCH, callID)) ?
		containerSize : 0;

	char* data = pKey->data + bd_dataBuffSize;
	while (dataSize--)
	{
		*(char*)(dataOut) = *data;
		dataOut++; data++;
	}
	return;
}


static inline void
BiDET_Get(char* keyName, void* dataOut, uint64_bd* containerSize, void* callID[])
{
	key_bd* pKey;
	uint32_bd unique = Retrieve_Key(keyName, &pKey);
	(ERR_CATCH(unique * KEYNOTFOUND, callID)) ?
		Retrieve_Data(pKey, dataOut, *containerSize, callID) : 0;
	return;
}


static inline void
Nextentry_Pushback()
{
	key_bd* lVoidKey = Stash->lastKey;
	uint32_bd lVoidKeySize = *(bd_dataBuffType*)(lVoidKey->data);
	uint32_bd pushbackCheck = ((lVoidKey->data + lVoidKeySize) == Stash->nextEntry);

	(pushbackCheck) ? Remove_Key(lVoidKey, lVoidKey) : 0;
	Stash->nextEntry -= pushbackCheck * lVoidKeySize;
	Stash->lastKey += pushbackCheck;
	Stash->numKeys -= pushbackCheck;

	return;
}


static inline void
Consolidate_Voids(key_bd* nVoid, key_bd* cVoid, key_bd* pLimit)
{
	*(bd_dataBuffType*)(nVoid->data) += *(bd_dataBuffType*)(cVoid->data);
	Remove_Key(cVoid, pLimit);
	Stash->lastKey++;
	Stash->numKeys--;
	return;
}


static inline void
Sweep_Void_Keys(key_bd* pLimit)
{
	key_bd* cVoidKey = pLimit;
	key_bd* nVoidKey = cVoidKey + 1;
	while (nVoidKey->hash == BD_HASHMAX)
	{
		((nVoidKey->data + *(bd_dataBuffType*)(nVoidKey->data) == cVoidKey->data)) ?
			Consolidate_Voids(nVoidKey, cVoidKey, pLimit) : 0;
		cVoidKey++; nVoidKey++;
	}
}


static inline void
Insert_Void_Key(key_bd* pLimit, void* voidBegin)
{
	key_bd* pTarget = pLimit;
	while ((pTarget - 1)->hash == BD_HASHMAX && (pTarget - 1)->data < voidBegin)
	{
		*pTarget = *(pTarget - 1);
		pTarget--;
	}
	*pTarget = (key_bd){ BD_HASHMAX, voidBegin };
	Sweep_Void_Keys(pLimit);
	return;
}


static void*
Void_Data_In_Mem(key_bd* pTarget, bd_dataBuffType sizeInMem)
{
	char* voidBegin = pTarget->data - bd_nameBuffSize;
	voidBegin -= *(uint32_bd*)voidBegin;
	*(bd_dataBuffType*)voidBegin = sizeInMem;

	return voidBegin;
}


static inline uint64_bd
Size_In_Mem(key_bd* pKey)
{
	char* data = pKey->data;
	uint64_bd sizeInMem = *(bd_dataBuffType*)data + bd_dataBuffSize;
	data -= bd_nameBuffSize;
	sizeInMem += *(bd_nameBuffType*)data + bd_nameBuffSize;

	return sizeInMem;
}


static inline void
Commit_Remove(key_bd* pTarget)
{
	uint64_bd sizeInMem = Size_In_Mem(pTarget);
	void* voidBegin = Void_Data_In_Mem(pTarget, sizeInMem);
	key_bd* pLimit = Stash->lastKey;

	Remove_Key(pTarget, pLimit);
	Insert_Void_Key(pLimit, voidBegin);
	Nextentry_Pushback();

	return;
}


static inline void
BiDET_Remove(char* keyName, void* v, uint64_bd* pi, void* callID[])
{
	key_bd* pKey;
	uint32_bd unique = Retrieve_Key(keyName, &pKey);

	(ERR_CATCH(unique * KEYNOTFOUND, callID)) ?
		Commit_Remove(pKey) : 0;

	return;
}


static inline void
Build_Reservation(void** ppResTarget, void* pDataPtr, uint64_bd sizeReserved)
{
	*(bd_dataBuffType*)*ppResTarget = sizeReserved;
	void* dataPtr = *ppResTarget + bd_dataBuffSize;
	*ppResTarget += (bd_dataBuffSize + sizeReserved);
	*(void**)pDataPtr = dataPtr;
	return;
}


static inline void
Comit_Reserve
(	char*		keyName,		key_bd*			pKey,
	uint64_bd	sizeRequired,	uint64_bd		hash,
	void*		dataPtr,		bd_dataBuffType	sizeReserved)
{
	key_bd* pVoidKey = Check_Void(sizeRequired);
	void** writeTarget = (pVoidKey) ?
		Consolidate_Void(pVoidKey, sizeRequired, &sizeReserved) : &Stash->nextEntry;

	Store_Key_Name(keyName, writeTarget);
	Insert_Key(pKey, *writeTarget, hash);
	Build_Reservation(writeTarget, dataPtr, sizeReserved);

	return;
}


static inline void
BiDET_Reserve(char* keyName, void* dataPtr, uint64_bd* sizeReserved, void** callID)
{
	bd_nameBuffType keyNameLen = 0;
	uint64_bd hash = Hash(keyName, &keyNameLen);
	uint64_bd sizeRequired = keyNameLen + BD_METAINFOSIZE + *sizeReserved;
	uint32_bd inadequateSize = (Size_Remaining() < sizeRequired + BD_KEYSIZE);

	key_bd* pKey = Index(hash);
	uint32_bd duplicate = Check_Unique_Hash(keyName, &pKey, hash) * (!inadequateSize);

	ERR_CATCH((duplicate * DUPLICATEKY) + (inadequateSize * SPACENEEDED), callID) ?
		Comit_Reserve(keyName, pKey, sizeRequired, hash, dataPtr, *sizeReserved) : 0;
	return;
}


static inline void
Commit_Fill(key_bd* pKey, void* dataIn, uint64_bd dataSize, void* callID[])
{
	char* data = pKey->data;
	bd_dataBuffType containerSize = *(bd_dataBuffType*)data;
	ERR_CATCH((dataSize < *(bd_dataBuffType*)data) * DATATRUNCATION, callID);

	data += bd_dataBuffSize;
	while (containerSize--)
		*data++ = *(char*)dataIn++;
	return;
}


static inline void
BiDET_Fill(char* keyName, void* dataIn, uint64_bd* dataSize, void* callID[])
{
	key_bd* pKey;
	uint32_bd unique = Retrieve_Key(keyName, &pKey);

	(ERR_CATCH(unique * KEYNOTFOUND, callID)) ?
		Commit_Fill(pKey, dataIn, *dataSize, callID) : 0;

	return;
}


static inline void
Retrieve_Pointer(key_bd* pKey, void* pointerIO, uint64_bd* sizeReserved)
{
	*sizeReserved = *(bd_dataBuffType*)pKey->data;
	*(void**)pointerIO = pKey->data + bd_dataBuffSize;
	return;
}


static inline void
BiDET_Retrieve(char* keyName, void* pointerIO, uint64_bd* sizeReserved, void** callID)
{
	key_bd* pKey;
	uint32_bd unique = Retrieve_Key(keyName, &pKey);

	(ERR_CATCH(unique * KEYNOTFOUND, callID)) ?
		Retrieve_Pointer(pKey, pointerIO, sizeReserved) : BD_NULL;
	return;
}


static inline void
BiDET_Request_Bypass_(char* c, void* v, uint64_bd* pi, void** noID)
{
	return;
}

static void
(*_action_dispatch[7])(char*, void*, uint64_bd*, void**) = {
	BiDET_Request_Bypass_,
	BiDET_Get,
	BiDET_Store,
	BiDET_Remove,
	BiDET_Reserve,
	BiDET_Fill,
	BiDET_Retrieve
};


void
BiDET_Verify_Req(char* keyName, void* dataIO, uint64_bd* sizeInput, bd_action reqAction, void* callID[])
{
	ERR_CATCH((Stash == BD_NULL) * STASHMISSNG, callID);
	reqAction *= ERR_CATCH((keyName == BD_NULL) * INVALIDNAME, callID);

	return _action_dispatch[reqAction](keyName, dataIO, sizeInput, callID);
