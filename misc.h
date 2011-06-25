#ifndef MISC_H
#define MISC_H

extern int running;
extern double now;

void die(const char *fmt, ...);

#ifndef abs
#define abs(a) ((a)<0?-(a):(a))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#endif
