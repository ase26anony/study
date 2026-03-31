/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   PROTOTYPES WITH VARIOUS ATTRIBUTE COMBINATIONS
   These should trigger the built-in declaration path
   ============================================ */

/* Core prototype with all relevant attributes */
int __hidden_builtin(int x) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Variations with different attribute combinations */
int __hidden_builtin2(int x, int y)
    __attribute__((visibility("hidden"), used));

int __hidden_builtin3(void)
    __attribute__((visibility("hidden"), extern, artificial));

int __hidden_builtin4(float f)
    __attribute__((visibility("hidden"), extern, used, artificial));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), extern, used, artificial));
    
void __builtin_ia32_sfence(void)
    __attribute__((visibility("hidden"), extern, used, artificial));
    
int __builtin_ia32_crc32qi(int crc, char v)
    __attribute__((visibility("hidden"), extern, used, artificial));
#endif

#ifdef __x86_64__
/* x86_64 specific built-ins */
unsigned long long __builtin_ia32_rdtsc(void)
    __attribute__((visibility("hidden"), extern, used, artificial));
    
void __builtin_ia32_mfence(void)
    __attribute__((visibility("hidden"), extern, used, artificial));
    
int __builtin_popcountll(unsigned long long x)
    __attribute__((visibility("hidden"), extern, used, artificial));
#endif

#ifdef __arm__
/* ARM specific built-ins */
unsigned int __builtin_arm_rbit(unsigned int x)
    __attribute__((visibility("hidden"), extern, used, artificial));
    
void __builtin_arm_dmb(unsigned int x)
    __attribute__((visibility("hidden"), extern, used, artificial));
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
unsigned long long __builtin_aarch64_rbitll(unsigned long long x)
    __attribute__((visibility("hidden"), extern, used, artificial));
#endif

#ifdef __powerpc__
/* PowerPC specific built-ins */
int __builtin_ppc_popcntb(unsigned int x)
    __attribute__((visibility("hidden"), extern, used, artificial));
#endif

/* ============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================ */

/* Volatile function pointer to prevent optimization */
typedef int (*volatile_func_ptr)(int);

/* Array of function pointers for iteration */
volatile_func_ptr func_array[10];

/* Opaque operation that compiler can't analyze */
static int opaque_operation(int x) {
    volatile int result = 0;
    for (int i = 0; i < 100; i++) {
        result ^= (x + i) * 7919; /* Prime number for mixing */
    }
    return result;
}

/* ============================================
   MAIN FUNCTION WITH COMPLEX CONTROL FLOW
   ============================================ */

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent control flow */
    int input_value = 0;
    if (argc > 1) {
        input_value = atoi(argv[1]);
        global_seed = input_value;
    }
    
    /* Initialize function pointer array with various targets */
    volatile_func_ptr volatile_fp = NULL;
    
    /* Architecture-specific built-in selection */
#if defined(__i386__) || defined(__x86_64__)
    /* Use x86 built-ins */
    volatile_fp = (volatile_func_ptr)__builtin_ia32_rdtsc;
    func_array[0] = (volatile_func_ptr)__builtin_ia32_rdtsc;
    func_array[1] = (volatile_func_ptr)__hidden_builtin;
#elif defined(__arm__)
    /* Use ARM built-ins */
    volatile_fp = (volatile_func_ptr)__builtin_arm_rbit;
    func_array[0] = (volatile_func_ptr)__builtin_arm_rbit;
    func_array[1] = (volatile_func_ptr)__hidden_builtin;
#elif defined(__aarch64__)
    /* Use AArch64 built-ins */
    volatile_fp = (volatile_func_ptr)__builtin_aarch64_rbitll;
    func_array[0] = (volatile_func_ptr)__builtin_aarch64_rbitll;
    func_array[1] = (volatile_func_ptr)__hidden_builtin;
#elif defined(__powerpc__)
    /* Use PowerPC built-ins */
    volatile_fp = (volatile_func_ptr)__builtin_ppc_popcntb;
    func_array[0] = (volatile_func_ptr)__builtin_ppc_popcntb;
    func_array[1] = (volatile_func_ptr)__hidden_builtin;
#else
    /* Fallback to prototype */
    volatile_fp = __hidden_builtin;
    func_array[0] = __hidden_builtin;
#endif
    
    /* Fill remaining array slots with various prototypes */
    func_array[2] = __hidden_builtin2;
    func_array[3] = __hidden_builtin3;
    func_array[4] = __hidden_builtin4;
    
    /* Complex conditional that can't be optimized away */
    int result = 0;
    volatile int condition = global_seed;
    
    /* Loop with opaque operations to ensure all functions are processed */
    for (int i = 0; i < 5; i++) {
        if (func_array[i] != NULL) {
            /* Create non-optimizable comparison */
            if ((condition & (1 << i)) != 0) {
                /* This should trigger built-in processing */
                result += opaque_operation(i);
                
                /* Store function pointer in volatile variable */
                volatile_func_ptr temp_fp = func_array[i];
                
                /* Compare against known addresses (non-optimizable) */
                if (temp_fp == volatile_fp) {
                    result ^= 0xDEADBEEF;
                }
            }
        }
    }
    
    /* Final conditional using argv to prevent dead code elimination */
    if (argc > 2 && strcmp(argv[2], "test") == 0) {
        /* This path might actually call through the function pointer */
        if (volatile_fp != NULL && (global_seed % 7) == 0) {
            result = volatile_fp(result);
        }
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF; /* Return non-constant value */
}

/* ============================================
   DUMMY IMPLEMENTATIONS (to satisfy linker if needed)
   ============================================ */

/* These won't be called if the built-ins are properly recognized */
int __hidden_builtin(int x) {
    abort(); /* Should never be called */
}

int __hidden_builtin2(int x, int y) {
    return x ^ y;
}

int __hidden_builtin3(void) {
    return global_seed;
}

int __hidden_builtin4(float f) {
    return (int)f;
}
