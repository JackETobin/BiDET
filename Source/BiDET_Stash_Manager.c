#include "BiDET.h"
#include <stdlib.h>

#define voidPSize sizeof(void*)

void
BiDET_Exit(uint32_bd errorCode)
{ exit(errorCode); }


static void
Clean_Stash()
{
	void* pStash = *BiDET_Retrieve_Stash();
	free(pStash);
	return;
}


void
BiDET_Reset()
{
	stash_bd* pStash = *(stash_bd**)BiDET_Retrieve_Stash();
	pStash->numKeys = 0;

	pStash->lastKey = pStash->nextStash = (void*)pStash + (pStash->sizeByte - voidPSize);
	pStash->nextEntry = (void*)pStash + sizeof(stash_bd);

	for (uint64_bd i = 0; (void*)pStash->nextEntry + i < (void*)pStash->nextStash; i++)
		*((char*)pStash->nextEntry + i) = 0;

	return;
}


void
BiDET_Make_Stash(const uint64_bd sizeByte)
{
	stash_bd* pStash = malloc(sizeByte);
	for (uint64_bd i = 0; i < sizeByte; i++)
		*((char*)pStash + i) = 0;

	pStash->lastKey = pStash->nextStash = (void*)pStash + (sizeByte - voidPSize);
	pStash->nextEntry = (void*)pStash + sizeof(stash_bd);
	pStash->sizeByte = sizeByte;

	stash_bd** ppStash = BiDET_Retrieve_Stash();
	*ppStash = pStash;
	atexit(Clean_Stash);
	return;
}


static void
Get_Requested_Info(void* pInfoOut, bd_info infoReq)
{
	stash_bd* activeStash = *BiDET_Retrieve_Stash();
	switch (infoReq)
	{
		case SPACE_TOTAL:
			*(uint64_bd*)pInfoOut = activeStash->sizeByte;
			break;

		case SPACE_REMAINING:
			*(uint64_bd*)pInfoOut = (void*)activeStash->lastKey - activeStash->nextEntry;
			break;

		case NUM_KEYS:
			*(uint64_bd*)pInfoOut = activeStash->numKeys;
			break;

		case NUM_VOIDS:
			uint64_bd numVoids = 0;
			key_bd* pKey = activeStash->lastKey;
			while(pKey->hash == BD_HASHMAX)
			{ numVoids++; pKey++; }
			*(uint64_bd*)pInfoOut = numVoids;
			break;

		case NUM_STASHES:
			*(uint64_bd*)pInfoOut = (activeStash != BD_NULL);
			break;

		case HANDLE_STASH:
			*(stash_bd**)pInfoOut = activeStash;
			break;

		case HANDLE_KEYS:
			*(key_bd**)pInfoOut = activeStash->lastKey;
			break;

		default:
			break;
	}
}


void
BiDET_Stash_Request(void* pInfoOut, bd_info infoReq)
{
	uint32_bd requestVerification = (pInfoOut != BD_NULL);
	(requestVerification) ? Get_Requested_Info(pInfoOut, infoReq) : 0;
	return;
}
