/*
 * Created on Wed Mar 17 2021
 *
 * Copyright (c) 2021 Peterson Yuhala, IIUN
 */

#ifndef INET_H
#define INET_H

#define NS_INT16SZ 2
#define NS_INADDRSZ 4
#define NS_IN6ADDRSZ 16

#if defined(__cplusplus)
extern "C"
{
#endif
    static int hex_digit_value(char ch);
    static int inet_pton4(const char *src, const char *end, unsigned char *dst);
    static int inet_pton6(const char *src, const char *src_endp, unsigned char *dst);

#if defined(__cplusplus)
}
#endif

#endif /* INET_H */
