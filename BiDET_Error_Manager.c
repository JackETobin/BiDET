#include "BiDET_Error.h"

#ifdef ENABLE_DEBUG

static void(*pErrorCallback)(bd_errpack);

static char* BiDET_Error_Message[] =
{
	"All clear...",
	"Invalid key name...",
	"Call MakeStash() before using c_store functionality...",
	"Key name duplicate detected...",
	"Requested key not found...",
	"Missmatch between size of data and size of container...",
	"Additional stash has been allocated...",
	"Possible data truncation...",
	"Not enough space in Stash..."
};


void
BiDET_Set_Callback(void(*pCallback)(bd_errpack))
{ pErrorCallback = pCallback; return; }


err_uint32
BiDET_Error_Catch(bd_errcode errorCode, void* callID[])
{
	if (pErrorCallback)
		pErrorCallback((bd_errpack)
	{
		errorCode,
		BiDET_Error_Message[errorCode MESSAGE],
		errorCode SEVERITY,
		*(err_uint32*)callID[BD_ERR_LINE],
		callID[BD_ERR_FILE],
		callID[BD_ERR_KEYN]
	});
	return (errorCode == BD_ALLCLEAR);
}

#else

void
BiDET_Set_Callback(void(*pCallback)(bd_errpack))
{
	if (pCallback)
		pCallback((bd_errpack)
	{
		0,
		"Error callback disabled in release mode...",
		0, 0,
		"Disabled", "Disabled"
	});
	return;
}

#endif