/* Built-in function stress test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* No prototypes for built-ins - we want undeclared references */

/* Helper function that references built-ins without declarations */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference generic built-ins without prototypes */
    if (*state & 1) {
        /* This creates a reference to __builtin_expect */
        if (__builtin_expect(*state > 100, 0)) {
            /* Reference __builtin_trap without prototype */
            __builtin_trap();
        }
    }
    
    if (*state & 2) {
        /* Reference __builtin_constant_p */
        if (!__builtin_constant_p(*state)) {
            /* Reference __builtin_unreachable */
            __builtin_unreachable();
        }
    }
    
    /* Take address of built-in functions */
    void (*trap_ptr)(void) = __builtin_trap;
    void (*unreachable_ptr)(void) = __builtin_unreachable;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(trap_ptr), "r"(unreachable_ptr));
}

/* Another helper for architecture-specific built-ins */
static void __attribute__((noinline)) use_arch_builtins(volatile int *state) {
#ifdef __x86_64__
    /* x86 specific built-in without prototype */
    unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    if (*state & 4) {
        unsigned long long cycles = rdtsc_ptr();
        *state ^= (int)(cycles & 0xFFFFFFFF);
    }
    
    /* Use in inline assembly to create additional references */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif

#ifdef __arm__
    /* ARM specific built-in without prototype */
    unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    if (*state & 8) {
        unsigned int val = rbit_ptr(*state);
        *state ^= val;
    }
    
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif

#ifdef __aarch64__
    /* AArch64 specific built-in */
    unsigned long (*clz_ptr)(unsigned long) = __builtin_aarch64_clz_si;
    if (*state & 16) {
        unsigned long clz_val = clz_ptr(*state);
        *state ^= (int)clz_val;
    }
#endif
}

/* Function that mixes built-in usage patterns */
static int __attribute__((noinline)) builtin_stress_test(volatile int iterations) {
    volatile int state = 42;
    volatile int checksum = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Alternate between different built-in usage patterns */
        switch (state % 5) {
            case 0:
                /* Direct call to undeclared built-in */
                if (__builtin_constant_p(i)) {
                    __builtin_unreachable();
                }
                break;
                
            case 1:
                /* Function pointer to built-in */
                {
                    int (*expect_ptr)(long, int) = __builtin_expect;
                    if (expect_ptr(state, 0)) {
                        __builtin_trap();
                    }
                }
                break;
                
            case 2:
                /* Complex expression with built-in */
                state = __builtin_ffs(state) ^ __builtin_popcount(state);
                break;
                
            case 3:
                /* Use helper functions */
                use_builtins(&state);
                break;
                
            case 4:
                /* Architecture-specific built-ins */
                use_arch_builtins(&state);
                break;
        }
        
        /* Mix in some inline assembly references */
#ifdef __GNUC__
        asm volatile("# Builtin reference %0" : : "i"(__builtin_trap));
        asm volatile("# Another reference %0" : : "i"(__builtin_unreachable));
#endif
        
        /* Update checksum */
        checksum ^= state;
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return checksum;
}

int main(void) {
    volatile int iterations = 100;
    volatile int result = 0;
    
    printf("Starting built-in function stress test...\n");
    
    /* First, create references to various built-ins without prototypes */
    
    /* Generic built-ins */
    void (*trap_func)(void) = __builtin_trap;
    void (*unreachable_func)(void) = __builtin_unreachable;
    int (*expect_func)(long, int) = __builtin_expect;
    int (*constant_p_func)(int) = __builtin_constant_p;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(trap_func), "r"(unreachable_func), 
                 "r"(expect_func), "r"(constant_p_func));
    
    /* Architecture-specific built-ins */
#ifdef __x86_64__
    unsigned long long (*rdtsc_func)(void) = __builtin_ia32_rdtsc;
    asm volatile("" : : "r"(rdtsc_func));
#endif
    
#ifdef __arm__
    unsigned int (*rbit_func)(unsigned int) = __builtin_arm_rbit;
    asm volatile("" : : "r"(rbit_func));
#endif
    
    /* Run the stress test */
    result = builtin_stress_test(iterations);
    
    /* Use result to prevent optimization */
    printf("Result checksum: %d\n", result);
    
    /* Final forced references in main */
    if (result == 0x12345678) {  /* Never true, but compiler doesn't know */
        __builtin_trap();
        __builtin_unreachable();
    }
    
    return result != 0 ? 0 : 1;
}

/* Additional global references to built-ins */
static void * volatile builtin_refs[] = {
#ifdef __GNUC__
    (void *)__builtin_trap,
    (void *)__builtin_unreachable,
    (void *)__builtin_expect,
    (void *)__builtin_constant_p,
#ifdef __x86_64__
    (void *)__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
    (void *)__builtin_arm_rbit,
#endif
#ifdef __aarch64__
    (void *)__builtin_aarch64_clz_si,
#endif
#endif
    NULL
};
