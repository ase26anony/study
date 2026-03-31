/* 
 * Program to trigger targhooks.cc uncovered lines 981-990
 * Compile with: gcc -O0 -march=native -fdump-tree-original -fvisibility=hidden
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global volatile variable to prevent optimization */
volatile int g_volatile = 0;

/* ============================================
 * PHASE 1: Declare prototypes with target attributes
 * These should mimic built-in function declarations
 * ============================================ */

/* Prototype 1: Full attribute combination matching uncovered lines */
int __attribute__((visibility("hidden"), extern, used, artificial, noinline, noreturn))
__hidden_builtin_full(int x);

/* Prototype 2: Hidden visibility with external linkage */
int __attribute__((visibility("hidden"), extern))
__hidden_builtin_extern(int x);

/* Prototype 3: Hidden visibility with used attribute */
int __attribute__((visibility("hidden"), used))
__hidden_builtin_used(int x);

/* Prototype 4: Hidden visibility with artificial */
int __attribute__((visibility("hidden"), artificial))
__hidden_builtin_artificial(int x);

/* Prototype 5: Just hidden visibility */
int __attribute__((visibility("hidden")))
__hidden_builtin_simple(int x);

/* ============================================
 * PHASE 2: Target-specific built-in declarations
 * Using actual GCC built-ins for different architectures
 * ============================================ */

/* x86/x86_64 specific built-ins */
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)

/* x86 built-in with hidden visibility */
int __attribute__((visibility("hidden"), used, artificial, extern))
__builtin_ia32_rdtsc_hidden(void);

/* Another x86 built-in variant */
void __attribute__((visibility("hidden"), extern, used))
__builtin_ia32_cpuid_hidden(int *eax, int *ebx, int *ecx, int *edx);

/* SSE built-in */
__m128 __attribute__((visibility("hidden"), artificial, extern))
__builtin_ia32_loadups_hidden(const float *p);

#endif /* x86 */

/* ARM specific built-ins */
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)

/* ARM built-in with hidden visibility */
unsigned int __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_arm_rbit_hidden(unsigned int x);

/* ARM barrier built-in */
void __attribute__((visibility("hidden"), extern))
__builtin_arm_dmb_hidden(unsigned int x);

#endif /* ARM */

/* PowerPC specific built-ins */
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)

/* PPC built-in with hidden visibility */
unsigned long __attribute__((visibility("hidden"), extern, used, artificial))
__builtin_ppc_mftb_hidden(void);

#endif /* PowerPC */

/* ============================================
 * PHASE 3: Function pointer array with volatile storage
 * to prevent optimization and ensure processing
 * ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Volatile array of function pointers */
volatile func_ptr_t volatile_funcs[5];

/* Opaque function to prevent optimization */
static int __attribute__((noinline, used))
get_input_value(void) {
    return g_volatile;
}

/* ============================================
 * PHASE 4: Main function with non-optimizable logic
 * ============================================ */

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize volatile global from argv to create runtime dependency */
    if (argc > 1) {
        g_volatile = atoi(argv[1]);
    } else {
        g_volatile = 42; /* Default value */
    }
    
    /* ============================================
     * Initialize function pointer array with addresses
     * This should trigger built-in processing
     * ============================================ */
    
    /* Assign addresses - compiler should process these as built-ins */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_full;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_extern;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_used;
    volatile_funcs[3] = (func_ptr_t)__hidden_builtin_artificial;
    volatile_funcs[4] = (func_ptr_t)__hidden_builtin_simple;
    
    /* ============================================
     * Target-specific built-in usage
     * ============================================ */
    
#if defined(__i386__) || defined(__x86_64__) || defined(__i686__)
    /* Use x86 built-ins */
    volatile unsigned long long tsc;
    int cpuid_eax = 0, cpuid_ebx = 0, cpuid_ecx = 0, cpuid_edx = 0;
    
    /* Take address of x86 built-in */
    volatile void (*volatile_rdtsc)(void) = (void (*)(void))__builtin_ia32_rdtsc_hidden;
    
    /* Call through volatile pointer to prevent optimization */
    if (g_volatile > 100) {
        /* This creates a conditional that can't be resolved at compile time */
        volatile_rdtsc();
    }
    
    /* Another usage pattern */
    __builtin_ia32_cpuid_hidden(&cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    
#endif /* x86 */
    
#if defined(__arm__) || defined(__aarch64__) || defined(__ARM_ARCH)
    /* Use ARM built-ins */
    unsigned int arm_val = 0x12345678;
    unsigned int reversed;
    
    /* Take address of ARM built-in */
    unsigned int (*volatile_rbit)(unsigned int) = __builtin_arm_rbit_hidden;
    
    /* Conditional call */
    if (g_volatile % 2 == 0) {
        reversed = volatile_rbit(arm_val);
        result ^= reversed;
    }
    
#endif /* ARM */
    
    /* ============================================
     * Loop through function pointers with opaque operations
     * Ensures compiler processes all declarations
     * ============================================ */
    
    for (int i = 0; i < 5; i++) {
        /* Create non-optimizable condition */
        if ((g_volatile + i) % 3 == 0) {
            /* Store function pointer in volatile memory */
            volatile func_ptr_t volatile_ptr = volatile_funcs[i];
            
            /* Compare against NULL (can't be optimized away) */
            if (volatile_ptr != NULL) {
                /* Opaque operation on the function address */
                result += (uintptr_t)volatile_ptr & 0xFF;
            }
        }
    }
    
    /* ============================================
     * Additional forced processing through inline assembly
     * ============================================ */
    
    /* Use inline assembly to reference the functions */
    asm volatile (
        "/* Reference to hidden built-in functions */\n\t"
        : /* no output */
        : /* no input */
        : "memory"
    );
    
    /* Final result depends on runtime input */
    return result % 256;
}

/* ============================================
 * PHASE 5: Dummy implementations to satisfy linker
 * In reality, these would be handled by GCC as built-ins
 * ============================================ */

/* These weak implementations ensure the program links */
int __attribute__((weak, visibility("hidden")))
__hidden_builtin_full(int x) {
    return x + 1;
}

int __attribute__((weak, visibility("hidden")))
__hidden_builtin_extern(int x) {
    return x * 2;
}

int __attribute__((weak, visibility("hidden")))
__hidden_builtin_used(int x) {
    return x - 1;
}

int __attribute__((weak, visibility("hidden")))
__hidden_builtin_artificial(int x) {
    return x / 2;
}

int __attribute__((weak, visibility("hidden")))
__hidden_builtin_simple(int x) {
    return x ^ 0xFF;
}
