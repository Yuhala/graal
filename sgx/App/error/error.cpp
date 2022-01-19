
/*
 * Created on Fri Aug 14 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */

#include "error.h"

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist / sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++)
    {
        if (ret == sgx_errlist[idx].err)
        {
            if (NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl && ret != 0)
        printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
    if (ret == 0)
        printf("Enclave returned successfully\n");
}