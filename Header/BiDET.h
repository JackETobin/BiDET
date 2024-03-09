#ifndef C_STORE
#define C_STORE

typedef long int			int32_bd;
typedef unsigned long int		uint32_bd;
typedef long long			int64_bd;
typedef unsigned long long		uint64_bd;

#define BD_NULL ((void*)0)

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

#define bd_nameBuffType	uint32_bd
#define bd_nameBuffSize sizeof(uint32_bd)

#ifdef BiDET_LARGE_BUFFER
#define bd_dataBuffType uint64_bd
#define bd_dataBuffSize sizeof(uint64_bd)
#else
#define bd_dataBuffType uint32_bd
#define bd_dataBuffSize sizeof(uint32_bd)
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
	key_bd*		lastKey;
	uint64_bd	sizeByte;
	int64_bd	numKeys;
	void*		nextEntry;
}stash_bd;

typedef struct BiDET_Stash_Properties
{
	stash_bd*	stashHandle;
	uint64_bd	spaceTotal;
	uint64_bd	spaceRemaining;
	uint64_bd	numKeysLive;
	uint64_bd	numKeysVoid;
	key_bd*		keyHandle;
}stashprops_bd;

void
BiDET_Exit(uint32_bd errorCode);
stash_bd**
BiDET_Retrieve_Stash();
void 
BiDET_Verify_Req(char* keyName, void* dataIO, uint64_bd* sizeIO, bd_action action, void** callID);


void									BiDET_Set_Callback(void(*pCallback)(bd_errpack));
#define BD_SetErrCallback(pCallbackFunc)				BiDET_Set_Callback(pCallbackFunc)

void									BiDET_Make_Stash(uint64_bd sizeBytes);
#define BD_MakeStash(sizeByte)						BiDET_Make_Stash((uint64_bd)sizeByte)

void									BiDET_Stash_Info_Request(stashprops_bd* stashProps);
#define BD_StashInfo(stashprops)					BiDET_Stash_Info_Request(&stashprops)

void									BiDET_Reset();
#define BD_ClearStash							BiDET_Reset()


#define BD_Store(keyName, data)						BiDET_Verify_Req(keyName, &data, &(uint64_bd){sizeof(data)}, STORE, BD_ERR_ID(keyName));


#define BD_Get(keyName, container)					BiDET_Verify_Req(keyName, &container, &(uint64_bd){sizeof(container)}, GET, BD_ERR_ID(keyName))


#define BD_Remove(keyName)						BiDET_Verify_Req(keyName, BD_NULL, BD_NULL, REMOVE, BD_ERR_ID(keyName))


#define BD_Reserve(keyName, dataPtr, sizeBytes)				BiDET_Verify_Req(keyName, &dataPtr, &(uint64_bd){sizeBytes}, RESERVE, BD_ERR_ID(keyName))


#define BD_Retrieve(keyName, dataPtr, dataSize)				BiDET_Verify_Req(keyName, &dataPtr, &dataSize, RETRIEVE, BD_ERR_ID(keyName))


#define BD_Fill(keyName, data)						BiDET_Verify_Req(keyName, &data, &(uint64_bd){sizeof(data)}, FILL, BD_ERR_ID(keyName))


#endif
