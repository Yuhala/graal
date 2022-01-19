#ifndef __MAIN_H
#define __MAIN_H

#include <graal_isolate.h>


#if defined(__cplusplus)
extern "C" {
#endif

int run_main(int argc, char** argv);

int graal_add(graal_isolatethread_t*, int, int);

void vmLocatorSymbol(graal_isolatethread_t* thread);

#if defined(__cplusplus)
}
#endif
#endif
