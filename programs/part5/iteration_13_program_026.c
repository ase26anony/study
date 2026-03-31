/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* No built-in prototypes included - we want undeclared references */

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference various built-ins without declaration */
    if (*state & 1) {
        /* Force reference to common built-ins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    }
    
    if (*state & 2) {
        /* Reference trap/unreachable built-ins */
        if (*state > 100) {
            __builtin_trap();  /* No prototype */
        } else {
            __builtin_unreachable();  /* No prototype */
        }
    }
}

/* Another helper with function pointer manipulation */
static void __attribute__((noinline)) builtin_function_pointers(volatile int *state) {
    /* Take addresses of built-ins without declaration */
    void (*trap_ptr)(void) = __builtin_trap;
    void (*unreachable_ptr)(void) = __builtin_unreachable;
    
    /* Store in volatile to prevent optimization */
    volatile void *volatile ptr_storage;
    
    if (*state & 4) {
        ptr_storage = (void *)trap_ptr;
    } else {
        ptr_storage = (void *)unreachable_ptr;
    }
    
    /* Prevent dead code elimination */
    (void)ptr_storage;
}

/* Target-specific built-in references */
static void target_specific_builtins(volatile int *state) {
#ifdef __x86_64__
    /* x86 specific built-ins */
    if (*state & 8) {
        /* Reference without prototype */
        unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
        volatile unsigned long long result = rdtsc_ptr();
        (void)result;
        
        /* Inline asm reference to the same built-in */
        asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    }
#endif

#ifdef __arm__
    /* ARM specific built-ins */
    if (*state & 16) {
        unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
        volatile unsigned int val = 0x12345678;
        volatile unsigned int result = rbit_ptr(val);
        (void)result;
    }
#endif

#ifdef __aarch64__
    /* AArch64 specific built-ins */
    if (*state & 32) {
        unsigned long (*cls_ptr)(unsigned long) = __builtin_aarch64_clsdi;
        volatile unsigned long result = cls_ptr(0xFFFFFFFF);
        (void)result;
    }
#endif
}

/* Complex control flow with built-in references */
static int complex_builtin_usage(volatile int iterations) {
    volatile int state = 42;
    volatile int checksum = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary the state to take different paths */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call helper functions that reference built-ins */
        use_builtins(&state);
        builtin_function_pointers(&state);
        target_specific_builtins(&state);
        
        /* Direct built-in calls in complex conditions */
        if (state % 3 == 0) {
            /* Use __builtin_expect without prototype */
            if (__builtin_expect((state & 0xFF) > 128, 0)) {
                checksum += 1;
            }
        } else if (state % 3 == 1) {
            /* Use __builtin_constant_p without prototype */
            if (!__builtin_constant_p(state)) {
                checksum += 2;
            }
        } else {
            /* Mix with inline assembly */
            asm volatile(
                "/* Force reference to builtin */"
                : 
                : "i"(__builtin_trap), "i"(__builtin_unreachable)
                : "memory"
            );
            checksum += 3;
        }
        
        /* Additional architecture-specific paths */
#ifdef __GNUC__
        /* Generic GCC built-ins */
        if (state % 7 == 0) {
            volatile int popcnt = __builtin_popcount(state);
            checksum += popcnt;
        }
        
        if (state % 11 == 0) {
            volatile long long bswap = __builtin_bswap64(state);
            checksum += (int)(bswap & 0xFF);
        }
#endif
    }
    
    return checksum;
}

int main(void) {
    volatile int iterations = 100;
    volatile int seed = 0;
    
    /* Initialize volatile state from environment or random source */
    char *env_iter = getenv("BUILTIN_TEST_ITERATIONS");
    if (env_iter) {
        iterations = atoi(env_iter);
        if (iterations < 1) iterations = 1;
        if (iterations > 10000) iterations = 10000;
    }
    
    char *env_seed = getenv("BUILTIN_TEST_SEED");
    if (env_seed) {
        seed = atoi(env_seed);
    }
    
    /* Use seed to vary execution paths */
    srand(seed);
    
    printf("Testing built-in function declaration generation...\n");
    printf("Iterations: %d, Seed: %d\n", iterations, seed);
    
    /* Main test with complex control flow */
    int result = complex_builtin_usage(iterations);
    
    printf("Checksum result: %d\n", result);
    
    /* Final forced references to ensure all built-ins are touched */
    volatile void *final_refs[] = {
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
    };
    
    /* Prevent optimization */
    asm volatile("" : : "r"(final_refs) : "memory");
    
    return (result > 0) ? 0 : 1;
}
