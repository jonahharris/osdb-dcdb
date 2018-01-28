#include <windows.h>
#include <winsock2.h> 
#ifndef __MINGW32__
#include "pthread.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <limits.h>
#include <direct.h>   
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <ws2tcpip.h>		// IP multicast definitions
#include <wtypes.h>

#ifndef _WIN_32_UTIL
#define _WIN_32_UTIL
 
//#define strncasecmp strnicmp 
#define false FALSE 

#define strncasecmp strnicmp

#define strcasecmp stricmp

#define _strncasecmp strncmp

#define vsnprintf _vsnprintf

#define snprintf _snprintf
 
struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

#ifndef __MINGW32__
typedef int mode_t;
#endif
int gettimeofday(struct timeval *tv , char * ); 
 
 
#endif 
