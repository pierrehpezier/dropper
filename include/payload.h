#include <windows.h>
#include <winhttp.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY 4
uint32_t payload(void *lpParameter);
