/*
 * Copyright (C) 2011-2018 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <immintrin.h>
#include <cpuid.h>
#include <stdbool.h>
#include "encryptip.h"
#include "pcl_common.h"

/*#include "openssl/evp.h"
#include "openssl/ossl_typ.h"
#include "openssl/sha.h"
#include <sgx_tseal.h>
#include <sgx_tcrypto.h>

#include "sgx_pcl_guid.h"*/


/*
 * @func print_usage prints sgx_encrypt usage instructions 
 * @param IN char* encip_name is the name of the application
 */
void print_usage(IN char* encip_name)
{
    printf("\n");
    printf("\tUsage: \n");
    printf("\t  %s -i <input enclave so file name> -o <output enclave so file name> [-d]\n", encip_name);
    printf("\n");
}



/*
 * @func parse_elf prases the ELF buffer and assigns dat astruct with:
 * 1. Pointer to elf sections array
 * 2. Index of sections names strings section
 * 3. Number of sections
 * 4. Pointer to sections names structure
 * 5. Pointer to elf segments array 
 * 6. Number of segments
 * @param IN const void* const elf_buf_in is a pointer to the ELF binary in memory
 * @param size_t elf_size, ELF binary size in bytes
 * @param OUT pcl_data_t* dat is a pointer to the struct holding output
 * @return:
 * ENCIP_ERROR_PARSE_ELF_INVALID_PARAM if input parameters are NULL
 * ENCIP_ERROR_PARSE_ELF_INVALID_IMAGE if ELF image is not valid
 * ENCIP_SUCCESS if success
 */

static encip_ret_e parse_elf(const void* const elf_buf_in, size_t elf_size, pcl_data_t* dat)
{
    if(NULL == elf_buf_in || NULL == dat)
        return ENCIP_ERROR_PARSE_ELF_INVALID_PARAM;
    uint8_t* elf_buf = (uint8_t*)elf_buf_in;
    // Elf header is at the encalve input file start address:
    if(sizeof(Elf64_Ehdr) > elf_size)
        return ENCIP_ERROR_PARSE_ELF_INVALID_IMAGE;

    Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*)(elf_buf);
    // Verify magic: 
    if(ELFMAG0 != elf_hdr->e_ident[EI_MAG0] ||
       ELFMAG1 != elf_hdr->e_ident[EI_MAG1] ||
       ELFMAG2 != elf_hdr->e_ident[EI_MAG2] ||
       ELFMAG3 != elf_hdr->e_ident[EI_MAG3])
        return ENCIP_ERROR_PARSE_ELF_INVALID_IMAGE;

    // Find the number of sections:
    dat->nsections = elf_hdr->e_shnum;

    // Find the index of the section which contains the sections names strings:
    uint16_t shstrndx = elf_hdr->e_shstrndx;
    dat->shstrndx = shstrndx;
    if(dat->nsections <= shstrndx)
        return ENCIP_ERROR_PARSE_ELF_INVALID_IMAGE;

    // Find the array of the sections headers:
    if((elf_hdr->e_shoff >= elf_size) ||
       (elf_hdr->e_shoff + dat->nsections * sizeof(Elf64_Shdr) < elf_hdr->e_shoff) ||
       (elf_hdr->e_shoff + dat->nsections * sizeof(Elf64_Shdr) > elf_size))
        return ENCIP_ERROR_PARSE_ELF_INVALID_IMAGE;

    dat->elf_sec = (Elf64_Shdr*)(elf_buf + elf_hdr->e_shoff);

    // Find the begining of the section which contains the sections names strings
    if((dat->elf_sec[shstrndx].sh_offset >= elf_size) ||  
       (dat->elf_sec[shstrndx].sh_offset + dat->elf_sec[shstrndx].sh_size < dat->elf_sec[shstrndx].sh_offset) ||
       (dat->elf_sec[shstrndx].sh_offset + dat->elf_sec[shstrndx].sh_size > elf_size))
        return ENCIP_ERROR_PARSE_ELF_INVALID_IMAGE;

    dat->sections_names = (char*)(elf_buf + dat->elf_sec[shstrndx].sh_offset);

    // Find number of segments: 
    dat->nsegments = elf_hdr->e_phnum;

    if((elf_hdr->e_phoff >= elf_size) ||
       (elf_hdr->e_phoff + dat->nsegments * sizeof(Elf64_Phdr) < elf_hdr->e_phoff) ||
       (elf_hdr->e_phoff + dat->nsegments * sizeof(Elf64_Phdr) > elf_size))
        return ENCIP_ERROR_PARSE_ELF_INVALID_IMAGE;

    dat->phdr = (Elf64_Phdr*)(elf_buf + elf_hdr->e_phoff);
    
    return ENCIP_SUCCESS;
}


