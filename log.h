#ifndef __LOG_H__
#define __LOG_H__

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_ERROR 3

void _log(int level, const char *fmt, ...);
void log_set_level(int level);
int log_get_level(void);

const char *log_level_to_str(int level);

#define LOG_DEBUG(fmt, ...) do { _log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__); } while (0)
#define LOG_INFO(fmt, ...) do { _log(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__); } while (0)
#define LOG_WARN(fmt, ...) do { _log(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__); } while (0)
#define LOG_ERROR(fmt, ...) do { _log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__); } while (0)

#endif /* __LOG_H__ */
