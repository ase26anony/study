/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination matching the uncovered block */
extern int __builtin_hidden_test1(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Variant with different attribute order */
int __builtin_hidden_test2(float) 
    __attribute__((extern, 
                   visibility("hidden"), 
                   used, 
                   artificial));

/* Prototype 3: Without 'extern' but with visibility specified */
static int __builtin_hidden_test3(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

/* Prototype 4: With volatile-like behavior hint */
volatile int __builtin_hidden_test4(int, int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial, 
                   const));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* Function pointer type for volatile calls */
typedef int (*volatile_func_ptr)(int);

/* Array of function pointers to ensure processing */
volatile_func_ptr func_table[4];

/* ============================================
   ARCHITECTURE-SPECIFIC BUILT-INS
   ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

extern int __builtin_ia32_add(int, int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
#endif

#ifdef __x86_64__
/* x86_64 specific built-ins */
extern void __builtin_ia32_lfence(void) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

extern unsigned long long __builtin_ia32_rdtscp(unsigned int*) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
#endif

#ifdef __arm__
/* ARM specific built-ins */
extern unsigned int __builtin_arm_rbit(unsigned int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

extern int __builtin_arm_clz(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
extern unsigned long long __builtin_aarch64_rbitll(unsigned long long) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));
#endif

#ifdef __powerpc__
/* PowerPC specific built-ins */
extern int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));
#endif

/* ============================================
   MAIN FUNCTION WITH VOLATILE ACCESS PATTERN
   ============================================ */

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent behavior */
    int input_value = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int control = input_value;
    
    /* Initialize function pointer table */
    /* These assignments force the compiler to process the declarations */
    
#ifdef __i386__
    func_table[0] = (volatile_func_ptr)__builtin_ia32_add;
    func_table[1] = (volatile_func_ptr)__builtin_ia32_rdtsc;
#elif defined(__x86_64__)
    func_table[0] = (volatile_func_ptr)__builtin_ia32_lfence;
    func_table[1] = (volatile_func_ptr)__builtin_ia32_rdtscp;
#elif defined(__arm__)
    func_table[0] = (volatile_func_ptr)__builtin_arm_clz;
    func_table[1] = (volatile_func_ptr)__builtin_arm_rbit;
#elif defined(__aarch64__)
    func_table[0] = (volatile_func_ptr)__builtin_aarch64_rbitll;
#elif defined(__powerpc__)
    func_table[0] = (volatile_func_ptr)__builtin_ppc_mftb;
#endif
    
    /* Assign prototype function pointers */
    func_table[2] = (volatile_func_ptr)__builtin_hidden_test1;
    func_table[3] = (volatile_func_ptr)__builtin_hidden_test2;
    
    /* Volatile function pointer to prevent optimization */
    volatile_func_ptr volatile_fp = NULL;
    
    /* Input-dependent selection of function pointer */
    if (control % 2 == 0) {
        volatile_fp = func_table[0];
    } else if (control % 3 == 0) {
        volatile_fp = func_table[1];
    } else if (control % 5 == 0) {
        volatile_fp = func_table[2];
    } else {
        volatile_fp = func_table[3];
    }
    
    /* Non-optimizable comparison */
    volatile int comparison_result = 0;
    for (int i = 0; i < 4; i++) {
        if (func_table[i] == volatile_fp) {
            comparison_result = i + 1;
            break;
        }
    }
    
    /* Opaque use of function pointers */
    int result = 0;
    if (comparison_result > 0 && volatile_fp != NULL) {
        /* This call may be optimized away, but the declaration processing
           should still trigger the target hook */
        result = volatile_fp(comparison_result);
    }
    
    /* Additional opaque computation to ensure all paths are considered */
    global_seed = (result ^ input_value) | comparison_result;
    
    /* Loop that references all function pointers */
    for (int i = 0; i < 4; i++) {
        if (func_table[i] != NULL) {
            /* Create artificial dependency */
            global_seed += (long)func_table[i];
        }
    }
    
    /* Return value depends on all the opaque operations */
    return (global_seed & 0xFF) == 0 ? 0 : 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (to satisfy linker if needed)
   ============================================ */

/* These won't be called if built-ins are properly recognized,
   but provide them to avoid linker errors in some configurations */

int __builtin_hidden_test1(int x) {
    return x + 1;
}

int __builtin_hidden_test2(float x) {
    return (int)x * 2;
}

int __builtin_hidden_test3(void) {
    return global_seed;
}

int __builtin_hidden_test4(int a, int b) {
    return a + b;
}
