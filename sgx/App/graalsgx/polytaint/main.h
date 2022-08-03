#ifndef __MAIN_H
#define __MAIN_H

#include <graal_isolate.h>


#if defined(__cplusplus)
extern "C" {
#endif

int run_main(int argc, char** argv);

void poly_2_entry(graal_isolatethread_t*);

int funcA_entry(graal_isolatethread_t*, int);

int funcD_entry(graal_isolatethread_t*, int);

int funcN_entry(graal_isolatethread_t*, int, int);

void sayHello_entry(graal_isolatethread_t*);

void vmLocatorSymbol(graal_isolatethread_t* thread);

#if defined(__cplusplus)
}
#endif
#endif
