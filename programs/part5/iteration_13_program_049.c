/* Built-in function declaration stress test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* No prototypes for built-ins - we want the compiler to create them */

/* Helper function that references built-ins without declarations */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference generic built-ins without prototypes */
    if (*state & 1) {
        /* This should trigger creation of external declaration */
        __builtin_expect(*state, 0);
    }
    
    if (*state & 2) {
        /* Another built-in reference */
        __builtin_constant_p(*state);
    }
    
#ifdef __GNUC__
    /* Architecture-specific built-ins */
#ifdef __x86_64__
    /* x86 specific built-in - compiler should create external decl */
    void (*rdtsc_ptr)(void) = (void (*)(void))__builtin_ia32_rdtsc;
    /* Use inline assembly to reference the built-in */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM specific built-in */
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
#endif
    
    /* More generic built-in references */
    if (*state > 100) {
        /* These should force external declaration creation */
        __builtin_trap();
    } else if (*state < 0) {
        __builtin_unreachable();
    }
}

/* Another helper with different built-in usage pattern */
static int __attribute__((noinline)) check_builtins(volatile int val) {
    int result = 0;
    
    /* Take address of built-in functions */
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Use function pointers conditionally */
    if (val % 3 == 0) {
        /* Call through pointer - compiler needs declaration */
        if (trap_ptr) {
            /* Reference only, don't actually call trap */
            asm volatile("" : : "r"(trap_ptr));
        }
        result += __builtin_popcount(val);
    }
    
    if (val % 5 == 0) {
        if (unreachable_ptr) {
            asm volatile("" : : "r"(unreachable_ptr));
        }
        result += __builtin_clz(val);
    }
    
    /* CPU feature detection built-in without prototype */
#ifdef __GNUC__
    if (val % 7 == 0) {
        /* This built-in often requires external declaration */
        int cpu_supports = __builtin_cpu_supports("sse2");
        result += cpu_supports;
    }
#endif
    
    return result;
}

int main(void) {
    volatile int state = 0;
    volatile int checksum = 0;
    volatile int i;
    
    /* Initialize with random-ish value */
    state = (int)(__builtin_return_address(0) & 0xFF);
    
    /* Complex loop with multiple built-in references */
    for (i = 0; i < 10; i++) {
        state ^= i * 37;
        
        /* Call helper that references undeclared built-ins */
        use_builtins(&state);
        
        /* Direct built-in calls without prototypes */
        if (state % 4 == 0) {
            /* Force compiler to handle __builtin_unreachable */
            if (state > 1000) {
                __builtin_unreachable();
            }
            checksum += __builtin_ffs(state);
        }
        
        if (state % 6 == 0) {
            /* Force compiler to handle __builtin_trap */
            if (state < -1000) {
                __builtin_trap();
            }
            checksum += __builtin_ctz(state);
        }
        
        /* More architecture-specific built-in references */
#ifdef __GNUC__
#ifdef __x86_64__
        /* Reference x86 built-in in loop */
        unsigned long long (*rdtsc_ptr)(void) = 
            (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        asm volatile("" : : "r"(rdtsc_ptr));
#elif defined(__i386__)
        unsigned long long (*rdtsc_ptr)(void) = 
            (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        asm volatile("" : : "r"(rdtsc_ptr));
#endif
#endif
        
        /* Call second helper */
        checksum += check_builtins(state);
        
        /* Modify state to create different code paths */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use synchronization built-in without prototype */
    __sync_synchronize();
    
    /* Final built-in references */
    checksum += __builtin_popcount(checksum);
    checksum ^= __builtin_bswap32(checksum);
    
    /* Print result to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* One more built-in at the end */
    if (checksum == 0x12345678) {
        __builtin_trap();
    }
    
    return checksum & 0xFF;
}

/* Additional global scope references to built-ins */
#ifdef __GNUC__
/* These global pointers force the compiler to handle built-in addresses */
static void * const builtin_refs[] = {
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
    0
};
#endif
