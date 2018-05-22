#include "stdafx.h"
#include "nvapihelper.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		LOGFILE("---> process attached module:0x%p reserved:0x%p pid:%d tid:%d\n", hModule, lpReserved, GetCurrentProcessId(), GetCurrentThreadId());
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		LOGFILE("<--- process detached module:0x%p reserved:0x%p pid:%d tid:%d\n", hModule, lpReserved, GetCurrentProcessId(), GetCurrentThreadId());
		break;
	}
	return TRUE;
}

