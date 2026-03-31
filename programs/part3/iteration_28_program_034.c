/* Built-in function visibility test program */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF PROTOTYPES WITH TARGET ATTRIBUTES
   ============================================ */

/* Prototype 1: Full attribute combination matching uncovered lines */
extern int __attribute__((visibility("hidden"), used, artificial))
__hidden_builtin_proto(int x) __asm__("__hidden_builtin");

/* Prototype 2: Variant with extern linkage explicitly specified */
extern int __attribute__((visibility("hidden"), extern, used, artificial))
__hidden_builtin_extern(int x);

/* Prototype 3: Without artificial flag (different code path) */
extern int __attribute__((visibility("hidden"), used))
__hidden_builtin_noartificial(int x);

/* Prototype 4: With noinline to ensure function pointer works */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__hidden_builtin_noinline(int x);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#ifdef __i386__
/* x86 specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_cpuid(int regs[4], int leaf);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_addss(int a, int b);
#endif

#ifdef __x86_64__
/* x86_64 specific built-ins */
extern long long __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_cpuid(int regs[4], int leaf);

extern double __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_addsd(double a, double b);
#endif

#ifdef __arm__
/* ARM specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(int x);

extern unsigned int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_clz(unsigned int x);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_dmb(void);
#endif

#ifdef __aarch64__
/* AArch64 specific built-ins */
extern unsigned long long __attribute__((visibility("hidden"), used, artificial))
__builtin_aarch64_rbitll(unsigned long long x);

extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_aarch64_cls_si(int x);
#endif

#ifdef __powerpc__
/* PowerPC specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_popcntb(int x);

extern double __attribute__((visibility("hidden"), used, artificial))
__builtin_ppc_fabs(double x);
#endif

/* ============================================
   FUNCTION POINTER ARRAY AND OPAQUE OPERATIONS
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Volatile function pointer array to prevent optimization */
volatile func_ptr_t func_array[8] = {0};

/* Opaque operation that compiler can't analyze */
static int opaque_operation(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

/* Function to initialize function pointers based on architecture */
static void init_function_pointers(void) {
    int i = 0;
    
    /* Use the prototypes as function pointers */
    func_array[i++] = (func_ptr_t)__hidden_builtin_proto;
    func_array[i++] = (func_ptr_t)__hidden_builtin_extern;
    func_array[i++] = (func_ptr_t)__hidden_builtin_noartificial;
    func_array[i++] = (func_ptr_t)__hidden_builtin_noinline;
    
    /* Add target-specific built-ins based on architecture */
    #if defined(__i386__) || defined(__x86_64__)
    func_array[i++] = (func_ptr_t)__builtin_ia32_rdtsc;
    #elif defined(__arm__)
    func_array[i++] = (func_ptr_t)__builtin_arm_rbit;
    #elif defined(__aarch64__)
    func_array[i++] = (func_ptr_t)__builtin_aarch64_rbitll;
    #elif defined(__powerpc__)
    func_array[i++] = (func_ptr_t)__builtin_ppc_popcntb;
    #endif
}

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    volatile int result = 0;
    volatile func_ptr_t volatile_fp = NULL;
    
    /* Initialize based on command line to prevent compile-time resolution */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 42;
    }
    
    /* Initialize function pointers */
    init_function_pointers();
    
    /* Select function pointer based on input (non-optimizable) */
    int index = opaque_operation(global_seed) % 8;
    volatile_fp = func_array[index];
    
    /* Non-optimizable comparison with NULL */
    if (volatile_fp != NULL) {
        /* Try to call through the pointer if it's not NULL */
        if (index < 4) {
            /* Call with prototype-compatible signature */
            result = ((func_ptr_t)volatile_fp)(global_seed);
        } else {
            /* For target built-ins, we need to handle different signatures */
            #if defined(__i386__) || defined(__x86_64__)
            if (volatile_fp == (func_ptr_t)__builtin_ia32_rdtsc) {
                /* This would need proper casting for the actual return type */
                result = 1;
            }
            #endif
        }
    }
    
    /* Loop through all function pointers for additional processing */
    for (int i = 0; i < 8; i++) {
        if (func_array[i] != NULL) {
            /* Opaque operation on function pointer addresses */
            result ^= (int)((long long)func_array[i] & 0xFFFFFFFF);
        }
    }
    
    /* Use result in a way that can't be optimized away */
    if (result == 0x12345678) {
        printf("Impossible condition\n");
    }
    
    return result & 0xFF;
}

/* ============================================
   DUMMY IMPLEMENTATIONS TO SATISFY LINKER
   (These won't be used if built-ins are properly recognized)
   ============================================ */

int __hidden_builtin_proto(int x) {
    return x + 1;
}

int __hidden_builtin_extern(int x) {
    return x * 2;
}

int __hidden_builtin_noartificial(int x) {
    return x - 1;
}

int __hidden_builtin_noinline(int x) {
    return x ^ 0x55;
}
