# BiDET
BiDET is a hash table based memory management system that uses a binary search to retrieve keys and detect issues before insertion into an ordered list. This system is for small data sets and occupied a fixed space in memory.

BiDET does not resize dynamically; if there is an attempt to store data when there is insufficuent space, the data will not be stored.

BiDET tracks voidspace in memory and will attempt to fill voidspace if possible. If after filling a void the resulting space is too small to be practical, the remaining space will be combined with the data allocated into void.

## 3/10/2024: BiDET cannot write into voids in memory as it is unable to properly read void size. If you want to use remove, you can only use it after the most recent call to a storge function.

I am also working on a compaction function that compacts all live data into the front of the allocation and pushes back the allocator.

### Function that are currently working as intended:

+ BD_SetErrCallback(pCallbackFunc)
  
+ BD_MakeStash(sizeByte)

+ BD_StashInfo(stashinfo)

+ BD_ClearStash

+ BD_Store(keyName, data)

+ BD_Get(keyName, container)
  
+ BD_Reserve(keyName, dataPtr, sizeBytes)

+ BD_Retrieve(keyName, dataPtr, dataSizeReturn)

+ BD_Fill(keyName, data)


BiDET is a work in progress, and a proper README file is in the works.
