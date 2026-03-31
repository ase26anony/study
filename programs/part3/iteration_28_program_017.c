/* Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden builtin_test.c */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* ============================================
   DECLARATION OF BUILT-IN LIKE FUNCTIONS WITH
   ATTRIBUTES TO TRIGGER TARGET HOOK LOGIC
   ============================================ */

/* Prototype 1: Full attribute combination targeting the uncovered block */
__attribute__((
    visibility("hidden"),
    extern,
    used,
    artificial,
    noinline,
    noreturn
)) void __hidden_builtin_full(int x);

/* Prototype 2: Variation with fewer attributes */
__attribute__((
    visibility("hidden"),
    used,
    artificial
)) int __hidden_builtin_var1(int a, int b);

/* Prototype 3: Another variation */
__attribute__((
    visibility("hidden"),
    extern,
    artificial
)) float __hidden_builtin_var2(float *f);

/* Prototype 4: Minimal hidden visibility */
__attribute__((visibility("hidden"))) 
void __hidden_builtin_minimal(void);

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)
    /* Use actual GCC x86 built-ins */
    int __builtin_ia32_addss(int, int) 
        __attribute__((visibility("hidden"), used, artificial));
    
    void __builtin_ia32_mfence(void)
        __attribute__((visibility("hidden"), extern, used, artificial));
    
    unsigned long long __builtin_ia32_rdtsc(void)
        __attribute__((visibility("hidden"), artificial));
    
    /* SIMD built-in */
    int __builtin_ia32_paddd128(int, int)
        __attribute__((visibility("hidden"), extern, used));
#endif

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
    /* ARM specific built-ins */
    unsigned int __builtin_arm_rbit(unsigned int)
        __attribute__((visibility("hidden"), used, artificial));
    
    void __builtin_arm_dmb(unsigned int)
        __attribute__((visibility("hidden"), extern, artificial));
    
    /* NEON built-in */
    int __builtin_arm_vaddvq_s32(int)
        __attribute__((visibility("hidden"), used));
#endif

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    unsigned int __builtin_ppc_mftb(void)
        __attribute__((visibility("hidden"), artificial));
    
    void __builtin_ppc_sync(void)
        __attribute__((visibility("hidden"), extern, used, artificial));
#endif

/* Generic fallback if no architecture-specific built-ins are available */
#ifndef TARGET_BUILTIN_DEFINED
#define TARGET_BUILTIN_DEFINED
/* Create a dummy built-in-like function */
static void __dummy_builtin(void) 
    __attribute__((visibility("hidden"), extern, used, artificial, noinline));
#endif

/* ============================================
   VOLATILE FUNCTION POINTERS TO PREVENT OPTIMIZATION
   ============================================ */

/* Array of volatile function pointers */
typedef void (*volatile func_ptr_void_t)(void);
typedef int (*volatile func_ptr_int_t)(int, int);

volatile func_ptr_void_t volatile_funcs[4];
volatile func_ptr_int_t volatile_int_funcs[2];

/* Opaque operation that compiler can't analyze */
static int opaque_condition(int seed) {
    return (seed * 1103515245 + 12345) & 0x7fffffff;
}

/* ============================================
   MAIN FUNCTION WITH NON-OPTIMIZABLE LOGIC
   ============================================ */

int main(int argc, char *argv[]) {
    /* Use argv to create input-dependent condition */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int condition = opaque_condition(seed);
    
    /* Initialize volatile function pointers with built-in addresses */
    
    /* For x86/x86_64 */
    #if defined(__i386__) || defined(__x86_64__)
        volatile_funcs[0] = (func_ptr_void_t)__builtin_ia32_mfence;
        volatile_int_funcs[0] = (func_ptr_int_t)__builtin_ia32_addss;
    #endif
    
    /* For ARM */
    #if defined(__arm__) || defined(__aarch64__)
        volatile_funcs[0] = (func_ptr_void_t)__builtin_arm_dmb;
    #endif
    
    /* For PowerPC */
    #if defined(__powerpc__) || defined(__ppc__)
        volatile_funcs[0] = (func_ptr_void_t)__builtin_ppc_sync;
    #endif
    
    /* Always use our declared prototypes */
    volatile_funcs[1] = (func_ptr_void_t)__hidden_builtin_full;
    volatile_funcs[2] = (func_ptr_void_t)__hidden_builtin_minimal;
    
    /* Create a non-optimizable conditional that references built-ins */
    int result = 0;
    
    /* Loop that forces compiler to process all function declarations */
    for (int i = 0; i < 3; i++) {
        if (volatile_funcs[i] != NULL) {
            /* Create side effect to prevent dead code elimination */
            global_seed += (uintptr_t)volatile_funcs[i];
            
            /* Conditional call based on opaque condition */
            if ((condition >> i) & 0x1) {
                /* The compiler can't eliminate this at compile time */
                if (i == 0) {
                    /* Call through volatile pointer */
                    #if defined(__i386__) || defined(__x86_64__)
                        if (volatile_int_funcs[0] != NULL) {
                            result += volatile_int_funcs[0](i, condition);
                        }
                    #endif
                }
            }
        }
    }
    
    /* Another opaque use of function addresses */
    if ((global_seed & 1) && (condition > 1000)) {
        /* Force compiler to consider function addresses as used */
        volatile uintptr_t addr1 = (uintptr_t)__hidden_builtin_var1;
        volatile uintptr_t addr2 = (uintptr_t)__hidden_builtin_var2;
        
        if (addr1 != addr2) {
            result += (int)(addr1 - addr2);
        }
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d (seed: %d)\n", result, seed);
    
    return result != 0 ? 0 : 1;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (NEVER CALLED)
   These exist only to satisfy linker in case
   the built-ins aren't properly recognized
   ============================================ */

void __hidden_builtin_full(int x) {
    /* Should never be called - just for declaration */
    abort();
}

int __hidden_builtin_var1(int a, int b) {
    return a + b;
}

float __hidden_builtin_var2(float *f) {
    return *f;
}

void __hidden_builtin_minimal(void) {
    /* Empty */
}

#ifndef TARGET_BUILTIN_DEFINED
void __dummy_builtin(void) {
    /* Dummy implementation */
}
#endif
