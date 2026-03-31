/* 
 * Program to trigger built-in function declaration with hidden visibility
 * Target: targhooks.cc lines 981-990
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variable to prevent optimization */
volatile int global_seed = 0;

/* 
 * Prototype 1: Mimic built-in with explicit hidden visibility and all relevant attributes
 * This should trigger the target hook processing
 */
extern int __attribute__((visibility("hidden"), used, artificial, noinline, noclone))
__hidden_builtin_proto(int x) __asm__("__hidden_builtin_impl");

/* 
 * Prototype 2: Another variation with different attribute order
 * The combination of extern + hidden visibility + artificial should reach the target block
 */
extern int __attribute__((artificial, visibility("hidden"), used))
__hidden_builtin_variant(int x, int y);

/*
 * Prototype 3: With volatile qualifier simulation
 * TREE_THIS_VOLATILE flag setting
 */
extern int __attribute__((visibility("hidden"), used, artificial, noinline))
__volatile_builtin_like(void) __asm__("__volatile_builtin");

/* Function to create runtime-dependent condition */
static int get_runtime_value(void) {
    /* Use argv or environment to create runtime dependency */
    extern char **environ;
    static volatile int counter = 0;
    
    /* Mix in some runtime-dependent values */
    if (environ != NULL && *environ != NULL) {
        counter += (int)(*environ)[0];
    }
    return counter & 1;
}

/* 
 * Target-specific built-in declarations
 * These will go through targetm.builtin_decl hook
 */

#ifdef __i386__
/* x86 specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), artificial, used))
__builtin_ia32_sfence(void);

extern int __attribute__((visibility("hidden"), used))
__builtin_ia32_addss(int, int) __asm__("__builtin_ia32_addss");

#elif defined(__x86_64__)
/* x86_64 specific built-ins */
extern long long __attribute__((visibility("hidden"), artificial, used))
__builtin_ia32_rdtsc(void);

extern void __attribute__((visibility("hidden"), used, artificial))
__builtin_ia32_lfence(void);

extern double __attribute__((visibility("hidden"), artificial))
__builtin_ia32_addsd(double, double);

#elif defined(__arm__) || defined(__aarch64__)
/* ARM specific built-ins */
extern int __attribute__((visibility("hidden"), used, artificial))
__builtin_arm_rbit(int);

extern void __attribute__((visibility("hidden"), artificial))
__builtin_arm_dmb(void);

extern int __attribute__((visibility("hidden"), used))
__builtin_arm_clz(int);

#else
/* Generic fallback - declare as regular extern to still test the path */
extern int __attribute__((visibility("hidden"), used, artificial))
__generic_hidden_builtin(void);
#endif

/* Array of function pointers to ensure processing */
typedef int (*func_ptr_t)(void);

/* Volatile function pointer array to prevent optimization */
static volatile func_ptr_t volatile_funcs[4];

/* Opaque operation that compiler can't optimize away */
static void opaque_operation(volatile func_ptr_t fp) {
    /* Create a memory barrier effect */
    asm volatile("" : : "r"(fp) : "memory");
}

int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Initialize with runtime-dependent value */
    global_seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* 
     * Store addresses of built-in-like functions in volatile pointers
     * This forces the compiler to process their declarations
     */
    
    /* Use asm to get function addresses without direct calls */
    void *addr1, *addr2, *addr3;
    
    /* Get addresses (simulated - real built-ins would be resolved by compiler) */
#ifdef __i386__
    asm volatile("movl $__builtin_ia32_rdtsc, %0" : "=r"(addr1));
    asm volatile("movl $__builtin_ia32_sfence, %0" : "=r"(addr2));
#elif defined(__x86_64__)
    asm volatile("movq $__builtin_ia32_rdtsc, %0" : "=r"(addr1));
    asm volatile("movq $__builtin_ia32_lfence, %0" : "=r"(addr2));
#elif defined(__arm__) || defined(__aarch64__)
    asm volatile("mov %0, __builtin_arm_rbit" : "=r"(addr1));
    asm volatile("mov %0, __builtin_arm_dmb" : "=r"(addr2));
#else
    addr1 = (void*)&__generic_hidden_builtin;
    addr2 = (void*)&__hidden_builtin_proto;
#endif
    
    addr3 = (void*)&__hidden_builtin_variant;
    
    /* Store in volatile array */
    volatile_funcs[0] = (func_ptr_t)addr1;
    volatile_funcs[1] = (func_ptr_t)addr2;
    volatile_funcs[2] = (func_ptr_t)addr3;
    volatile_funcs[3] = (func_ptr_t)&__volatile_builtin_like;
    
    /* Create runtime-dependent condition */
    int condition = get_runtime_value();
    if (argc > 1) {
        condition += (int)argv[1][0];
    }
    
    /* 
     * Conditional that can't be resolved at compile time
     * This ensures references to built-ins aren't optimized away
     */
    volatile func_ptr_t selected_func = volatile_funcs[condition & 3];
    
    /* Perform opaque operations with all function pointers */
    for (int i = 0; i < 4; i++) {
        opaque_operation(volatile_funcs[i]);
        
        /* Compare addresses in a way compiler can't optimize */
        if ((void*)volatile_funcs[i] == (void*)selected_func) {
            result += i * 7;
        }
    }
    
    /* 
     * Simulate taking address of built-in-like functions
     * The act of taking address should trigger declaration processing
     */
    void *builtin_addrs[] = {
        (void*)&__hidden_builtin_proto,
        (void*)&__hidden_builtin_variant,
        (void*)&__volatile_builtin_like,
    };
    
    /* Use the addresses in a computation */
    for (size_t i = 0; i < sizeof(builtin_addrs)/sizeof(builtin_addrs[0]); i++) {
        result += ((long)builtin_addrs[i] >> 4) & 0xF;
    }
    
    printf("Result: %d (seed: %d)\n", result, global_seed);
    
    /* 
     * Actual call through volatile pointer if condition is met
     * This may not execute, but the reference is there
     */
    if (global_seed % 7 == 0) {
        /* The cast and call through volatile should preserve the reference */
        int (*volatile fp)(int) = (int (*)(int))selected_func;
        if (fp != NULL) {
            result = fp(result);
        }
    }
    
    return result & 255;
}

/* 
 * Dummy implementations to satisfy linker if built-ins aren't available
 * These won't be called in normal execution due to runtime conditions
 */
int __hidden_builtin_proto(int x) {
    return x * 2;
}

int __hidden_builtin_variant(int x, int y) {
    return x + y;
}

int __volatile_builtin_like(void) {
    return global_seed;
}

/* Generic fallback implementation */
int __generic_hidden_builtin(void) {
    return 42;
}
