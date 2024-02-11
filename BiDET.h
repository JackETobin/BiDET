#ifndef C_STORE
#define C_STORE

#define KBtoBYTE 1024
#define BD_NULL ((void*)0)

typedef long int				int32_bd;
typedef unsigned long int		uint32_bd;
typedef long long				int64_bd;
typedef unsigned long long		uint64_bd;

#ifdef BiDET_ENABLE_DEBUG
#define BiDET_ERR_ENABLE_DEBUG
#endif

#include "BiDET_Error.h"

typedef enum BiDET_Action
{
	GET = 1,
	STORE,
	REMOVE,
	RESERVE,
	FILL,
	RETRIEVE
}bd_action;

typedef enum BiDET_Stash_Info	//RETURN TYPES:
{
	SPACE_TOTAL,				//uint64_bd
	SPACE_REMAINING,			//uint64_bd
	NUM_KEYS,					//uint64_bd
	NUM_STASHES,				//uint64_bd
	HANDLE_STASH,				//stash_bd*
	HANDLE_KEYS					//key_bd*
}bd_info;

#ifdef BiDET_LARGE_BUFFER
#define bd_buffSize sizeof(uint64_bd)
#define bd_buffType uint64_bd
#else
#define bd_buffSize sizeof(uint32_bd)
#define bd_buffType uint32_bd
#endif

#define BD_HASHMAX 0xffffffffffffffffUi64
typedef struct BiDET_Key
{
	uint64_bd	hash;
	void*		data;
}key_bd;

typedef struct BiDET_Stash
{
	void*		nextStash;
	key_bd*		keyRing;
	uint64_bd	sizeByte;
	int64_bd	numKeys;
	void*		nextEntry;
}stash_bd;

void
BiDET_Exit(uint32_bd errorCode);
stash_bd**
BiDET_Retrieve_Stash();
void 
BiDET_Verify_Req(char* keyName, void* dataIO, uint64_bd* sizeIO, bd_action action, void** callID);
void
BiDET_Stash_Request(void* pInfoOut, uint32_bd stashID, bd_info infoReq);


void														BiDET_Set_Callback(void(*pCallback)(bd_errpack));
#define BD_SetErrCallback(pCallbackFunc)					BiDET_Set_Callback(pCallbackFunc)

void														BiDET_Make_Stash(uint64_bd sizeBytes);
#define BD_MakeStash(sizeKB)								BiDET_Make_Stash((uint64_bd)sizeKB * KBtoBYTE)

void														BiDET_Reset();
#define BD_ClearStash										BiDET_Reset()


#define BD_Store(keyName, data)								BiDET_Verify_Req(keyName, &data, &(uint64_bd){sizeof(data)}, STORE, BD_ERR_ID(keyName));


#define BD_Get(keyName, container)							BiDET_Verify_Req(keyName, &container, &(uint64_bd){sizeof(container)}, GET, BD_ERR_ID(keyName))


#define BD_Remove(keyName)									BiDET_Verify_Req(keyName, BD_NULL, BD_NULL, REMOVE, BD_ERR_ID(keyName))


#define BD_Reserve(keyName, dataPtr, sizeBytes)						BiDET_Verify_Req(keyName, &dataPtr, &(uint64_bd){sizeBytes}, RESERVE, BD_ERR_ID(keyName))


#define BD_Retrieve(keyName, dataPtr, dataSize)				BiDET_Verify_Req(keyName, &dataPtr, &dataSize, RETRIEVE, BD_ERR_ID(keyName))


#define BD_Fill(keyName, data)								BiDET_Verify_Req(keyName, &data, &(uint64_bd){sizeof(data)}, FILL, BD_ERR_ID(keyName))


/*
* FEED DESIRED BD_INFO FIELD INTO THE REQUESTEDINFO
* FIELD OF BD_STASHINFO. RETURN VALUES WILL BE FED
* INTO THE INFORET FIELD. INFORET RETURN TYPES ARE
* DENOTED NEXT TO THE CORRESPONDING BD_INFO REQUEST.
*/
void														BiDET_Stash_Request(void* pInfoOut, uint32_bd stashID, bd_info infoReq);
#define BD_StashInfo(infoRet, stashID, requestedInfo)		BiDET_Stash_Request(&infoRet, stashID, requestedInfo)

#endif