/* 
 * This program is designed to trigger GCC's default_builtin_extdecl hook
 * to generate artificial external declarations for built-in functions,
 * specifically aiming to cover the flag-setting block in targhooks.cc.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Helper function that references built-ins without declarations */
__attribute__((noinline))
static void builtin_references(int selector) {
    volatile int v = selector;
    
    /* Reference generic built-ins without prototypes */
    if (v & 1) {
        /* __builtin_expect is commonly used but may not have declaration */
        if (__builtin_expect(v > 100, 0)) {
            /* __builtin_trap without declaration */
            __builtin_trap();
        }
    }
    
    if (v & 2) {
        /* __builtin_constant_p without declaration */
        int is_const = __builtin_constant_p(v);
        use(&is_const);
    }
    
    if (v & 4) {
        /* __builtin_unreachable without declaration */
        if (v < 0) {
            __builtin_unreachable();
        }
    }
    
#ifdef __x86_64__
    if (v & 8) {
        /* x86-specific built-in without declaration */
        unsigned long long tsc = __builtin_ia32_rdtsc();
        use(&tsc);
    }
#endif

#ifdef __arm__
    if (v & 16) {
        /* ARM-specific built-in without declaration */
        unsigned int reversed = __builtin_arm_rbit(v);
        use(&reversed);
    }
#endif

#ifdef __aarch64__
    if (v & 32) {
        /* AArch64-specific built-in */
        unsigned long long reversed = __builtin_aarch64_rbitll(v);
        use(&reversed);
    }
#endif
}

/* Function that takes addresses of built-ins */
__attribute__((noinline))
static void take_builtin_addresses(void) {
    /* Create function pointers to undeclared built-ins */
    void (*trap_ptr)(void) = __builtin_trap;
    void (*unreachable_ptr)(void) = __builtin_unreachable;
    
#ifdef __x86_64__
    unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    use((void*)rdtsc_ptr);
#endif

#ifdef __arm__
    unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    use((void*)rbit_ptr);
#endif

    use((void*)trap_ptr);
    use((void*)unreachable_ptr);
    
    /* Use inline assembly to reference built-in names */
    asm volatile(
        "# References to built-ins\n"
        "1:\n"
        ".ifdef __x86_64__\n"
        ".long __builtin_ia32_rdtsc - 1b\n"
        ".endif\n"
        ".long __builtin_trap - 1b\n"
        ".long __builtin_unreachable - 1b\n"
        : : : "memory"
    );
}

/* Another layer of indirection to increase declaration pressure */
__attribute__((noinline))
static void indirect_builtin_user(int x) {
    volatile int counter = x;
    
    while (counter-- > 0) {
        /* Mix different built-in references in a loop */
        if (counter % 3 == 0) {
            /* This should trigger external declaration creation */
            int result = __builtin_constant_p(counter);
            use(&result);
        }
        
        if (counter % 5 == 0) {
#ifdef __x86_64__
            unsigned long long tsc = __builtin_ia32_rdtsc();
            use(&tsc);
#endif
        }
        
        if (counter % 7 == 0) {
            /* Use inline assembly with built-in reference */
            asm volatile("" : : "i"(__builtin_trap) : "memory");
        }
    }
}

int main(void) {
    volatile int state = 0;
    volatile int iterations = 10;
    int checksum = 0;
    
    /* Initialize with some entropy */
    state = (int)(__builtin_ia32_rdtsc() & 0xFF);
    
    /* Main loop with multiple built-in references */
    for (int i = 0; i < iterations; i++) {
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call helper that references built-ins */
        builtin_references(state);
        
        /* Take addresses of built-ins periodically */
        if (state % 3 == 0) {
            take_builtin_addresses();
        }
        
        /* Use indirect references */
        if (state % 4 == 0) {
            indirect_builtin_user(state % 10);
        }
        
        /* More direct built-in usage without declarations */
        if (state % 11 == 0) {
            /* __builtin_popcount without declaration */
            int popcnt = __builtin_popcount(state);
            checksum += popcnt;
        }
        
        if (state % 13 == 0) {
            /* __builtin_clz without declaration */
            int lz = __builtin_clz(state | 1);
            checksum -= lz;
        }
        
        /* Architecture-specific built-ins */
#ifdef __x86_64__
        if (state % 17 == 0) {
            /* Another x86 built-in */
            unsigned int mxcsr = __builtin_ia32_stmxcsr();
            checksum ^= mxcsr;
        }
#endif
        
#ifdef __powerpc__
        if (state % 19 == 0) {
            /* PowerPC built-in */
            unsigned long tb = __builtin_ppc_get_timebase();
            checksum += (tb & 0xFFFF);
        }
#endif
    }
    
    /* Use checksum to prevent optimization */
    printf("Checksum: %d\n", checksum);
    
    /* Final built-in reference in a different context */
    if (checksum < 0) {
        __builtin_unreachable();
    }
    
    return checksum == 0 ? 0 : 1;
}