/*
 * @func update_flags updates the flags of sections or segments that must become writable
 * @param uint16_t secidx, index of section for which the flags are currently updated
 * @param INOUT pcl_data_t* dat, ELF data
 * @return encip_ret_e, ENCIP_ERROR_UPDATEF_INVALID_PAR if dat is NULL, else ENCIP_SUCCESS
 */
static inline encip_ret_e update_flags(uint16_t secidx, INOUT pcl_data_t* dat)
{
    if(NULL == dat)
        return ENCIP_ERROR_UPDATEF_INVALID_PAR;

    // Mark section as writable:
    dat->elf_sec[secidx].sh_flags |= SHF_WRITE;
    Elf64_Addr secstart = dat->elf_sec[secidx].sh_addr;
    size_t     secsize  = dat->elf_sec[secidx].sh_size;
    /*
     * If section overlaps segment:
     * 1. Verify segment is readable
     * 2. Mark segment as writable
     */
	//	Elf64_Half	p_type;		/* entry type */
	//	Elf64_Half	p_flags;	/* flags */
	//	Elf64_Off	p_offset;	/* offset */
	//	Elf64_Addr	p_vaddr;	/* virtual address */
	//	Elf64_Addr	p_paddr;	/* physical address */
	//	Elf64_Xword	p_filesz;	/* file size */
	//	Elf64_Xword	p_memsz;	/* memory size */
	//	Elf64_Xword	p_align;	/* memory & file alignment */

    for(uint16_t segidx=0;segidx<dat->nsegments;segidx++)
    {
        Elf64_Addr segstart = dat->phdr[segidx].p_vaddr;
        size_t     segsize  = dat->phdr[segidx].p_memsz;

			printf("ELF Segment Index=%d, name=\"%s\", type=%d, flags=%d, memsize=%lu\n", segidx, dat->sections_names + dat->elf_sec[secidx].sh_name, dat->phdr[segidx].p_type, dat->phdr[segidx].p_flags, dat->phdr[segidx].p_memsz);

		if (dat->phdr[segidx].p_type == 1 && dat->phdr[segidx].p_flags == (PF_W | PF_R))
		{   dat->phdr[segidx].p_flags = (Elf64_Word)(PF_W | PF_R | PF_X);
			printf("Flag is marked as executable\n");
			printf("(Changed) ELF Segment Index=%d, name=\"%s\", type=%d, flags=%d, memsize=%lu\n", segidx, dat->sections_names + dat->elf_sec[secidx].sh_name, dat->phdr[segidx].p_type, dat->phdr[segidx].p_flags, dat->phdr[segidx].p_memsz);
		}

        if(((secstart           <  segstart + segsize) && (secstart           >= segstart          )) ||
           ((secstart + secsize >  segstart          ) && (secstart + secsize <= segstart + segsize)))
        {
            // Segment must be readible: 
            if(!(dat->phdr[segidx].p_flags & (Elf64_Word)PF_R))
            {
                printf("\n\nError: segment %d ", segidx);

                if(dat->elf_sec[secidx].sh_name < dat->elf_sec[dat->shstrndx].sh_size)
                {
                    char* sec_name = dat->sections_names + dat->elf_sec[secidx].sh_name;
                    /*
                     * Verifying string starts before end of section. Assuming (but not checking) 
                     * that string ends before end of section. Additional check will complicate code.
                     * Assuming the platform this application is running on is not compromized. 
                     */
                    printf("overlaps encrypted section \"%s\" and ", sec_name);
                }    
                printf("is not readable. Exiting!!!\n\n\n");   
                return ENCIP_ERROR_SEGMENT_NOT_READABLE;
            }
            // Mark segment as wirtable:
            dat->phdr[segidx].p_flags |= (Elf64_Word)PF_W;
				printf("Flag is marked as writable\n");
				printf("(Changed) ELF Segment Index=%d, name=\"%s\", type=%d, flags=%d, memsize=%lu\n", segidx, dat->sections_names + dat->elf_sec[secidx].sh_name, dat->phdr[segidx].p_type, dat->phdr[segidx].p_flags, dat->phdr[segidx].p_memsz);
        }
    }
    return ENCIP_SUCCESS;
}

