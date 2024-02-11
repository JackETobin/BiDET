#ifndef	C_STORE_ERROR
#define C_STORE_ERROR

#define BD_ALLCLEAR 0
#define STASHMISSNG 402
#define INVALIDNAME 401
#define KEYNOTFOUND 604
#define DUPLICATEKY 303
#define SPACENEEDED 408
#define DATATRUNCATION 407
#define CONTAINERMISSMATCH 205

typedef unsigned long int		err_uint32;
typedef unsigned long long		err_uint64;

#ifdef _DEBUG
#define ENABLE_DEBUG

#define MESSAGE  %100
#define SEVERITY %7
/*
* ERROR SEVERITY:
* INFO		-> 0
* WARNING	-> 1
* ERROR		-> 2
* NOOP		-> 3
* 
* NOOP INDICATES A COMPLETE LACK OF FUNCTIONALITY
*/

#define BD_ERR_LINE 0
#define BD_ERR_FILE 1
#define BD_ERR_KEYN 2
#define BD_ERR_ID(keyName) (void*[]){&(err_uint32){__LINE__}, (char*){__FILE__}, (char*){keyName}}


typedef enum BiDET_Error_Code
{
	BD_ALL_CLEAR = 0,			//INFO			%100 = 0		%7 = 0
	BD_NAMEINVAL = 401,			//ERROR			%100 = 1		%7 = 2
	BD_MAKESTASH = 402,			//NOOP			%100 = 2		%7 = 3
	BD_DUPLICATE = 303,			//ERROR			%100 = 3		%7 = 2
	BD_KEYNOTFND = 604,			//ERROR			%100 = 4		%7 = 2
	BD_TYPEMATCH = 205,			//WARNING		%100 = 5		%7 = 2
	BD_EXPDSTASH = 106,			//WARNING		%100 = 6		%7 = 1
	BD_DATATRUNC = 407,			//WARNING		%100 = 7		%7 = 1
	BD_NEEDSPACE = 408			//ERROR			%100 = 8		%7 = 2
}bd_errcode;


err_uint32					BiDET_Error_Catch(bd_errcode errorCode, void* callID[]);
#define ERR_CATCH(errCode, callID)		BiDET_Error_Catch(errCode, callID)

#else

#define BD_ERR_ID(x) BD_NULL
#define bd_errcode err_uint32
#define ERR_CATCH(errCode, callID) (errCode == BD_ALLCLEAR)

#endif

typedef struct BiDET_Error_Package
{
	bd_errcode		errCode;
	char*			errMessage;
	err_uint32		errSeverity;
	err_uint32		errLine;
	char*			errFile;
	char*			keyName;
}bd_errpack;

#endif
