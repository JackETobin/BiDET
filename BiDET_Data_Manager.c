/*
*	DATA STRUCTURE IN MEM: BOTTOM -> TOP
*	----DATA BOTTOM----
*
*	-----KEY NAME------
*	----NAME LENGTH----	<-- (uint32_bd)
*	-----DATA SIZE----- <--DATA PTR IN KEY (uint64_bd)
*	----DATA ITSELF----
*
*	-----DATA TOP------
*/

#include "BiDET.h"

/*
* --------------------TODO--------------------
* DECIGN A VOID COMPACTION SYSTEM
* 
* COMPACT-> NEEDS TO CHECK FOR VOIDS, SHIFT DATA
* IN MEM, GET ALL KEYS ASOCIATED WITH THE DATA,
* SHIFT POINTERS IN KEYS TO THEIR CORRESPONDING
* DATA'S NEW LOCATION, AND REMOVE ALL VOID KEYS
* POINTING TO THE VOIDS THAT WERE FILLED.
* 
* TEST Reserve()
* 
* 2/9/24 TIMES:
* LIST LENGTH:		1000 KEYS
* STORE() TIME:		0.088300MS
* GET() TIME:		0.054300MS
* TOTAL TIME:		0.142600MS
*/

#define BD_INT32SIZE sizeof(uint32_bd)
#define BD_KEYSIZE sizeof(key_bd)

#define BD_DATAMINLEN 1
#define BD_NAMEMINLEN 1
#define BD_DATAMINSIZE BD_INT32SIZE + bd_buffSize + BD_NAMEMINLEN + BD_DATAMINLEN

static stash_bd* Stash;

stash_bd**
BiDET_Retrieve_Stash()
{ return &Stash; }


