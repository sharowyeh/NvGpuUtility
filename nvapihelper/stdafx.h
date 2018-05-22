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
#include "nvapi.h"
#include "nvml.h"
#pragma comment(lib, "d3d11.lib")
#ifdef X64
#pragma comment(lib, "lib64/nvapi64.lib")
#pragma comment(lib, "lib64/nvml.lib")
#else
#pragma comment(lib, "lib86/nvapi.lib")
#endif
/// For debug log
#include <iostream>
#include <fstream>
#include <string>
#include <ShlObj.h>
#pragma comment(lib, "shell32.lib")
using std::ofstream;
using std::ios;
using std::string;
//tar201708081530 replace log path to common application data folder
#define LOGFILE(fmt, ...) {																					\
							if (true) {																\
								char buff[1024] = "";													\
								sprintf_s(buff, fmt, ##__VA_ARGS__);										\
								ofstream log;																\
								char path[MAX_PATH];														\
								if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path))) { \
									strcat_s(path, "\\NvApiHelper\\Log_NVAPI.txt");						\
									log.open(path, ios::app);												\
									log << (buff);															\
									log.close();															\
								}																			\
							}																				\
						  }

// TODO: reference additional headers your program requires here
