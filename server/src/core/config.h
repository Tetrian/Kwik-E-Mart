#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* ************************************************************************** */

#ifndef PORT
  #define PORT 8080
#endif

#ifndef __FILE_NAME__
  #define __FILE_NAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef ADDRESS
  #define ADDRESS "127.0.0.1"
#endif

/* ************************************************************************** */

#endif
