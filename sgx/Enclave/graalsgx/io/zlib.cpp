/**
 * @file zlib.cpp
 * @author Peterson Yuhala
 * @brief
 * Some helper functions for handling z_stream pointers
 * in and out of the enclave.
 * @version 0.1
 * @date 2022-07-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "../../Enclave.h" //for printf

// forward declarations
void replicate_internal_state(void *dest_streamp, void *src_streamp);

/**
 * @brief
 * Copies some fields from untrusted stream
 * to trusted stream.
 * @param dest_streamp
 * @param src_streamp
 */
void copy_zstream_in(void *dest_streamp, void *src_streamp)
{
    z_streamp src = (z_streamp)src_streamp;
    z_streamp dest = (z_streamp)dest_streamp;

    /**
     * @brief
     * Copy fields in the order in which they are defined
     * in the z_stream struct. Some fields are left out.
     */
    dest->next_in = src->next_in;
    dest->avail_in = src->avail_in;
    dest->total_in = src->total_in;

    /**
     * @brief
     * We just replace the enclave internal state with
     * the one from outside.
     */
    // dest->state = src->state;

    dest->next_out = src->next_out;
    dest->avail_out = src->avail_out;
    dest->total_out = src->total_out;
    // dest->msg = src->msg;

    // dest->data_type = src->data_type;
    // dest->adler = src->adler;
    // dest->reserved = src->reserved;
}

/**
 * @brief
 * Copies some fields (leaves out internal state struct field)
 * from src stream in-enclave to dest stream out of enclave.
 * @param dest_streamp
 * @param src_streamp
 */
void copy_zstream_out(void *dest_streamp, void *src_streamp, bool allocate)
{
    z_streamp src = (z_streamp)src_streamp;
    z_streamp dest = (z_streamp)dest_streamp;

    dest->avail_in = src->avail_in;
    if (src->avail_in != 0 && allocate)
    {
        printf(">>>>>>>>>>>>>> copying %d src->avail_in from enclave to outside stream\n", src->avail_in);
        dest->next_in = (Bytef *)malloc(src->avail_in * sizeof(Bytef));
        memcpy(dest->next_in, src->next_in, src->avail_in);
    }
    dest->avail_out = src->avail_out;

    if (src->avail_out != 0 && allocate)
    {
        printf(">>>>>>>>>>>>>> copying %d src->avail_out from enclave to outside stream\n", src->avail_out);
        dest->next_out = (Bytef *)malloc(src->avail_out * sizeof(Bytef));
        memcpy(dest->next_out, src->next_out, src->avail_out);
    }
    dest->total_in = src->total_in;
    dest->total_out = src->total_out;

    // dest->state = src->state;
    // replicate_internal_state(dest_streamp, src_streamp);

    dest->data_type = src->data_type;
    dest->adler = src->adler;
    dest->reserved = src->reserved;
}
/**
 * @brief
 * Prints out zstream information
 *
 * @param streamp
 */

void print_zstream(void *streamp)
{
    z_streamp strm = (z_streamp)streamp;

    printf("stream.next_in: %x\n", strm->next_in);
    printf("stream.avail_in: %d\n", strm->avail_in);
    printf("stream.next_out: %x\n", strm->next_out);
    printf("stream.avail_out: %d\n", strm->avail_out);
    printf("stream.total_in: %lu\n", strm->total_in);
    printf("stream.total_out: %lu\n", strm->total_out);
    printf("stream.data_type: %d\n", strm->data_type);
    printf("stream.adler: %lu\n", strm->adler);
    printf("stream.reserved: %lu\n", strm->reserved);
}

/**
 * @brief
 * Copies enclave state to outside state.
 * The internal state is invisible to applications,
 * so maybe this function is needless.
 *
 * @param dest_streamp
 * @param src_streamp
 */
void replicate_internal_state(void *dest_streamp, void *src_streamp)
{
    z_streamp src = (z_streamp)src_streamp;
    z_streamp dest = (z_streamp)dest_streamp;
}

/**
 * @brief
 * Copies some fields (leaves out internal state struct field)
 * from src stream to dest stream.
 * @param dest_streamp
 * @param src_streamp
 */
void copy_zstream(void *dest_streamp, void *src_streamp)
{
    z_streamp src = (z_streamp)src_streamp;
    z_streamp dest = (z_streamp)dest_streamp;

    /**
     * @brief
     * Copy fields in the order in which they are defined
     * in the z_stream struct. Some fields are left out.
     */
    if (src->next_in == NULL)
    {
        src->next_in = (Bytef *)malloc(sizeof(Bytef));
    }
    memcpy((void *)dest->next_in, (void *)src->next_in, sizeof(Bytef));
    dest->avail_in = src->avail_in;
    dest->total_in = src->total_in;

    if (src->next_out == NULL)
    {
        src->next_out = (Bytef *)malloc(sizeof(Bytef));
    }
    memcpy((void *)dest->next_out, (void *)src->next_out, sizeof(Bytef));
    dest->data_type = src->data_type;
    dest->adler = src->adler;
    dest->reserved = src->reserved;
}

/**
 * @brief
 * PYuhala
 * Print the meaning of the error code
 *
 * @param error_code
 */
void print_zlib_error(int error_code, const char *func_name)
{
    switch (error_code)
    {
    case Z_BUF_ERROR:
        printf("%s xxxxxxxxxxxxxxxxxx: Z_BUF_ERROR no progress possible: avail-in/out is zero\n", func_name);
        break;
    case Z_MEM_ERROR:
        printf("%s xxxxxxxxxxxxxxxxxx: Z_MEM_ERROR: insufficient memory\n", func_name);
        break;
    case Z_STREAM_ERROR:
        printf("%s xxxxxxxxxxxxxxxxxx: Z_STREAM_ERROR: state inconsistent or NULL\n", func_name);
        break;
    case Z_VERSION_ERROR:
        printf("%s xxxxxxxxxxxxxxxxxx: Z_VERSION_ERROR: zlib inconsistent version\n", func_name);
        break;
    case Z_DATA_ERROR:
        printf("%s xxxxxxxxxxxxxxxxxx: Z_DATA_ERROR: bad data\n", func_name);
        break;
    case Z_STREAM_END:
        printf("%s xxxxxxxxxxxxxxxxxx: Z_STREAM_END: ...\n", func_name);
        break;
    case Z_OK:
        printf("xxxxxxxxxxxxxxxxxx Z_OK: %s SUCCESS! \n", func_name);
        break;
    default:
        printf("%s xxxxxxxxxxxxxxxxxx unknown error code\n", func_name);
        break;
    }
}