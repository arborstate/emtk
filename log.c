#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

static int _log_level = LOG_LEVEL_DEBUG;

void
log_set_level(int level)
{
	_log_level = level;
}

const char *log_level_to_str(int level)
{
	const char *_level;

	switch (level) {
	case LOG_LEVEL_DEBUG:
		_level = "DEBUG";
		break;
	case LOG_LEVEL_INFO:
		_level = "INFO";
		break;
	case LOG_LEVEL_WARN:
		_level = "WARN";
		break;
	case LOG_LEVEL_ERROR:
		_level = "ERROR";
		break;
	default:
		_level = "UNKNOWN";
		break;
	}

	return _level;
}
