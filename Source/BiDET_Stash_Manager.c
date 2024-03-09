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
Get_Requested_Info(stashprops_bd* pStashProps)
{
	stash_bd* activeStash = *BiDET_Retrieve_Stash();

	uint64_bd numVoids = 0;
	key_bd* pKey = activeStash->lastKey;
	while (pKey->hash == BD_HASHMAX)
	{ numVoids++; pKey++; }

	*pStashProps = (stashprops_bd)
	{
		.spaceTotal = activeStash->sizeByte,
		.spaceRemaining = (void*)activeStash->lastKey - activeStash->nextEntry,
		.numKeysLive = activeStash->numKeys - numVoids,
		.numKeysVoid = numVoids,
		.stashHandle = activeStash,
		.keyHandle = activeStash->lastKey + numVoids
	};
}


void
BiDET_Stash_Request(stashprops_bd* pStashProps)
{
	(pStashProps) ? Get_Requested_Info(pStashProps) : 0;
	return;
}
