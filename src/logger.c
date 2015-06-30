#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>
#include "Logger.h"

#define LOGGER_BUF_SIZE 1024
#define LOG_FILENAME_MAX_LENGTH 256
#define LOGGER_PREFIX_MAX_SIZE 7

#define LOG_STRING_INFO "INFO"
#define LOG_STRING_WARN "WARN"
#define LOG_STRING_ERROR "ERROR"
#define LOG_STRING_DEBUG "DEBUG"
#define LOG_STRING_VERBOSE "VERBOSE"
#define LOG_STRING_PROLOG "PROLOG"
#define LOG_STRING_EPILOG "EPILOG"
#define LOG_STRING_INIT "\n*********<<<<<INIT LOGGER>>>>>*********\n"
#define LOG_STRING_CLOSE "\n\n*********<<<<<CLOSE LOGGER>>>>>*********\n"

typedef void (*_log_dmp_callback)(const char *const log);
void log_dmp_callback_console(const char *const log);
void log_dmp_callback_file(const char *const log);
static _log_dmp_callback g_log_dmp_console = NULL;
static _log_dmp_callback g_log_dmp_file = NULL;
static FILE *g_pFile = NULL;

void getLogFileName(char* fileName);
void getDateTime(struct tm *const tm, unsigned *const usec);
void getProcessAndThreadId(int *const pid, int *const tid);
void getPrefix(char* prefix, int log_level, int prefixId);

void _log_init(int log_output_id) {
    if (log_output_id == LOG_OUTPUT_CONSOLE) {
        g_log_dmp_console = log_dmp_callback_console;
        g_log_dmp_console(LOG_STRING_INIT);
    } else if (log_output_id == LOG_OUTPUT_FILE) {
        g_log_dmp_file = log_dmp_callback_file;

        char logFile[LOG_FILENAME_MAX_LENGTH] = "\0";
        getLogFileName(logFile);
        g_pFile = fopen(logFile, "w");
        g_log_dmp_file(LOG_STRING_INIT);
    }
}

void _log_close() {
    if (g_log_dmp_console != NULL) {
        g_log_dmp_console(LOG_STRING_CLOSE);
    }

    if (g_log_dmp_file != NULL) {
        g_log_dmp_file(LOG_STRING_CLOSE);

        if (g_pFile != NULL) {
            fclose(g_pFile);
        }
    }
}

void _log_write(const char *const func,
                const char *const file,
                const unsigned line,
                const int level,
                const int prefixId,
                const char *const fmt, ...) {
	char output[LOGGER_BUF_SIZE] = "\0";
	char message[LOGGER_BUF_SIZE] = "\0";
	char tmp[LOGGER_BUF_SIZE] = "\0";
    char prefix[LOGGER_PREFIX_MAX_SIZE] = "\0";

    bool doAssert = false;

    switch(level) {
        case LOG_LEVEL_VERBOSE: {
                switch(prefixId) {
                    case LOG_PREFIX_PROLOG:
                    case LOG_PREFIX_EPILOG: {
                        sprintf(message, "|%s|", func);
                        break;
                    }

                    default: {
                        va_list va;
                        va_start(va, fmt);
                        vsprintf(tmp, fmt, va);
                        sprintf(message, "|%s| %s", func, tmp);
                        va_end(va);

                        break;
                    }
                }

                break;
            }

        case LOG_LEVEL_ERROR: {
            va_list va;
            va_start(va, fmt);
            vsprintf(tmp, fmt, va);
            sprintf(message, "|%s line-%d| %s", func, line, tmp);
            va_end(va);
            doAssert = true;

            break;
        }

        default: {
            va_list va;
            va_start(va, fmt);
            vsprintf(tmp, fmt, va);
            sprintf(message, "|%s| %s", func, tmp);
            va_end(va);

            break;
        }
    }

    struct tm tm;
	unsigned usec = 0;
	int pid = 0;
	int tid = 0;

	getDateTime(&tm, &usec);
	getProcessAndThreadId(&pid, &tid);
    getPrefix(prefix, level, prefixId);

    sprintf(output,
            "\n%04u-%02u-%02u %02u:%02u:%02u.%03u [%5i - %5i] %6s %s",
            (unsigned) tm.tm_year+1900,
            (unsigned)tm.tm_mon,
            (unsigned)tm.tm_mday,
            (unsigned)tm.tm_hour,
            (unsigned)tm.tm_min,
            (unsigned)tm.tm_sec,
            (unsigned)(usec / 1000),
            pid,
            tid,
            prefix,
            message);

    if (g_log_dmp_console != NULL) {
        g_log_dmp_console(output);
    }

    if (g_log_dmp_file != NULL) {
        g_log_dmp_file(output);
    }

    if (doAssert) {
        LOG_CLOSE();
        assert(0);
        //_assert(output, file, line);
    }
}

void log_dmp_callback_console(const char *const log) {
    printf("%s", log);
}

void log_dmp_callback_file(const char *const log) {
    fputs(log,g_pFile);
}

void getLogFileName(char* fileName) {
    if (fileName != NULL) {
        struct tm tm;
        unsigned usec = 0;

        getDateTime(&tm, &usec);

        sprintf(fileName, "LOG-%04u%02u%02u%02u%02u%02u.txt",
                (unsigned) tm.tm_year+1900,
                (unsigned)tm.tm_mon,
                (unsigned)tm.tm_mday,
                (unsigned)tm.tm_hour,
                (unsigned)tm.tm_min,
                (unsigned)tm.tm_sec);
    }
}

void getDateTime(struct tm *const tm, unsigned *const usec) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	const time_t t = tv.tv_sec;
	*tm = *localtime(&t);
	*usec = tv.tv_usec;
}

void getProcessAndThreadId(int *const pid, int *const tid) {
	*pid = getpid();
#if defined(__linux__)
	*tid = syscall(SYS_gettid);
#elif defined(__MACH__)
	*tid = pthread_mach_thread_np(pthread_self());
#else
	*tid= pthread_self();
#endif
}

void getPrefix(char* prefix, int log_level, int prefixId) {
    if (prefix != NULL) {
        switch(log_level) {
            case LOG_LEVEL_INFO: {
                strncat(prefix, LOG_STRING_INFO, (LOGGER_PREFIX_MAX_SIZE - 1));
                break;
            }

            case LOG_LEVEL_WARN: {
                strncat(prefix, LOG_STRING_WARN, (LOGGER_PREFIX_MAX_SIZE - 1));
                break;
            }

            case LOG_LEVEL_ERROR: {
                strncat(prefix, LOG_STRING_ERROR, (LOGGER_PREFIX_MAX_SIZE - 1));
                break;
            }

            case LOG_LEVEL_DEBUG: {
                strncat(prefix, LOG_STRING_DEBUG, (LOGGER_PREFIX_MAX_SIZE - 1));
                break;
            }

            case LOG_LEVEL_VERBOSE: {
                switch(prefixId) {
                    case LOG_PREFIX_PROLOG: {
                        strncat(prefix, LOG_STRING_PROLOG, (LOGGER_PREFIX_MAX_SIZE - 1));
                        break;
                    }

                    case LOG_PREFIX_EPILOG: {
                        strncat(prefix, LOG_STRING_EPILOG, (LOGGER_PREFIX_MAX_SIZE - 1));
                        break;
                    }

                    default: {
                        strncat(prefix, LOG_STRING_VERBOSE, (LOGGER_PREFIX_MAX_SIZE - 1));
                        break;
                    }
                }

                break;
            }
        }
    }
}


