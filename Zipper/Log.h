#include "RingBuffer.h"

#include "Types.h"

#define LOG_EVENT_MAX_LENGTH 128
#define LOG_EVENT_SCOPE_MAX_LENGTH 128

enum LogLevel {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
};

class Time {
private:
    UINT64_T seconds_;
    UINT64_T nanoseconds_;
};

struct LogEvent {
    LogLevel level;
    Time time;
    SIZE_T line;
    char scope[LOG_EVENT_SCOPE_MAX_LENGTH];
    char buffer[LOG_EVENT_MAX_LENGTH];
};
