/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create opaque use of function pointers */
static int opaque_use(void *ptr) {
    return (int)((long)ptr & 1);
}

/* ============================================= */
/* PROTOTYPES WITH VISIBILITY AND LINKAGE ATTRIBUTES */
/* ============================================= */

/* Prototype 1: Full attribute combination */
int __hidden_builtin_1(int x) 
    __attribute__((visibility("hidden"), extern, used, artificial));

/* Prototype 2: Hidden visibility with used */
int __hidden_builtin_2(int x, int y) 
    __attribute__((visibility("hidden"), used));

/* Prototype 3: Hidden visibility with artificial */
int __hidden_builtin_3(void) 
    __attribute__((visibility("hidden"), artificial, extern));

/* Prototype 4: Just hidden visibility */
int __hidden_builtin_4(int *ptr) 
    __attribute__((visibility("hidden")));

/* Prototype 5: Hidden with static to test different path */
static int __hidden_builtin_5(int x) 
    __attribute__((visibility("hidden"), used, artificial));

/* ============================================= */
/* TARGET-SPECIFIC BUILT-IN DECLARATIONS */
/* ============================================= */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i386) || defined(__amd64__)

/* Use actual GCC x86 built-ins */
int __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), used, artificial, extern));
    
int __builtin_ia32_crc32qi(int crc, char v) 
    __attribute__((visibility("hidden"), used, artificial));

void __builtin_ia32_mfence(void) 
    __attribute__((visibility("hidden"), artificial, extern));

unsigned int __builtin_ia32_bsrsi(unsigned int) 
    __attribute__((visibility("hidden"), used));

/* Store in volatile function pointers to prevent optimization */
volatile void *volatile_fptr1 = (void *)__builtin_ia32_rdtsc;
volatile void *volatile_fptr2 = (void *)__builtin_ia32_crc32qi;

#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__thumb__)

int __builtin_arm_rbit(int x) 
    __attribute__((visibility("hidden"), used, artificial, extern));

int __builtin_arm_clz(int x) 
    __attribute__((visibility("hidden"), artificial));

void __builtin_arm_dmb(void) 
    __attribute__((visibility("hidden"), used, extern));

/* Store in volatile function pointers */
volatile void *volatile_fptr1 = (void *)__builtin_arm_rbit;
volatile void *volatile_fptr2 = (void *)__builtin_arm_clz;

#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

int __builtin_ppc_mftb(void) 
    __attribute__((visibility("hidden"), used, artificial, extern));

int __builtin_ppc_popcntb(int x) 
    __attribute__((visibility("hidden"), artificial));

/* Store in volatile function pointers */
volatile void *volatile_fptr1 = (void *)__builtin_ppc_mftb;
volatile void *volatile_fptr2 = (void *)__builtin_ppc_popcntb;

#endif

/* Generic fallback if no target-specific built-ins available */
#if !defined(__i386__) && !defined(__x86_64__) && !defined(__arm__) && \
    !defined(__aarch64__) && !defined(__powerpc__) && !defined(__ppc__)

/* Use standard built-ins with attributes */
void *__builtin_return_address(int level) 
    __attribute__((visibility("hidden"), used, artificial, extern));

int __builtin_popcount(unsigned int x) 
    __attribute__((visibility("hidden"), artificial));

/* Store in volatile function pointers */
volatile void *volatile_fptr1 = (void *)__builtin_return_address;
volatile void *volatile_fptr2 = (void *)__builtin_popcount;

#endif

/* ============================================= */
/* FUNCTION POINTER ARRAY FOR OPAQUE USE */
/* ============================================= */

/* Array of function pointers to ensure processing */
typedef int (*func_ptr_t)(int);

/* Volatile array to prevent optimization */
static volatile func_ptr_t func_array[] = {
    (func_ptr_t)__hidden_builtin_1,
    (func_ptr_t)__hidden_builtin_2,
    (func_ptr_t)__hidden_builtin_3,
    (func_ptr_t)__hidden_builtin_4,
    (func_ptr_t)__hidden_builtin_5,
    NULL
};

/* ============================================= */
/* MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC */
/* ============================================= */

int main(int argc, char **argv) {
    int result = 0;
    
    /* Use argv to create non-constant condition */
    int use_builtin = (argc > 1 && argv[1][0] != '\0') ? 1 : 0;
    
    /* Volatile variable to prevent compile-time resolution */
    volatile int volatile_choice = use_builtin;
    
    /* Create a non-optimizable conditional */
    if (volatile_choice) {
        /* Use target-specific built-in through volatile pointer */
#if defined(__i386__) || defined(__x86_64__)
        int (*fptr)(void) = (int (*)(void))volatile_fptr1;
        result = fptr();
#elif defined(__arm__) || defined(__aarch64__)
        int (*fptr)(int) = (int (*)(int))volatile_fptr1;
        result = fptr(global_seed);
#elif defined(__powerpc__) || defined(__ppc__)
        int (*fptr)(void) = (int (*)(void))volatile_fptr1;
        result = fptr();
#else
        int (*fptr)(unsigned int) = (int (*)(unsigned int))volatile_fptr2;
        result = fptr(global_seed);
#endif
    } else {
        /* Use our hidden prototype functions */
        for (int i = 0; i < 5; i++) {
            if (func_array[i]) {
                /* Opaque use that compiler can't eliminate */
                result += opaque_use((void *)func_array[i]);
            }
        }
    }
    
    /* Additional opaque operations with function pointers */
    volatile void *compare_ptr = NULL;
    
    /* Compare against known addresses (non-optimizable) */
    if (global_seed == 0) {
        compare_ptr = (void *)__hidden_builtin_1;
    } else if (global_seed == 1) {
        compare_ptr = volatile_fptr1;
    }
    
    /* Force evaluation by using in arithmetic */
    result += (int)((long)compare_ptr & 255);
    
    /* Loop with opaque use of all function pointers */
    for (int i = 0; func_array[i] != NULL && i < 10; i++) {
        /* Create artificial dependency on global_seed */
        if ((global_seed + i) % 2 == 0) {
            result += opaque_use((void *)func_array[i]);
        }
    }
    
    /* Return result based on function pointer addresses */
    return result % 256;
}

/* ============================================= */
/* DUMMY IMPLEMENTATIONS TO SATISFY LINKER */
/* ============================================= */

/* These won't be called if built-ins are properly recognized,
   but provide implementations to avoid linker errors */

int __hidden_builtin_1(int x) {
    return x + 1;
}

int __hidden_builtin_2(int x, int y) {
    return x + y;
}

int __hidden_builtin_3(void) {
    return global_seed;
}

int __hidden_builtin_4(int *ptr) {
    return ptr ? *ptr : 0;
}

static int __hidden_builtin_5(int x) {
    return x * 2;
}
