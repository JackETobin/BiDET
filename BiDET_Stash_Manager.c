#include "BiDET.h"
#include <stdlib.h>

/*
* --------------------TODO--------------------
* DESIGN AN EXPANSION SYSTEM FOR THE STASH TO
* FACILITATE THE POTENTIAL NEED TO GROW STORAGE
* DYNAMICALLY. EXPANSION SYSTEM MIGHT INCLUDE:
* 
* -> ITERATION MECHANISM FOR KEYS NOT LOCATED
* PREVIOUS STASH ALLOCATIONS.
* 
* -> STASH PASSTHROUGH FUNCTION THAT ITERATES
* THROUGH STASHES AND RETURNS THE ACTIVE STASH
* TO THE UTILITIES FILE GLOBAL STASH.
* 
* -> POINTER SYSTEM THAT LINKS THE UTILITIES 
* FILE TO THE STASH MANAGEMENT FILE IN SUCH A
* WAY THAT IT EXPOSES NEWLY NECESSARY FUNCTIONS
* WHEN MORE THAN ONE STASH ARE ALLOCATED.
*/

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

	pStash->keyRing = pStash->nextStash = (void*)pStash + (pStash->sizeByte - voidPSize);
	pStash->keyRing -= 1;
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

	pStash->keyRing = pStash->nextStash = (void*)pStash + (sizeByte - voidPSize);
	pStash->keyRing -= 1;
	pStash->nextEntry = (void*)pStash + sizeof(stash_bd);
	pStash->sizeByte = sizeByte;

	stash_bd** ppStash = BiDET_Retrieve_Stash();
	*ppStash = pStash;
	atexit(Clean_Stash);
	return;
}


/*
* METADATA
* ->STASH: SIZE LEFT						uint64_bd
* ->STASH: TOTAL SIZE						uint64_bd
* ->STASH: NUM KEYS							uint64_bd
* ->STASH: NUMBER OF ACTIVE STASHES			uint64_bd
* ->STASH: GET HANDLE						STASHPTR(stash*)
* ->STASH: PTR TO KEYS						KEYPTR(key*)
*/
static void
Get_Requested_Info(void* pInfoOut, uint32_bd stashID, bd_info infoReq)
{
	stash_bd* activeStash = *BiDET_Retrieve_Stash();
	switch (infoReq)
	{
		case SPACE_TOTAL:
			*(uint64_bd*)pInfoOut = activeStash->sizeByte;
			break;

		case SPACE_REMAINING:
			void* lastEntry = activeStash->nextEntry;
			void* lastKey = (void*)(activeStash->keyRing - (activeStash->numKeys - 1));
			*(uint64_bd*)pInfoOut = lastKey - lastEntry;
			break;

		case NUM_KEYS:
			*(uint64_bd*)pInfoOut = activeStash->numKeys;
			break;

		case NUM_STASHES:
			*(uint64_bd*)pInfoOut = (activeStash != BD_NULL);
			break;

		case HANDLE_STASH:
			*(stash_bd**)pInfoOut = activeStash;
			break;

		case HANDLE_KEYS:
			*(key_bd**)pInfoOut = activeStash->keyRing - (activeStash->numKeys - (activeStash->numKeys > 0));
			break;

		default:
			break;
	}
}


void
BiDET_Stash_Request(void* pInfoOut, uint32_bd stashID, bd_info infoReq)
{
	uint32_bd requestVerification = ((pInfoOut != BD_NULL) && (stashID > 0));
	(requestVerification) ? Get_Requested_Info(pInfoOut, stashID, infoReq) : 0;
	return;
}