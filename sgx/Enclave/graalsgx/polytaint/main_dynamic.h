#ifndef __MAIN_H
#define __MAIN_H

#include <graal_isolate_dynamic.h>


#if defined(__cplusplus)
extern "C" {
#endif

typedef int (*run_main_fn_t)(int argc, char** argv);

typedef void (*poly_2_entry_fn_t)(graal_isolatethread_t*);

typedef int (*funcA_entry_fn_t)(graal_isolatethread_t*, int);

typedef int (*funcD_entry_fn_t)(graal_isolatethread_t*, int);

typedef int (*funcN_entry_fn_t)(graal_isolatethread_t*, int, int);

typedef void (*sayHello_entry_fn_t)(graal_isolatethread_t*);

typedef void (*vmLocatorSymbol_fn_t)(graal_isolatethread_t* thread);

#if defined(__cplusplus)
}
#endif
#endif
