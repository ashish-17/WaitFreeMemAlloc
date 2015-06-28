#pragma once

#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#define LOG_LEVEL_NONE 0 // No logging
#define LOG_LEVEL_INFO 1 // Only informative logs
#define LOG_LEVEL_WARN 2 // Include Warnings in the log
#define LOG_LEVEL_ERROR 3 // Include the error and related description
#define LOG_LEVEL_DEBUG 4 // Include Debug info in the  logs
#define LOG_LEVEL_VERBOSE 0xffff // All the logs (Include prolog/epilog and other such logs)

#define LOG_OUTPUT_CONSOLE 0
#define LOG_OUTPUT_FILE 1

/*
    Messages with the defined level or lower will be printed others will be ignored.
    State the current log level by defining LOGGING_LEVEL
    This can be done, either in the make files scripts or in specific files in code
    <1>  CC_ARGS := -DLOGGING_LEVEL=LOG_LEVEL_VERBOSE
    <2>  #define LOGGING_LEVEL LOG_LEVEL_VERBOSE // Before including Logger.h in the .c file.

*/

#if defined(LOGGING_LEVEL)
    #define _LOGGING_LEVEL LOGGING_LEVEL
#else
    #define _LOGGING_LEVEL LOG_LEVEL_VERBOSE
#endif // defined

#define LOGGING_LEVEL_ALLOW(level) ((level) <= _LOGGING_LEVEL)
#define LOGGING_LEVEL_ALLOW_INFO LOGGING_LEVEL_ALLOW(LOG_LEVEL_INFO)
#define LOGGING_LEVEL_ALLOW_WARN LOGGING_LEVEL_ALLOW(LOG_LEVEL_WARN)
#define LOGGING_LEVEL_ALLOW_ERROR LOGGING_LEVEL_ALLOW(LOG_LEVEL_ERROR)
#define LOGGING_LEVEL_ALLOW_DEBUG LOGGING_LEVEL_ALLOW(LOG_LEVEL_DEBUG)
#define LOGGING_LEVEL_ALLOW_VERBOSE LOGGING_LEVEL_ALLOW(LOG_LEVEL_VERBOSE)

#define LOG_PREFIX_DEFAULT -1
#define LOG_PREFIX_PROLOG 0
#define LOG_PREFIX_EPILOG 1

void _log_init(int log_output_id);
void _log_close();

void _log_write(const char *const func,
                const char *const file,
                const unsigned line,
                const int level,
                const int prefixId,
                const char *const fmt, ...);

#define _LOG_MSG(level, ...) \
                if (LOGGING_LEVEL_ALLOW(level)) \
                    _log_write(__FUNCTION__, __FILE__, __LINE__, level, LOG_PREFIX_DEFAULT, __VA_ARGS__);

#define _LOG_MSG_PREFIXED(level, prefixId) \
                if (LOGGING_LEVEL_ALLOW(level)) \
                    _log_write(__FUNCTION__, __FILE__, __LINE__, level, prefixId, "");

/*
    Following macros should be used by user for logging.
*/
#define LOG_INFO(...) _LOG_MSG(LOG_LEVEL_INFO, __VA_ARGS__);
#define LOG_WARN(...) _LOG_MSG(LOG_LEVEL_WARN, __VA_ARGS__);
#define LOG_ERROR(...) _LOG_MSG(LOG_LEVEL_ERROR, __VA_ARGS__);
#define LOG_DEBUG(...) _LOG_MSG(LOG_LEVEL_DEBUG, __VA_ARGS__);
#define LOG_PROLOG() _LOG_MSG_PREFIXED(LOG_LEVEL_VERBOSE, LOG_PREFIX_PROLOG);
#define LOG_EPILOG() _LOG_MSG_PREFIXED(LOG_LEVEL_VERBOSE, LOG_PREFIX_EPILOG);

#define LOG_INIT_CONSOLE() _log_init(LOG_OUTPUT_CONSOLE);
#define LOG_INIT_FILE() _log_init(LOG_OUTPUT_FILE);
#define LOG_CLOSE() _log_close();

#endif // LOGGER_H_INCLUDED
