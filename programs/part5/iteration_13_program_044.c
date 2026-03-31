/* Built-in function reference test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static void use_result(volatile int result) {
    printf("Result: %d\n", result);
}

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) reference_builtins(volatile int *state) {
    /* Reference various built-ins without prototypes */
    if (*state & 1) {
        /* Use __builtin_expect without declaration */
        if (__builtin_expect(*state > 100, 0)) {
            /* Reference __builtin_trap without declaration */
            __builtin_trap();
        }
    } else {
        /* Reference __builtin_constant_p without declaration */
        if (!__builtin_constant_p(*state)) {
            /* Reference __builtin_unreachable without declaration */
            __builtin_unreachable();
        }
    }
    
    /* Take address of architecture-specific built-ins */
#ifdef __x86_64__
    /* x86 specific built-in without declaration */
    void (*rdtsc_ptr)(void) = (void (*)(void))__builtin_ia32_rdtsc;
    /* Create additional reference via inline asm */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM specific built-in without declaration */
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
    
    /* Reference generic built-ins */
    int (*expect_ptr)(long, int) = (int (*)(long, int))__builtin_expect;
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Call through function pointers */
    if (*state & 2) {
        int r = expect_ptr(*state, 0);
        *state += r;
    }
}

/* Another helper with different control flow */
static void __attribute__((noinline)) complex_builtin_usage(volatile int *counter) {
    for (int i = 0; i < 3; i++) {
        switch (*counter % 4) {
            case 0:
                /* Reference __builtin_popcount without declaration */
                *counter += __builtin_popcount(*counter);
                break;
            case 1:
                /* Reference __builtin_clz without declaration */
                *counter += __builtin_clz(*counter);
                break;
            case 2:
                /* Reference __builtin_ctz without declaration */
                *counter += __builtin_ctz(*counter);
                break;
            case 3:
                /* Reference __builtin_ffs without declaration */
                *counter += __builtin_ffs(*counter);
                break;
        }
        
        /* Mix with CPU-specific built-ins */
#ifdef __GNUC__
        /* Reference __builtin_cpu_supports without declaration */
        if (__builtin_cpu_supports("sse2")) {
            (*counter)++;
        }
        
        /* Reference __builtin_cpu_init without declaration */
        __builtin_cpu_init();
#endif
    }
}

int main(void) {
    volatile int state = 42;
    volatile int checksum = 0;
    
    /* Loop with complex control flow around built-in references */
    for (volatile int i = 0; i < 5; i++) {
        /* Reference built-ins in different contexts */
        reference_builtins(&state);
        
        /* More built-in references in conditionals */
        if (state % 3 == 0) {
            /* Reference __builtin_abs without declaration */
            state = __builtin_abs(state);
        } else if (state % 3 == 1) {
            /* Reference __builtin_sqrt without declaration */
            double (*sqrt_ptr)(double) = (double (*)(double))__builtin_sqrt;
            state += (int)sqrt_ptr((double)state);
        } else {
            /* Reference __builtin_return_address without declaration */
            void *ra = __builtin_return_address(0);
            state += (ra != 0);
        }
        
        /* Complex builtin usage in nested loop */
        complex_builtin_usage(&state);
        
        /* Additional architecture-specific built-ins */
#ifdef __i386__
        /* Reference x86 specific built-in without declaration */
        void (*cpuid_ptr)(int, int*, int*, int*, int*) = 
            (void (*)(int, int*, int*, int*, int*))__builtin_ia32_cpuid;
        asm volatile("" : : "i"(__builtin_ia32_cpuid));
#elif defined(__powerpc__)
        /* PowerPC specific built-in */
        unsigned int (*cntlzw_ptr)(unsigned int) = 
            (unsigned int (*)(unsigned int))__builtin_cntlzw;
        asm volatile("" : : "i"(__builtin_cntlzw));
#endif
        
        checksum += state;
        state = (state * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final built-in references */
    if (checksum > 1000) {
        /* Reference __builtin_prefetch without declaration */
        __builtin_prefetch(&checksum, 0, 0);
    }
    
    /* Use function pointers to built-ins one more time */
    {
        int (*parity_ptr)(unsigned int) = (int (*)(unsigned int))__builtin_parity;
        unsigned int (*bswap32_ptr)(unsigned int) = 
            (unsigned int (*)(unsigned int))__builtin_bswap32;
        
        checksum ^= parity_ptr(checksum);
        checksum ^= bswap32_ptr(checksum);
    }
    
    use_result(checksum);
    return checksum != 0 ? 0 : 1;
}
