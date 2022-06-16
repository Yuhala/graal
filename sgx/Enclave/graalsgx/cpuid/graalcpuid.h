#ifndef F0973410_BF0E_4EAF_B2A8_5C06997C9E5A
#define F0973410_BF0E_4EAF_B2A8_5C06997C9E5A

#if defined(__cplusplus)
extern "C"
{
#endif


unsigned int get_cpuid_max (unsigned int ext, unsigned int *sig);
int get_cpuid_count (unsigned int leaf, unsigned int subleaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
int get_cpuid (unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx);

#if defined(__cplusplus)
}
#endif


#endif /* F0973410_BF0E_4EAF_B2A8_5C06997C9E5A */
