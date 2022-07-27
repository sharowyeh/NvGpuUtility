#pragma once
#include "stdafx.h"

// common log levels from verbose(0) to fatal(5)
typedef enum log_levels_t {
	LOG_VERBOSE = 0,
	LOG_DEBUG = 1,
	LOG_INFO = 2,
	LOG_WARNING = 3,
	LOG_ERROR = 4,
	LOG_FATAL = 5
} LOG_LEVELS;

void __declspec(dllexport) _set_log(int level);
void __declspec(dllexport) _log_to_appdata(int level, const char* format, ...);

#define LOGFILE(fmt, ...) _log_to_appdata(LOG_INFO, fmt, ##__VA_ARGS__)
#define SETLOG(level, path) _set_log(level, path)
#define LOGLEVEL(level, fmt, ...) _log_to_appdata(level, fmt, ##__VA_ARGS__)
