// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <d3d11.h>
#include "include/nvapi.h"
#include "include/nvml.h"
#pragma comment(lib, "d3d11.lib")
#ifdef X64
#pragma comment(lib, "lib64/nvapi64.lib")
#pragma comment(lib, "lib64/nvml.lib")
#else
#pragma comment(lib, "lib86/nvapi.lib")
#endif
