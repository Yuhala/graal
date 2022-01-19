

#ifndef OCALL_COUNTER_H
#define OCALL_COUNTER_H

#if defined(__cplusplus)
extern "C"
{
#endif

    void log_ocall(const char *func);
    void showOcallLog(int num);
    void writeVal(const char *path, int val);

#if defined(__cplusplus)
}
#endif

#endif /* OCALL_COUNTER_H */
