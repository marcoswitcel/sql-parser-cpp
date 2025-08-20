#pragma once

#include <stdlib.h>

#define Trace(MESSAGE, ...) { printf("%s:%d | ", __FILE__, __LINE__), printf(MESSAGE, __VA_ARGS__), printf("\n"); }