/*
 * @func parse_args parses the application's input argument. 
 * @param int argc is the number of arguments
 * @param IN char* argv[] is the array of input arguments
 * @param OUT char** ifname points to the name of the original input enclave binary file
 * @param OUT char** ofname points to the name of the modified output enclave binary file
 * @param OUT char** kfname points to the name of the input key file
 * @param OUT bool* debug, true if enclave needs to support debug
 * the encrypted enclave binary. 
 * @return encip_ret_e:
 * ENCIP_ERROR_PARSE_INVALID_PARAM if any of the input parameters is NULL
 * ENCIP_ERROR_PARSE_ARGS if input arguments are not supported
 * ENCIP_SUCCESS if success
 */
encip_ret_e parse_args(
                int argc, 
                IN char* argv[], 
                OUT char** ifname, 
                OUT char** ofname, 
                OUT bool* debug)
{
    if(NULL == argv)
        return ENCIP_ERROR_PARSE_INVALID_PARAM;

    char* encip_name = argv[0];

    if((argc != 5 && argc != 6) || 
        NULL == ifname || 
        NULL == ofname)
    {
        print_usage(encip_name);
        return ENCIP_ERROR_PARSE_INVALID_PARAM;
    }
    encip_ret_e ret = ENCIP_SUCCESS;
	
    for(int argidx = 1; argidx < argc; argidx++)
    {
        if(!strcmp(argv[argidx],"-d"))
        {
            *debug = true;
        }
        else if(!strcmp(argv[argidx],"-i") && argidx + 1 < argc)
        {
            argidx++;
            *ifname = argv[argidx];
        }
        else if(!strcmp(argv[argidx],"-o") && argidx + 1 < argc)
        {
            argidx++;
            *ofname = argv[argidx];        }
        else
        {
            ret = ENCIP_ERROR_PARSE_ARGS;
        }
    }
	
    if((ENCIP_SUCCESS != ret) || 
       (NULL == *ifname)      ||
       (NULL == *ofname)) 
    {
        print_usage(encip_name);
        ret = ENCIP_ERROR_PARSE_ARGS;
    }
    return ret;
}

/*
 * @func read_file reads file into buffer. 
 * @param IN const char* const ifname is the input file name
 * @param OUT uint8_t** buf_pp points to the output buffer
 * @param OUT size_t* size_out points to the output data size
 * @return encip_ret_e:
 * ENCIP_ERROR_READF_INVALID_PARAM if any of the input parameters is NULL
 * ENCIP_ERROR_READF_OPEN if unable to open input file
 * ENCIP_ERROR_READF_ALLOC if unable to allocate output buffer
 * ENCIP_ERROR_READF_READ if unable to read file to buffer
 * ENCIP_SUCCESS if success
 */
