# BiDET
BiDET is a hash table based memory management system that uses a binary search to retrieve keys and detect issues before insertion into an ordered list. This system is for small data sets and occupied a fixed space in memory.

BiDET does not resize dynamically; if there is an attempt to store data when there is insufficuent space, the data will not be stored.

BiDET tracks voidspace in memory and will attempt to fill voidspace if possible. If after filling a void the resulting space is too small to be practical, the remaining space will be combined with the data allocated into void.

### Function that are currently working as intended:

+ BD_SetErrCallback(pCallbackFunc)
  
+ BD_MakeStash(sizeByte)

+ BD_StashInfo(stashinfo)

+ BD_ClearStash

+ BD_Store(keyName, data)

+ BD_Get(keyName, container)
  
+ BD_Reserve(keyName, dataPtr, sizeBytes)

+ BD_Retrieve(keyName, dataPtr, dataSize)

+ BD_Fill(keyName, data)


BiDET is a work in progress, and a proper README file is in the works.
