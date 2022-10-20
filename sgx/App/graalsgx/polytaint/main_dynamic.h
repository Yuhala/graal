#ifndef __MAIN_H
#define __MAIN_H

#include <graal_isolate_dynamic.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef int (*run_main_fn_t)(int argc, char** argv);

typedef double (*funcA_entry_fn_t)(graal_isolatethread_t*);

typedef void (*sayHello_entry_fn_t)(graal_isolatethread_t*);

typedef int (*arraySum_entry_fn_t)(graal_isolatethread_t*, char*, int);

typedef char* (*readXData_entry_fn_t)(graal_isolatethread_t*, int);

typedef void (*vmLocatorSymbol_fn_t)(graal_isolatethread_t* thread);

#if defined(__cplusplus)
}
#endif
#endif
