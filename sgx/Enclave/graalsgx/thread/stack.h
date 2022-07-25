#ifndef B2A84EE9_BBB6_4CAE_9D53_93D76C449A43
#define B2A84EE9_BBB6_4CAE_9D53_93D76C449A43


#define CALL_STACK_MAXlEN 64
#define STACK_HIGH 0XFFFFFFFF

#define STACK_MIN 4096

#define PG_SZ 4096


#if defined(__cplusplus)
extern "C"
{
#endif

    void set_stack_address();
    void *get_stack_address(unsigned int level);
    void *get_stack_ptr();
    void *get_r15_register();
    void *get_stack_base();
    void *allocate_stack(size_t size_mb);
    
#if defined(__cplusplus)
}
#endif

#endif /* B2A84EE9_BBB6_4CAE_9D53_93D76C449A43 */
