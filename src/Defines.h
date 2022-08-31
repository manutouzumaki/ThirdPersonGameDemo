#ifndef _DEFINES_H_
#define _DEFINES_H_

#define TO_RAD(value) ((value)*(3.14159265359f/180.0f))

#define ArrayCount(array) (sizeof(array)/sizeof((array)[0]))
#define Assert(condition) if(!(condition)) { *(unsigned int*)0 = 0;}

#define Kilobyte(value) (value * 1024LL)
#define Megabyte(value) (value * 1024LL * 1024LL)
#define Gigabyte(value) (value * 1024LL * 1024LL * 1024LL)
#define Terabyte(value) (value * 1024LL * 1024LL * 1024LL * 1024LL)

#endif
