# BiDET
BiDET is a hash table based memory management system that uses a binary search to retrieve keys and detect issues before insertion into an ordered list. This system is for small data sets and occupied a fixed space in memory.

BiDET does not resize dynamically; if there is an attempt to store data when there is insufficuent space, the data will not be stored.

BiDET tracks voidspace in memory and will attempt to fill voidspace if possible. If after filling a void the resulting space is too small to be practical, the remaining space will be combined with the data allocated into void.

BiDET is a work in progress, and a proper README file is in the works.

# IMPORTANT NOTE: ONLY STORE AND GET ARE WORKING AS OF 2/18. FUNCTIONALITY WILL BE RESTORED SOON.