static encip_ret_e read_file(IN const char* const ifname, OUT uint8_t** buf_pp, OUT size_t* size_out)
{
    if(NULL == ifname || NULL == buf_pp || NULL == size_out)
         return ENCIP_ERROR_READF_INVALID_PARAM;

    FILE* fin = fopen(ifname, "rb");
    if(NULL == fin)
        return ENCIP_ERROR_READF_OPEN;

    fseek(fin,0,SEEK_END);
    size_t const size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    *buf_pp = (uint8_t*)malloc(size);
    if(NULL == *buf_pp)
    {
        fclose(fin);
        return ENCIP_ERROR_MEM_ALLOC;
    }

    size_t const num_bytes = fread(*buf_pp, 1, size, fin);
    if(num_bytes != size)
    {
        fclose(fin);
        free (*buf_pp);
        return ENCIP_ERROR_READF_READ;
    }
    fclose(fin); 
    *size_out = size;   
    return ENCIP_SUCCESS;
}

/*
 * @func write_file writes buffer into file 
 * @param IN char* ofname is the output file name
 * @param IN uint8_t* buf is the input buffer
 * @param size_t size is the size of the buffer
 * @return encip_ret_e:
 * ENCIP_ERROR_WRITEF_INVALID_PARAM if any of the input parameters is NULL
 * ENCIP_ERROR_WRITEF_OPEN if unable to open output file
 * ENCIP_ERROR_WRITEF_WRITE if unable to write buf to file
 * ENCIP_SUCCESS if success
 */
static encip_ret_e write_file(IN const char* const ofname, IN uint8_t* buf, size_t size)
{
    if(NULL == ofname || NULL == buf)
         return ENCIP_ERROR_WRITEF_INVALID_PARAM;
    FILE* fout = fopen(ofname, "wb");
    if(NULL == fout)
        return ENCIP_ERROR_WRITEF_OPEN;

    size_t num_bytes = fwrite(buf, 1, size, fout);
    if(num_bytes != size)
    {
        fclose(fout);
        return ENCIP_ERROR_WRITEF_WRITE;
    }

    fclose(fout);
    return ENCIP_SUCCESS;
}






int main(int argc, IN char *argv[])
{
    char* enclave_in_name = NULL;
    char* enclave_out_name = NULL;
    bool debug = false;
    uint8_t* enclave_buf = NULL;
//    size_t key_size = 0;
    size_t enclave_size = 0;
    // Parse the arguments: 
    encip_ret_e ret = parse_args(argc, argv, &enclave_in_name, &enclave_out_name, &debug);
    if(ENCIP_ERROR(ret))
        return (int)ret;

    // Read enclave file into buffer:
    ret = read_file(enclave_in_name, &enclave_buf, &enclave_size);
    if(ENCIP_ERROR(ret))
        return (int)ret;
    if(0 == enclave_size)
    {
        ret = ENCIP_ERROR_ENCLAVE_SIZE;
        goto Label_free_enclave_buffer;
    }

    // Modify enclave for PCL:
    //ret = encrypt_ip(enclave_buf, enclave_size, key_buf, debug);
    pcl_data_t dat;

	  dat.elf_sec = 0;
	  dat.shstrndx = 0;
	  dat.sections_names = NULL;
	  dat.phdr = NULL;
	  dat.nsections = 0;
	  dat.nsegments = 0;

    ret = parse_elf(enclave_buf, enclave_size, &dat);
    if(ENCIP_ERROR(ret))
      return ret;

	ret = update_flags(0, &dat);
	
    if(ENCIP_ERROR(ret)) 
        goto Label_free_key_and_enclave_buffers;

    // Write the buffer into enclave file:
    ret = write_file(enclave_out_name, enclave_buf, enclave_size);
    if(ENCIP_ERROR(ret))
        goto Label_free_key_and_enclave_buffers;

    // Set success:
    ret = ENCIP_SUCCESS;
printf("Success\n");
Label_free_key_and_enclave_buffers:

Label_free_enclave_buffer:
    free(enclave_buf);

    return (int)ret;
}


