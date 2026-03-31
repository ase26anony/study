/* Built-in function reference test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of variables */
static volatile int global_counter = 0;
static volatile int use_builtin = 1;

/* Helper function that references built-ins without declarations */
__attribute__((noinline))
static void use_undeclared_builtins(int selector) {
    volatile int local = selector;
    
    /* Reference generic built-ins without prototypes */
    if (local & 1) {
        /* __builtin_expect without declaration */
        if (__builtin_expect(local > 100, 0)) {
            /* __builtin_trap without declaration */
            __builtin_trap();
        }
    }
    
    if (local & 2) {
        /* __builtin_constant_p without declaration */
        if (!__builtin_constant_p(local)) {
            /* __builtin_unreachable without declaration */
            __builtin_unreachable();
        }
    }
    
    /* Architecture-specific built-in references */
#ifdef __x86_64__
    if (local & 4) {
        /* x86 specific built-in without declaration */
        unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
        volatile unsigned long long ts = rdtsc_ptr();
        global_counter += (int)(ts & 0xFF);
    }
#endif
    
#ifdef __arm__
    if (local & 8) {
        /* ARM specific built-in without declaration */
        unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
        volatile unsigned int val = rbit_ptr(0x12345678);
        global_counter += (int)(val & 0xFF);
    }
#endif
    
    /* Additional generic built-in reference */
    if (local & 16) {
        /* __builtin_popcount without declaration */
        volatile int pop = __builtin_popcount(local);
        global_counter += pop;
    }
}

/* Function that takes addresses of built-ins */
__attribute__((noinline))
static void take_builtin_addresses(void) {
    /* Function pointers to undeclared built-ins */
    void (*trap_ptr)(void) = __builtin_trap;
    void (*unreachable_ptr)(void) = __builtin_unreachable;
    
    /* Store pointers in volatile to prevent optimization */
    volatile long addr1 = (long)trap_ptr;
    volatile long addr2 = (long)unreachable_ptr;
    
    /* Use inline assembly to reference built-in names */
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
    
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
    
    /* Mix with arithmetic to create complex control flow */
    global_counter += (int)((addr1 ^ addr2) & 0xFF);
}

/* Main function with complex control flow */
int main(void) {
    volatile int i, result = 0;
    
    printf("Starting built-in reference test...\n");
    
    /* Loop with volatile condition to prevent optimization */
    for (i = 0; i < 100; i++) {
        volatile int selector = global_counter + i;
        
        /* Complex conditional calling of helper */
        if (selector % 3 == 0) {
            use_undeclared_builtins(selector);
        } else if (selector % 3 == 1) {
            take_builtin_addresses();
        } else {
            /* Direct built-in call in another path */
            if (selector > 50) {
                /* __builtin_expect without declaration */
                if (__builtin_expect(selector < 100, 1)) {
                    /* Reference CPU built-in if available */
#ifdef __GNUC__
                    volatile int cpu_supports = 0;
                    /* __builtin_cpu_supports without declaration */
                    if (__builtin_cpu_supports("sse2")) {
                        cpu_supports = 1;
                    }
                    global_counter += cpu_supports;
#endif
                }
            }
        }
        
        /* Additional built-in reference in loop */
        if (selector % 7 == 0) {
            /* __builtin_ffs without declaration */
            volatile int ffs_result = __builtin_ffs(selector);
            global_counter += ffs_result;
        }
        
        /* Prevent infinite loops with __builtin_unreachable */
        if (selector < 0) {
            __builtin_unreachable();
        }
    }
    
    /* Compute checksum from volatile state */
    result = global_counter & 0xFF;
    
    /* Use built-in in final computation */
    result = __builtin_popcount(result);
    
    printf("Result checksum: %d\n", result);
    
    /* Final trap if result is unexpected */
    if (result == 0) {
        __builtin_trap();
    }
    
    return result;
}

/* Additional function in different compilation unit context */
__attribute__((noinline, used))
static void another_builtin_user(void) {
    /* Different set of built-in references */
    volatile int x = 42;
    
    /* __builtin_clz without declaration */
    volatile int clz = __builtin_clz(x);
    
    /* __builtin_ctz without declaration */
    volatile int ctz = __builtin_ctz(x);
    
    /* Use in arithmetic */
    global_counter += clz - ctz;
    
    /* Architecture-specific with inline assembly */
#ifdef __x86_64__
    {
        unsigned long long (*rdtscp_ptr)(void) = __builtin_ia32_rdtscp;
        volatile unsigned long long ts = rdtscp_ptr();
        global_counter += (int)(ts & 0xFF);
    }
#endif
}
