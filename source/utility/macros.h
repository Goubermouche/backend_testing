#pragma once
#include "console.h"


#ifdef _WIN32
#include <intrin.h>

#define DEBUG_BREAK() __debugbreak()
#define SYSTEM_WINDOWS
#elif __linux__
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#define SYSTEM_LINUX
#else
#error "Unsupported platform!"
#endif

#ifdef DEBUG
#define ASSERT(__condition, __fmt, ...)                                         \
  do {                                                                          \
    if(!(__condition)) {                                                        \
      utility::console::err("ASSERTION FAILED: ({}:{}): ", __FILE__, __LINE__); \
      utility::console::err((__fmt), ##__VA_ARGS__);                            \
      utility::console::err("\n");                                              \
      utility::console::err_flush();                                            \
      DEBUG_BREAK();                                                            \
    }                                                                           \
  } while(false)

#define NOT_IMPLEMENTED()                                                                      \
  do {                                                                                         \
    utility::console::err("ASSERTION FAILED: ({}:{}): NOT IMPLEMENTED\n", __FILE__, __LINE__); \
    utility::console::errflerr_flushush();                                                     \
    DEBUG_BREAK();                                                                             \
  } while(false)

#define PANIC( __fmt, ...)                                                    \
  do {                                                                        \
    utility::console::err("ASSERTION FAILED: ({}:{}): ", __FILE__, __LINE__); \
    utility::console::err((__fmt), ##__VA_ARGS__);                            \
    utility::console::err("\n");                                              \
    utility::console::err_flush();                                            \
    DEBUG_BREAK();                                                            \
  } while(false)
#else
#define ASSERT(__condition, __fmt, ...)
#define NOT_IMPLEMENTED()
#define PANIC(__fmt, ...)
#endif

#define SUPPRESS_C4100(__statement) (void)__statement

namespace baremetal {
	
} // namespace baremetal
