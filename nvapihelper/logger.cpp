#include "stdafx.h"
#include "logger.h"

#include <iostream>
#include <fstream>
#include <string>
#include <ShlObj.h>
#pragma comment(lib, "shell32.lib")
// for current process loaded modules
#include <Psapi.h>
extern "C" IMAGE_DOS_HEADER __ImageBase;
// for va_args
#include <cstdarg>

using std::ofstream;
using std::ios;
using std::string;

// use multi bytes win32 api only because I'm lazy to deal with character configuration

int logger_level = LOG_INFO;
char logger_buff[4096] = { 0 };

void _set_log(int level, const char* path)
{
	//TODO: proc given path to specify file prefix or location
	logger_level = (log_levels_t)level;

	//TODO: maybe specify log file's name by the module and caller
	char inst_name[MAX_PATH] = { 0 };
	GetModuleFileNameA((HINSTANCE)&__ImageBase, inst_name, MAX_PATH);
	char proc_name[MAX_PATH] = { 0 };
	GetModuleBaseNameA(GetCurrentProcess(), NULL, proc_name, MAX_PATH);
}

void _log_to_appdata(int level, const char* format, ...)
{
	// ignore if level less than set level
	if (level < logger_level)
		return;

	char path[MAX_PATH] = { 0 };
	if (FAILED(SHGetFolderPathA(NULL, CSIDL_COMMON_APPDATA, NULL, 0, path)))
		return;
	
	strcat_s(path, "\\NvApiHelper\\");
	if (FAILED(SHCreateDirectoryExA(NULL, path, NULL)))
		return;

	SYSTEMTIME st;
	GetSystemTime(&st);
	sprintf_s(path, "%sLog_%04d%02d%02d.txt", path, st.wYear, st.wMonth, st.wDay);

	ZeroMemory(logger_buff, 4096);
	va_list args;
	va_start(args, format);
	vsprintf_s(logger_buff, format, args);
	va_end(args);
	
	OutputDebugStringA(logger_buff);

	ofstream log;
	log.open(path, ios::app);
	log << (logger_buff);
	log.close();
}
