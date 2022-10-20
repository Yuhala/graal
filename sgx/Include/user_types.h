/*
 * Created on Fri Jul 17 2020
 *
 * Copyright (c) 2020 Peterson Yuhala and Jämes Ménétrey, IIUN
 */

/* User defined types */

typedef void *buffer_t;
typedef int array_t[10];

typedef unsigned char Byte;
typedef unsigned char Bytef;
typedef long off64_t;
typedef size_t z_size_t;

typedef void (*__sighandler_t)(int);

#define SIGHUP 1  /* Hangup the process */
#define SIGINT 2  /* Interrupt the process */
#define SIGQUIT 3 /* Quit the process */
#define SIGILL 4  /* Illegal instruction. */
#define SIGTRAP 5 /* Trace trap. */
#define SIGABRT 6 /* Abort. */

#define LOOPS_PER_THREAD 500
//#define GRAAL_SGX_DEBUG 1

/* Enable/Disable debug messages */
#ifdef GRAAL_SGX_DEBUG
#define GRAAL_SGX_INFO() printf("[GRAAL_SGX_DEBUG]: File: %s Func: %s: %d\n", __FILE__, __FUNCTION__, __LINE__)
#define GRAAL_SGX_DEBUG_PRINT(message) printf("[GRAAL_SGX_DEBUG]: " message "\n")
#define GRAAL_SGX_DEBUG_PRINTF(format, args...) printf("[GRAAL_SGX_DEBUG]: " format "\n", args)
#else
#define GRAAL_SGX_INFO()
#define GRAAL_SGX_DEBUG_PRINT(message)
#define GRAAL_SGX_DEBUG_PRINTF(format, args...)
#endif

#define CHECK_STATUS(status)        \
    do                              \
    {                               \
        if (status != SGX_SUCCESS)  \
        {                           \
            printf("SGX called failed with status: %d\n",status); \
            abort();                \
        }                           \
    } while (0)
