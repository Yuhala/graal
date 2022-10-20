#ifndef __MAIN_H
#define __MAIN_H

#include <graal_isolate.h>


#if defined(__cplusplus)
extern "C" {
#endif

int run_main(int argc, char** argv);

double funcA_entry(graal_isolatethread_t*);

void sayHello_entry(graal_isolatethread_t*);

int arraySum_entry(graal_isolatethread_t*, char*, int);

char* readXData_entry(graal_isolatethread_t*, int);

void vmLocatorSymbol(graal_isolatethread_t* thread);

#if defined(__cplusplus)
}
#endif
#endif
