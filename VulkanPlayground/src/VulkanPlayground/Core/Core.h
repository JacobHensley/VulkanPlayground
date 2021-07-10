#include "Log.h"

#ifdef ENABLE_ASSERTS
#define ASSERT(x, ...) { if(!(x)) { LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define ASSERT(x, ...)
#endif