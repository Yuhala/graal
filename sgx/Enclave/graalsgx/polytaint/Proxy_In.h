#ifndef __PROXY_IN_H
#define __PROXY_IN_H

#if defined(__cplusplus)
extern "C" { 
#endif

void sayHello_proxy();
int arraySum_proxy(void* param1, int len1);
void* readXData_proxy(int param1);

#if defined(__cplusplus)
}
#endif

#endif

