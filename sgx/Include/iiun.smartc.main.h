#ifndef __IIUN_SMARTC_MAIN_H
#define __IIUN_SMARTC_MAIN_H

#include <graal_isolate.h>


#if defined(__cplusplus)
extern "C" {
#endif

int run_main(int argc, char** argv);

void vmLocatorSymbol(graal_isolatethread_t* thread);

#if defined(__cplusplus)
}
#endif
#endif
