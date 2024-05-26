
#ifndef LOGGING_H
#define LOGGING_H

  #include <stdio.h>
  #include <stdlib.h>
  #include <stdbool.h>
  #include <time.h>
  #include <assert.h>
  #include <sys/ioctl.h>
  #include <unistd.h>
  #include <errno.h>
  #include <signal.h>

  #ifndef TIME_FORMAT
    #define TIME_FORMAT "%02d:%02d:%02d "
  #endif

  #define TIME_FORMAT_ARGS(tm) tm.tm_hour, tm.tm_min, tm.tm_sec

  // Color definition
  #define ANSI_COLOR_RED      "\x1b[31m"
  #define ANSI_COLOR_GREEN    "\x1b[32m"
  #define ANSI_COLOR_YELLOW   "\x1b[33m"
  #define ANSI_COLOR_BLUE     "\x1b[34m"
  #define ANSI_COLOR_MAGENTA  "\x1b[35m"
  #define ANSI_COLOR_CYAN     "\x1b[36m"
  #define ANSI_COLOR_BRED     "\x1b[91m"
  #define ANSI_COLOR_BGREEN   "\x1b[92m"
  #define ANSI_COLOR_ORANGE   "\x1b[93m"
  #define ANSI_COLOR_BBLUE    "\x1b[94m"
  #define ANSI_COLOR_BMAGENTA "\x1b[95m"
  #define ANSI_COLOR_BCYAN    "\x1b[96m"
  #define ANSI_COLOR_RESET    "\x1b[0m"

  void log_format(const char* tag, const char* message, va_list args);
  void log_error(const char* message, ...);
  void log_info(const char* message, ...);
  void log_debug(const char* message, ...);

#endif
