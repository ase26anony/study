/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    return global_seed & 1;
}

/* ============================================
   DECLARE PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
    __hidden_builtin_1(int x);

/* Prototype 2: Hidden visibility with extern */
extern int __attribute__((visibility("hidden")))
    __hidden_builtin_2(int x, int y);

/* Prototype 3: Hidden + used + artificial */
extern int __attribute__((visibility("hidden"), used, artificial))
    __hidden_builtin_3(void);

/* Prototype 4: Just hidden visibility */
int __attribute__((visibility("hidden")))
    __hidden_builtin_4(long x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
extern long long __attribute__((visibility("hidden")))
    __builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used))
    __builtin_ia32_sfence(void);

extern unsigned int __attribute__((visibility("hidden"), artificial))
    __builtin_ia32_crc32qi(unsigned int, unsigned char);

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ia32_addcarryx_u32(unsigned char, unsigned int, 
                                 unsigned int, unsigned int *);

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
extern unsigned int __attribute__((visibility("hidden")))
    __builtin_arm_rbit(unsigned int);

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_arm_clz(int);

extern void __attribute__((visibility("hidden")))
    __builtin_arm_dmb(unsigned int);

extern unsigned long long __attribute__((visibility("hidden"), artificial))
    __builtin_arm_rbitll(unsigned long long);

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
extern int __attribute__((visibility("hidden")))
    __builtin_ppc_popcntb(unsigned int);

extern double __attribute__((visibility("hidden"), used))
    __builtin_ppc_fabs(double);

extern int __attribute__((visibility("hidden"), artificial))
    __builtin_ppc_isync(void);

#else
/* Generic fallback built-in declarations */
extern void __attribute__((visibility("hidden")))
    __builtin_trap(void);

extern int __attribute__((visibility("hidden"), used, artificial))
    __builtin_ffs(int);

extern void __attribute__((visibility("hidden")))
    __builtin_prefetch(const void *, ...);
#endif

/* ============================================
   FUNCTION POINTER ARRAY FOR OPAQUE OPERATIONS
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);
typedef void (*void_func_ptr_t)(void);

/* Array of function pointers with volatile storage */
static volatile func_ptr_t func_array[4] = {
    (func_ptr_t)__hidden_builtin_1,
    (func_ptr_t)__hidden_builtin_2,
    (func_ptr_t)__hidden_builtin_3,
    (func_ptr_t)__hidden_builtin_4
};

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    volatile int result = 0;
    volatile int i, j;
    
    /* Use argv to create runtime-dependent condition */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 12345;
    }
    
    /* Create a volatile function pointer */
    volatile void_func_ptr_t volatile_fp = NULL;
    
#if defined(__i386__) || defined(__x86_64__)
    /* Assign target-specific built-in to volatile pointer */
    volatile_fp = (void_func_ptr_t)__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__)
    volatile_fp = (void_func_ptr_t)__builtin_arm_rbit;
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    volatile_fp = (void_func_ptr_t)__builtin_ppc_popcntb;
#else
    volatile_fp = (void_func_ptr_t)__builtin_trap;
#endif
    
    /* Non-optimizable comparison */
    if (get_runtime_value()) {
        /* This path uses the volatile function pointer */
        if (volatile_fp != NULL) {
            /* Create artificial use that can't be optimized away */
            result = (int)((long)volatile_fp & 0xFF);
        }
    }
    
    /* Loop through function pointer array with opaque operations */
    for (i = 0; i < 4; i++) {
        volatile func_ptr_t current_fp = func_array[i];
        
        /* Create conditional that can't be resolved at compile time */
        if ((global_seed + i) % 3 == 0) {
            /* Compare function pointers (can't be optimized away) */
            if (current_fp == (func_ptr_t)__hidden_builtin_1) {
                result += 1;
            }
        } else if ((global_seed + i) % 5 == 0) {
            if (current_fp != NULL) {
                result += 2;
            }
        }
        
        /* Additional opaque computation */
        for (j = 0; j < 2; j++) {
            result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    /* Use result in a way that can't be optimized out */
    if (result == 0x12345678) {
        /* This should never happen, but prevents dead code elimination */
        printf("Impossible condition!\n");
    }
    
    return result & 0xFF;
}

/* ============================================
   DUMMY DEFINITIONS TO SATISFY LINKER
   (These won't be used when built-ins are available)
   ============================================ */

#ifdef FORCE_DEFINITIONS
int __hidden_builtin_1(int x) { return x + 1; }
int __hidden_builtin_2(int x, int y) { return x + y; }
int __hidden_builtin_3(void) { return global_seed; }
int __hidden_builtin_4(long x) { return (int)x; }
#endif
