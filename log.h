#ifndef __LOG_H__
#define __LOG_H__

void _log(const char *level, const char *fmt, ...);

#define LOG_DEBUG(fmt, ...) do { _log("DEBUG", fmt, ##__VA_ARGS__); } while (0)
#define LOG_INFO(fmt, ...) do { _log("INFO", fmt, ##__VA_ARGS__); } while (0)
#define LOG_WARN(fmt, ...) do { _log("WARN", fmt, ##__VA_ARGS__); } while (0)
#define LOG_ERROR(fmt, ...) do { _log("ERROR", fmt, ##__VA_ARGS__); } while (0)

#endif /* __LOG_H__ */
