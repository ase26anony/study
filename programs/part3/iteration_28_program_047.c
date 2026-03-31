/* Program to trigger built-in function declaration with hidden visibility */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create opaque condition */
static int get_opaque_value(void) {
    return global_seed & 1;
}

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   These should trigger the built-in processing path
   ============================================ */

/* Prototype 1: Full attribute combination */
int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), extern, used, artificial));

/* Prototype 2: Visibility specified with extern */
int __hidden_builtin_2(float) 
    __attribute__((visibility("hidden"), extern));

/* Prototype 3: Used and artificial with hidden visibility */
int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), used, artificial));

/* Prototype 4: Just hidden visibility */
int __hidden_builtin_4(double) 
    __attribute__((visibility("hidden")));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   These will use actual GCC built-ins when available
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)
    /* Use x86-specific built-ins */
    int __builtin_ia32_rdtsc(void) 
        __attribute__((visibility("hidden"), used, artificial));
    
    void __builtin_ia32_sfence(void) 
        __attribute__((visibility("hidden"), extern));
    
    unsigned long long __builtin_ia32_readeflags_u64(void)
        __attribute__((visibility("hidden"), used));
    
    /* MMX/SSE built-ins */
    void __builtin_ia32_emms(void)
        __attribute__((visibility("hidden"), artificial));
    
    /* For x86_64 specifically */
    #ifdef __x86_64__
        void __builtin_ia32_xsave(void*)
            __attribute__((visibility("hidden"), extern, used));
    #endif

/* ARM specific built-ins */
#elif defined(__arm__) || defined(__aarch64__) || defined(__thumb__)
    /* ARM specific built-ins */
    unsigned int __builtin_arm_rbit(unsigned int)
        __attribute__((visibility("hidden"), used, artificial));
    
    void __builtin_arm_dsb(unsigned int)
        __attribute__((visibility("hidden"), extern));
    
    /* For AArch64 */
    #ifdef __aarch64__
        unsigned long long __builtin_aarch64_rbitll(unsigned long long)
            __attribute__((visibility("hidden"), used));
    #endif

/* PowerPC specific built-ins */
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    unsigned int __builtin_ppc_mftb(void)
        __attribute__((visibility("hidden"), used, artificial));
    
    void __builtin_ppc_sync(void)
        __attribute__((visibility("hidden"), extern));

/* Generic fallback - use standard built-ins */
#else
    /* Use some generic built-ins */
    void* __builtin_return_address(unsigned int)
        __attribute__((visibility("hidden"), used, artificial));
    
    int __builtin_clz(unsigned int)
        __attribute__((visibility("hidden"), extern));
#endif

/* ============================================
   MAIN FUNCTION WITH VOLATILE FUNCTION POINTERS
   ============================================ */

int main(int argc, char *argv[]) {
    /* Initialize global seed from argv if available */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    }
    
    /* Array of volatile function pointers to prevent optimization */
    typedef int (*func_ptr_t)(void);
    volatile func_ptr_t func_ptrs[4] = {0};
    
    /* Assign addresses to volatile pointers - compiler can't optimize these away */
    /* Use target-specific built-ins based on architecture */
    
#if defined(__i386__) || defined(__x86_64__)
    /* x86 built-ins */
    func_ptrs[0] = (func_ptr_t)__builtin_ia32_rdtsc;
    func_ptrs[1] = (func_ptr_t)__builtin_ia32_sfence;
    #ifdef __x86_64__
        func_ptrs[2] = (func_ptr_t)__builtin_ia32_readeflags_u64;
    #endif
    
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM built-ins */
    func_ptrs[0] = (func_ptr_t)__builtin_arm_rbit;
    func_ptrs[1] = (func_ptr_t)__builtin_arm_dsb;
    
#elif defined(__powerpc__) || defined(__ppc__)
    /* PowerPC built-ins */
    func_ptrs[0] = (func_ptr_t)__builtin_ppc_mftb;
    func_ptrs[1] = (func_ptr_t)__builtin_ppc_sync;
    
#else
    /* Generic built-ins */
    func_ptrs[0] = (func_ptr_t)__builtin_return_address;
    func_ptrs[1] = (func_ptr_t)__builtin_clz;
#endif
    
    /* Also assign our prototype functions */
    func_ptrs[2] = (func_ptr_t)__hidden_builtin_3;
    
    /* Opaque condition based on input/global state */
    int condition = get_opaque_value();
    
    /* Non-optimizable comparison and potential call */
    volatile func_ptr_t selected_func = func_ptrs[condition % 3];
    
    /* Create a comparison that can't be resolved at compile time */
    if ((intptr_t)selected_func != (intptr_t)&main) {
        /* This condition is opaque to the compiler */
        if (global_seed > 100) {
            /* This path might be taken depending on runtime input */
            int result = 0;
            /* Try to call through the pointer if it's safe */
            if (selected_func != (func_ptr_t)0) {
                /* The call itself may not execute, but the reference exists */
                result = 1;
            }
            return result;
        }
    }
    
    /* Loop to ensure all function pointers are processed */
    for (int i = 0; i < 4; i++) {
        if (func_ptrs[i]) {
            /* Opaque operation on function pointers */
            global_seed += (intptr_t)func_ptrs[i] & 0xFF;
        }
    }
    
    /* Additional reference to prototypes to ensure they're processed */
    volatile int (*proto_refs[4])(void) = {
        (int (*)(void))__hidden_builtin_1,
        (int (*)(void))__hidden_builtin_2,
        (int (*)(void))__hidden_builtin_3,
        (int (*)(void))__hidden_builtin_4
    };
    
    /* Use the references in an opaque way */
    for (int i = 0; i < 4; i++) {
        global_seed += (intptr_t)proto_refs[i] & 0xF;
    }
    
    return global_seed & 0x7F;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (if needed)
   These might be needed to satisfy linker, but the compiler
   should treat them as built-ins before reaching linking stage
   ============================================ */

/* Only define these if we're not using actual built-ins */
#ifndef __GNUC__
int __hidden_builtin_1(int x) { return x + 1; }
int __hidden_builtin_2(float x) { return (int)x; }
int __hidden_builtin_3(void) { return 42; }
int __hidden_builtin_4(double x) { return (int)x; }
#endif
