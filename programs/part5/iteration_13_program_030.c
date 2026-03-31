/* Built-in function declaration stress test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Helper function to create additional scope for built-in references */
__attribute__((noinline, cold))
static void builtin_stress_test(volatile int *state) {
    /* Reference built-ins without declarations */
    if (*state & 1) {
        /* Force creation of __builtin_trap declaration */
        void (*trap_ptr)(void) = __builtin_trap;
        if (*state & 2) {
            trap_ptr();
        }
    }
    
    if (*state & 4) {
        /* Force creation of __builtin_unreachable declaration */
        void (*unreachable_ptr)(void) = __builtin_unreachable;
        /* Use in asm to create additional reference */
        asm volatile("" : : "i"(__builtin_unreachable));
    }
}

/* Another helper with architecture-specific built-ins */
__attribute__((noinline))
static void arch_builtin_test(volatile int *state) {
#ifdef __x86_64__
    /* x86 specific built-in without declaration */
    unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    if (*state & 8) {
        unsigned long long cycles = rdtsc_ptr();
        use(&cycles);
    }
    
    /* More x86 built-ins */
    void (*pause_ptr)(void) = __builtin_ia32_pause;
    asm volatile("" : : "i"(__builtin_ia32_pause));
#endif

#ifdef __arm__
    /* ARM specific built-in */
    unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    if (*state & 16) {
        unsigned int val = rbit_ptr(0x12345678);
        use(&val);
    }
#endif

#ifdef __aarch64__
    /* AArch64 built-in */
    unsigned long (*clz_ptr)(unsigned long) = __builtin_aarch64_clz;
    asm volatile("" : : "i"(__builtin_aarch64_clz));
#endif
}

/* Test generic built-ins */
__attribute__((noinline))
static void generic_builtin_test(volatile int *state) {
    /* __builtin_expect without declaration */
    long (*expect_ptr)(long, long) = __builtin_expect;
    if (*state & 32) {
        long result = expect_ptr(*state, 0);
        use(&result);
    }
    
    /* __builtin_constant_p without declaration */
    int (*constant_p_ptr)(...) = __builtin_constant_p;
    asm volatile("" : : "i"(__builtin_constant_p));
    
    /* __builtin_popcount without declaration */
    int (*popcount_ptr)(unsigned int) = __builtin_popcount;
    if (*state & 64) {
        int count = popcount_ptr(0xFFFFFFFF);
        use(&count);
    }
    
    /* __builtin_bswap without declaration */
    unsigned int (*bswap_ptr)(unsigned int) = __builtin_bswap32;
    asm volatile("" : : "i"(__builtin_bswap32));
}

int main(void) {
    volatile int state = 0;
    volatile int iteration = 0;
    int checksum = 0;
    
    /* Initialize with unpredictable value */
    asm volatile("" : "=r"(state) : "0"(0));
    
    /* Loop creating multiple references to undeclared built-ins */
    for (iteration = 0; iteration < 10; iteration++) {
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call helper functions that reference built-ins */
        builtin_stress_test(&state);
        arch_builtin_test(&state);
        generic_builtin_test(&state);
        
        /* Direct references in main loop */
        if (state & 128) {
            /* __builtin_prefetch without declaration */
            void (*prefetch_ptr)(const void *, ...) = __builtin_prefetch;
            char buffer[64];
            prefetch_ptr(buffer, 0, 3);
        }
        
        if (state & 256) {
            /* __builtin_memcpy without declaration */
            void *(*memcpy_ptr)(void *, const void *, size_t) = __builtin_memcpy;
            char src[16], dst[16];
            memcpy_ptr(dst, src, sizeof(src));
        }
        
        /* Mix with some declared functions to avoid all calls being optimized */
        checksum += state & 0xFF;
        
        /* Prevent tail call optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Use __builtin_abs without declaration in final computation */
    int (*abs_ptr)(int) = __builtin_abs;
    checksum = abs_ptr(checksum);
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* One more built-in reference at the end */
    void (*trap_final)(void) = __builtin_trap;
    asm volatile("" : : "i"(__builtin_trap));
    
    return checksum > 100 ? 0 : 1;
}

/* Additional global scope references to built-ins */
static void *global_builtin_refs[] = {
    (void *)__builtin_trap,
    (void *)__builtin_unreachable,
#ifdef __x86_64__
    (void *)__builtin_ia32_rdtsc,
#endif
    (void *)__builtin_expect,
    (void *)__builtin_constant_p,
    NULL
};
