#include "BiDET_Test.h"

void callback(bd_errpack errorPack)
{
	if(errorPack.errSeverity > 0)
		printf("%s\"%s\"\nFile:\t\t%s\nLine:\t\t%lu\n\n", errorPack.errMessage, errorPack.keyName, errorPack.errFile, errorPack.errLine);
	return;
}

int main(int argc, char** argv)
{
	BD_SetErrCallback(&callback);
	BD_MakeStash(32);
	//Store("validName", (uint32_bd) { 12 });

	LARGE_INTEGER frequency;
	LARGE_INTEGER timeBegin;
	LARGE_INTEGER timeEnd;
	long double elapsedTime;
	long double shortestTime = 1.0;
	char toStore = 3;

	printf("\n\t--BINARY INSERTION--\n\n");

#define listLen 1000
	int runCount = 5000;
	uint32_bd numStored = 0;

	for (int run = 0; run < runCount; run++)
	{
		BD_ClearStash;
		
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&timeBegin);

		for (int i = 0; i < listLen; i++)
			BD_Store(testWords[i], toStore);

		QueryPerformanceCounter(&timeEnd);
		elapsedTime = (timeEnd.QuadPart - timeBegin.QuadPart) * 1000.0 / frequency.QuadPart;
		shortestTime -= (elapsedTime < shortestTime) * (shortestTime - elapsedTime);
	}

	printf("\nStore Time:\t\t\t%Lf ms. \n", shortestTime);
	long double totalTime = shortestTime;
	//printf("Conflicts:\t\t\t%lli\n\n", conflicts);
	
	/*
	stash_bd* stash;
	BD_StashInfo(stash, 1, HANDLE_STASH);

	for (int i = 0; i < listLen; i++)
		printf("Item %i: \tHash: %zu\n", i, (stash->firstKey - i)->hash);
	printf("\n\n");
	*/

	//printf("Hash Average: %zu\n\n", aveHash);
	
	/*
	for (int i = 0; i < listLen; i++)
		printf("Item %i: \tHash: %zu\n\t\tData: %p\n\n", i, (Stash->keyRing - i)->hash, (Stash->keyRing - i)->data);
	printf("\n\n");
	*/
	/*
	char* retrievePtr;
	numStored = 0;
	shortestTime = 1.0;
	for (int run = 0; run < runCount; run++)
	{
		numStored = 0;
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&timeBegin);

		for (int i = 0; i < listLen; i++)
		{
			BD_Retrieve(testWords[i], retrievePtr, (uint64_bd){0});
			numStored += *retrievePtr;
		}

		QueryPerformanceCounter(&timeEnd);
		elapsedTime = (timeEnd.QuadPart - timeBegin.QuadPart) * 1000.0 / frequency.QuadPart;
		shortestTime -= (elapsedTime < shortestTime) * (shortestTime - elapsedTime);
	}
	printf("\nRetrieve Time:\t\t\t%Lf ms. \n", shortestTime);
	printf("Total retrieved:\t\t%lu\n", numStored);
	totalTime += shortestTime;

	
	char fill = 4;
	shortestTime = 1.0;
	for (int run = 0; run < runCount; run++)
	{
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&timeBegin);

		for (int i = 0; i < listLen; i++)
			BD_Fill(testWords[i], fill);

		QueryPerformanceCounter(&timeEnd);
		elapsedTime = (timeEnd.QuadPart - timeBegin.QuadPart) * 1000.0 / frequency.QuadPart;
		shortestTime -= (elapsedTime < shortestTime) * (shortestTime - elapsedTime);
	}
	printf("\nFill Time:\t\t\t%Lf ms. \n", shortestTime);
	totalTime += shortestTime;
	*/
	
	char container = 0;
	shortestTime = 1.0;
	for (int run = 0; run < runCount; run++)
	{
		numStored = 0;
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&timeBegin);

		for (int i = 0; i < listLen; i++)
		{
			container = 0;
			BD_Get(testWords[i], container);
			numStored += container;
		}

		QueryPerformanceCounter(&timeEnd);
		elapsedTime = (timeEnd.QuadPart - timeBegin.QuadPart) * 1000.0 / frequency.QuadPart;
		shortestTime -= (elapsedTime < shortestTime) * (shortestTime - elapsedTime);
	}
	printf("\nGet Time:\t\t\t%Lf ms. \n", shortestTime);
	printf("Total stored:\t\t\t%lu\n", numStored);
	totalTime += shortestTime;
	printf("\nTotal Time:\t\t\t%Lf ms. \n", totalTime);
	
	/*
	numStored = 0;
	shortestTime = 1.0;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&timeBegin);

	for (int i = 0; i < listLen; i++)
	{
		BD_Remove(testWords[i]);
		i++;
	}

	QueryPerformanceCounter(&timeEnd);
	elapsedTime = (timeEnd.QuadPart - timeBegin.QuadPart) * 1000.0 / frequency.QuadPart;
	shortestTime -= (elapsedTime < shortestTime) * (shortestTime - elapsedTime);

	printf("\nRemove Time:\t\t\t%Lf ms. \n\n", shortestTime);
	*/
	/*
	for (int i = 0; i < listLen; i++)
		printf("Item %i: \tHash: %zu\n\t\tData: %p\n\n", i, (stash->firstKey - i)->hash, (stash->firstKey - i)->data);
	printf("\n\n");
	*/

	//Get("alread", 0);
	
	//key* keyRing = Stash->keyRing;
	//uint32_bd Break = 0;
	//for (int i = 0; i < listLen; i++)
	//	Remove(testWords[i]);
	
	/*
	for (int i = 0; i < listLen; i++)
		printf("Item %i: \tHash: %zu\n", i, (Stash->keyRing - i)->hash);
	printf("\n\n");
	*/

	//Store(NULL, toStore);
	//Get("definitely not present", container);

	//uint64_bd largeContainer;
	//Get("a", largeContainer);

	uint64_bd stashSize = 0;
	BD_StashInfo(stashSize, SPACE_TOTAL);

	uint64_bd spaceRemaining = 0;
	BD_StashInfo(spaceRemaining, SPACE_REMAINING);

	uint64_bd numKeys = 0;
	BD_StashInfo(numKeys, NUM_KEYS);

	uint64_bd numStashes = 0;
	BD_StashInfo(numStashes, NUM_STASHES);

	uint64_bd numVoids = 0;
	BD_StashInfo(numVoids, NUM_VOIDS);

	key_bd* lastKey;
	BD_StashInfo(lastKey, HANDLE_KEYS);

	BD_ClearStash;
	
	uint32_bd store = 5;
	BD_Store("a", store);
	BD_Store("b", store);
	BD_Store("c", store);
	BD_Store("d", store);
	BD_Remove("b");
	BD_Remove("c");
	BD_Store("e", store);
	BD_Store("f", store);

	void* reservePtr;
	uint64_bd reserveFill = 14;
	//BD_Reserve("reserved_space", reservePtr, 8);
	//BD_Fill("reserved_space", reserveFill);
	
	return 0;
}
