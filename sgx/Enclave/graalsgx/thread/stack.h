#ifndef B2A84EE9_BBB6_4CAE_9D53_93D76C449A43
#define B2A84EE9_BBB6_4CAE_9D53_93D76C449A43

#if defined(__cplusplus)
extern "C"
{
#endif

    void set_stack_address();
    void *get_stack_address(unsigned int level);
    void *get_stack_ptr();
    void *get_r15_register();
    void *get_stack_bottom();
    
#if defined(__cplusplus)
}
#endif

#endif /* B2A84EE9_BBB6_4CAE_9D53_93D76C449A43 */
