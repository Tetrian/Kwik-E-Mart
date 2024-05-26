#include <stdarg.h>
#include <stdio.h>

#include "logging.h"

// Returns the values that represent the corresponding local time
static inline struct tm get_time() {
  time_t t = time(NULL);
  return *localtime(&t);
}

// Send a formattable output to sdtout
void log_format(const char* tag, const char* message, va_list args) {
  struct tm tm = get_time();
  printf(TIME_FORMAT "%s", TIME_FORMAT_ARGS(tm), tag);
  vprintf(message, args);
}

// Print a message in an error format
void log_error(const char* message, ...) {
  va_list args; // accept a variable number of arguments
  va_start(args, message); // initialize the list to poin at first argument
  log_format(ANSI_COLOR_RED "error " ANSI_COLOR_RESET, message, args);
  va_end(args); // clean the list
}

// Print a message in an info format
void log_info(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log_format(ANSI_COLOR_BLUE "info  " ANSI_COLOR_RESET, message, args);
  va_end(args);
}

// Print a message in a debug format
void log_debug(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log_format(ANSI_COLOR_ORANGE "debug " ANSI_COLOR_RESET, message, args);
  va_end(args);
}
