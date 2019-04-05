#ifndef __LOG_H__
#define __LOG_H__

void _log(const char *level, const char *fmt, ...);

#define LOG_DEBUG(...) _log("DEBUG", __VA_ARGS__)
#define LOG_INFO(...) _log("INFO", __VA_ARGS__)
#define LOG_WARN(...) _log("WARN", __VA_ARGS__)
#define LOG_ERROR(...) _log("ERROR", __VA_ARGS__)


#endif /* __LOG_H__ */
