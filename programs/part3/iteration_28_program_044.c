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
   DECLARATION OF BUILT-IN LIKE FUNCTIONS WITH
   VARIOUS ATTRIBUTE COMBINATIONS
   ============================================ */

/* Prototype 1: Full attribute set matching target block */
extern int __hidden_builtin_1(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline, 
                   noreturn));

/* Prototype 2: Similar but with extern explicitly in attributes */
int __hidden_builtin_2(int) 
    __attribute__((visibility("hidden"), 
                   extern, 
                   used, 
                   artificial));

/* Prototype 3: Without artificial flag */
extern int __hidden_builtin_3(int) 
    __attribute__((visibility("hidden"), 
                   used));

/* Prototype 4: With volatile asm marker */
int __hidden_builtin_4(int) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial, 
                   noinline));

/* ============================================
   TARGET-SPECIFIC BUILT-IN DECLARATIONS
   ============================================ */

#if defined(__i386__) || defined(__x86_64__)
/* x86/x86-64 specific built-ins */
extern long long __builtin_ia32_rdtsc(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern void __builtin_ia32_sfence(void) 
    __attribute__((visibility("hidden"), 
                   used, 
                   artificial));

extern unsigned int __builtin_ia32_crc32qi(unsigned int, unsigned char)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

#elif defined(__arm__) || defined(__aarch64__)
/* ARM/AArch64 specific built-ins */
extern unsigned int __builtin_arm_rbit(unsigned int)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

extern void __builtin_arm_dmb(unsigned int)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
/* PowerPC specific built-ins */
extern unsigned int __builtin_ppc_mftb(void)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

#else
/* Generic fallback - use GCC common built-ins */
extern void *__builtin_alloca(size_t)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));

extern int __builtin_clz(unsigned int)
    __attribute__((visibility("hidden"),
                   used,
                   artificial));
#endif

/* ============================================
   FUNCTION POINTER ARRAY AND RUNTIME LOGIC
   ============================================ */

/* Typedef for function pointers */
typedef int (*func_ptr_t)(int);

/* Volatile function pointer array to prevent optimization */
static volatile func_ptr_t volatile_funcs[4];

/* Opaque initialization that compiler can't analyze */
static void init_func_pointers(void) {
    /* These assignments force the compiler to process declarations */
    volatile_funcs[0] = (func_ptr_t)__hidden_builtin_1;
    volatile_funcs[1] = (func_ptr_t)__hidden_builtin_2;
    volatile_funcs[2] = (func_ptr_t)__hidden_builtin_3;
    volatile_funcs[3] = (func_ptr_t)__hidden_builtin_4;
}

/* Function that creates complex control flow */
static int call_through_pointer(int idx, int value) {
    func_ptr_t fp;
    
    /* Volatile read to prevent optimization */
    fp = volatile_funcs[idx & 3];
    
    /* This call can't be optimized away due to volatile indirection */
    if (fp != 0) {
        return fp(value);
    }
    return 0;
}

/* ============================================
   TARGET-SPECIFIC BUILT-IN USAGE
   ============================================ */

static void use_target_builtins(void) {
    volatile unsigned int result = 0;
    
#if defined(__i386__) || defined(__x86_64__)
    /* Use x86 built-ins */
    result = __builtin_ia32_crc32qi(0x12345678, 0x9A);
    
    /* Take address to force declaration processing */
    void (*sfence_ptr)(void) = __builtin_ia32_sfence;
    if (sfence_ptr) {
        /* Prevent unused variable warning */
        (void)sfence_ptr;
    }
    
#elif defined(__arm__) || defined(__aarch64__)
    /* Use ARM built-ins */
    result = __builtin_arm_rbit(0x12345678);
    
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
    /* Use PowerPC built-ins */
    result = __builtin_ppc_mftb();
    
#else
    /* Use generic built-ins */
    result = __builtin_clz(0x12345678);
#endif
    
    /* Use result in opaque way */
    global_seed ^= result;
}

/* ============================================
   MAIN FUNCTION WITH COMPLEX CONTROL FLOW
   ============================================ */

int main(int argc, char **argv) {
    int i, result = 0;
    
    /* Initialize based on runtime input */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    } else {
        global_seed = 42;
    }
    
    /* Initialize function pointers */
    init_func_pointers();
    
    /* Use target-specific built-ins */
    use_target_builtins();
    
    /* Loop with runtime-dependent iterations */
    int iterations = (global_seed % 4) + 1;
    
    for (i = 0; i < iterations; i++) {
        /* Create complex control flow */
        int idx = (global_seed + i) & 3;
        
        /* Call through volatile function pointer */
        int val = (global_seed * i) & 0xFF;
        result ^= call_through_pointer(idx, val);
        
        /* Mix in target built-in results */
        use_target_builtins();
    }
    
    /* Final opaque computation */
    result = (result ^ global_seed) & 0xFF;
    
    /* Return value depends on all computations */
    return result;
}

/* ============================================
   DUMMY IMPLEMENTATIONS (to satisfy linker)
   ============================================ */

/* These won't be called if built-ins are properly recognized */
int __hidden_builtin_1(int x) {
    return x ^ 0x55;
}

int __hidden_builtin_2(int x) {
    return x + 1;
}

int __hidden_builtin_3(int x) {
    return x * 2;
}

int __hidden_builtin_4(int x) {
    return x >> 1;
}