static inline int32_bd
Name_Check(char* inKeyName, char* inStoreName)
{
	uint32_bd storedLen = *(uint32_bd*)inStoreName;
	inStoreName -= storedLen;
	while (*inKeyName == *inStoreName && storedLen)
	{
		inKeyName++; inStoreName++; storedLen--;
	}
	return (*inKeyName == storedLen);
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


static inline uint64_bd 
Hash(char* keyName, uint32_bd* pKeyLen)
{
	uint32_bd keyLen = 0;
	uint64_bd hashVal = 1;
	while (*keyName != '\0')
	{
		hashVal += *keyName * (*keyName - *(keyName + 1));
		keyName++; keyLen++;
	} 
	*pKeyLen = keyLen;
	return hashVal;
}


static inline key_bd*
Border_Index(uint64_bd hash)
{
	key_bd* keyRing = Stash->keyRing;
	int64_bd numKeys = Stash->numKeys;
	return (hash <= keyRing->hash) ? keyRing : (keyRing - numKeys);
}


static inline key_bd*
Binary_Index(uint64_bd hash)
{
	int64_bd numKeys = Stash->numKeys;
	key_bd* keyRing = Stash->keyRing;
	key_bd* pKey = keyRing - (numKeys >> 1);

	int32_bd positionCheck =	(pKey - 1 > keyRing - numKeys) *
								((hash < (pKey + 1)->hash) -
								(hash >= (pKey - 1)->hash));
	uint32_bd i = 0;
	while (positionCheck)
	{
		i += (numKeys >> (i + 2) > 0);
		pKey += positionCheck * (numKeys >> (i + 1));
		positionCheck = (hash < (pKey + 1)->hash) -
						(hash >= (pKey - 1)->hash);
	}
	return pKey;
}


static inline key_bd*
Index(uint64_bd hash)
{
	key_bd* pKey = Stash->keyRing;
	uint32_bd border = (hash <= pKey->hash ||
					 hash >= (pKey - (Stash->numKeys - 1))->hash);
	return border ? Border_Index(hash) : Binary_Index(hash);
}


static void
Insert_Key(key_bd* pTarget, void* nextEntry, uint64_bd hash)
{
	key_bd* pKey = Stash->keyRing - Stash->numKeys;
	pTarget -= (pTarget->hash < hash);

	while (pKey < pTarget)
	{
		*pKey = *(pKey + 1);
		pKey++;
	}
	
	*pKey = (key_bd){ hash, nextEntry };
	Stash->numKeys++;
	return;
}


static inline uint32_bd
Check_Duplicate_Key_Name(char* keyName, key_bd** pTarget, const uint64_bd hash)
{
	key_bd* pKey = *pTarget;
	pKey += (pKey->hash != hash);

	uint32_bd duplicate = 0;
	do {
		duplicate = Name_Check(keyName, pKey->data - BD_INT32SIZE);
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
	*((bd_buffType*)entry) = dataSize;
	entry += bd_buffSize;

	while (dataSize--)
		*entry++ = *dataIn++;

	*ppTarget = (void*)entry;
	return;
}


static inline void*
Store_Key_Name(char* keyName, void* pTarget)
{
	char* pReturn = (char*)pTarget;
	while (*keyName != '\0')
		*pReturn++ = *keyName++;

	*(uint32_bd*)pReturn = pReturn - (char*)pTarget;
	pReturn += BD_INT32SIZE;
	return (void*)pReturn;
}


static inline bd_buffType
Void_Shrink(key_bd* pVoidKey, uint64_bd shiftSize, bd_buffType newSize)
{
	pVoidKey->data += shiftSize;
	*(bd_buffType*)pVoidKey->data = newSize;
	return 0;
}


static inline bd_buffType
Void_Fill(key_bd* pVoidKey, bd_buffType sizeRemaining)
{
	key_bd* pLimit = Stash->keyRing - (Stash->numKeys - 1);
	Remove_Key(pVoidKey, pLimit);
	return sizeRemaining;
}


static inline key_bd*
Check_Void(uint64_bd totalSize)
{
	key_bd* pVoidKey = Stash->keyRing - (Stash->numKeys - 1);
	uint32_bd spaceFound = 0;
	while (pVoidKey->hash == BD_HASHMAX && !spaceFound)
	{
		spaceFound = (*(bd_buffType*)(pVoidKey->data) >= totalSize);
		pVoidKey -= (!spaceFound);
	}
	return (spaceFound) ? pVoidKey : BD_NULL;
}


static inline void*
Consolidate_Void(key_bd* pVoidKey, uint64_bd sizeRequired, bd_buffType* dataSize)
{
	void* writeTarget = pVoidKey->data;
	bd_buffType sizeDiff = *(bd_buffType*)pVoidKey->data - sizeRequired;
	*dataSize += (sizeDiff < BD_DATAMINSIZE) ?
		Void_Fill(pVoidKey, sizeDiff) : Void_Shrink(pVoidKey, sizeRequired, sizeDiff);
	return writeTarget;
}

static inline void
Commit_Store
(	char*		keyName,		key_bd*			pKey, 
	uint64_bd	sizeRequired,	uint64_bd		hash, 
	void*		dataIn,			bd_buffType		dataSize)
{
	key_bd* pVoidKey = Check_Void(sizeRequired);
	void** writeTarget = (pVoidKey) ? 
		&(void*) { Consolidate_Void(pVoidKey, sizeRequired, &dataSize) } : 
		&Stash->nextEntry;

	*writeTarget = (char*)Store_Key_Name(keyName, (char*)*writeTarget);
	Insert_Key(pKey, *writeTarget, hash);
	Store_Data(writeTarget, dataIn, dataSize);
	return;
}


static inline uint64_bd Size_Remaining()
{
	uint64_bd sizeRemaining = (void*)(Stash->keyRing - (Stash->numKeys - 1)) - Stash->nextEntry;
	return sizeRemaining;
}


static inline void
BiDET_Store(char* keyName, void* dataIn, uint64_bd* dataSize, void** callID)
{
	uint32_bd keyLen = 0;
	uint64_bd hash = Hash(keyName, &keyLen);
	uint64_bd sizeRequired = keyLen + BD_INT32SIZE + bd_buffSize + *dataSize;
	uint32_bd inadequateSize = (Size_Remaining() < sizeRequired + BD_KEYSIZE);

	key_bd* pKey = Index(hash);
	uint32_bd duplicate = Check_Unique_Hash(keyName, &pKey, hash) * (!inadequateSize);

	ERR_CATCH((duplicate * DUPLICATEKY) + (inadequateSize * SPACENEEDED), callID) ? 
		Commit_Store(keyName, pKey, sizeRequired, hash, dataIn, (bd_buffType)*dataSize) : 0;
	return;
}


static inline uint32_bd
Retrieve_Key(char* keyName, key_bd** ppKey)
{
	uint32_bd keyLen = 0;
	uint64_bd hash = Hash(keyName, &keyLen);
	*ppKey = Index(hash);
	uint32_bd unique = !Check_Unique_Hash(keyName, ppKey, hash);
	return unique;
}



static inline void
Retrieve_Data(const key_bd* pKey, void* dataOut, const uint64_bd containerSize, void* callID[])
{
	bd_buffType dataSize = *(bd_buffType*)pKey->data;
	dataSize = (ERR_CATCH((dataSize < containerSize) * CONTAINERMISSMATCH, callID)) ?
				containerSize : 0;

	char* data = pKey->data + bd_buffSize;
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
	key_bd* lVoidKey = Stash->keyRing - (Stash->numKeys - 1);
	uint32_bd lVoidKeySize = *(bd_buffType*)(lVoidKey->data);
	uint32_bd pushbackCheck = ((lVoidKey->data + lVoidKeySize) == Stash->nextEntry);

	(pushbackCheck) ? Remove_Key(lVoidKey, lVoidKey) : 0;
	Stash->nextEntry -= pushbackCheck * lVoidKeySize;
	Stash->numKeys -= pushbackCheck;

	return;
}


static inline void
Consolidate_Voids(key_bd* nVoid, key_bd* cVoid, key_bd* pLimit)
{
	*(bd_buffType*)(nVoid->data) += *(bd_buffType*)(cVoid->data);
	Remove_Key(cVoid, pLimit);
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
		((nVoidKey->data + *(bd_buffType*)(nVoidKey->data) == cVoidKey->data)) ?
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
Void_Data_In_Mem(key_bd* pTarget, bd_buffType sizeInMem)
{
	char* voidBegin = pTarget->data - BD_INT32SIZE;
	voidBegin -= *(uint32_bd*)voidBegin;
	*(bd_buffType*)voidBegin = sizeInMem;
	
	return voidBegin;
}


static inline uint64_bd 
Size_In_Mem(key_bd* pKey)
{
	char* access = pKey->data - BD_INT32SIZE;
	uint64_bd sizeInMem = *(uint32_bd*)access + BD_INT32SIZE;
	access += BD_INT32SIZE;
	sizeInMem += *(bd_buffType*)access + bd_buffSize;

	return sizeInMem;
}


static inline void
Commit_Remove(key_bd* pTarget)
{
	uint64_bd sizeInMem = Size_In_Mem(pTarget);
	void* voidBegin = Void_Data_In_Mem(pTarget, sizeInMem);
	key_bd* pLimit = Stash->keyRing - (Stash->numKeys - 1);

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


static inline void*
Build_Reservation(void** ppResTarget, uint64_bd sizeReserved)
{
	*(bd_buffType*)*ppResTarget = sizeReserved;
	void* dataPtr = *ppResTarget + bd_buffSize;
	*ppResTarget += (bd_buffSize + sizeReserved);
	return dataPtr;
}


static inline void
Comit_Reserve
(char*		keyName,		key_bd*			pKey,
uint64_bd	sizeRequired,	uint64_bd		hash,
void*		dataPtr,		bd_buffType		sizeReserved)
{
	key_bd* pVoidKey = Check_Void(sizeRequired);
	void** writeTarget = (pVoidKey) ?
		Consolidate_Void(pVoidKey, sizeRequired, &sizeReserved) : &Stash->nextEntry;

	*writeTarget = (char*)Store_Key_Name(keyName, (char*)*writeTarget);
	Insert_Key(pKey, *writeTarget, hash);
	*(void**)dataPtr = Build_Reservation(writeTarget, sizeReserved);
	
	return;
}


static inline void
BiDET_Reserve(char* keyName, void* dataPtr, uint64_bd* sizeReserved, void** callID)
{
	uint32_bd keyLen = 0;
	uint64_bd hash = Hash(keyName, &keyLen);
	uint64_bd sizeRequired = keyLen + BD_INT32SIZE + bd_buffSize + *sizeReserved;
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
	bd_buffType containerSize = *(bd_buffType*)data;
	ERR_CATCH((dataSize < *(bd_buffType*)data) * DATATRUNCATION, callID);

	data += bd_buffSize;
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
	*sizeReserved = *(bd_buffType*)pKey->data;
	*(void**)pointerIO = pKey->data + bd_buffSize;
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
{ return; }

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
}