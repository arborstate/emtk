#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

void
_log(int level, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "%s: ", log_level_to_str(level));
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}
