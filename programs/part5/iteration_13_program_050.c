/* Built-in function reference test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Helper function that references built-ins without declarations */
__attribute__((noinline))
static int helper_function(volatile int x) {
    /* Reference generic built-ins without prototypes */
    if (x > 100) {
        /* Force generation of external declaration for __builtin_trap */
        __builtin_trap();
    } else if (x > 50) {
        /* Force generation for __builtin_unreachable */
        __builtin_unreachable();
    }
    
    /* Use __builtin_expect without declaration */
    if (__builtin_expect(x > 10, 1)) {
        /* Use __builtin_constant_p without declaration */
        if (!__builtin_constant_p(x)) {
            x = x * 2;
        }
    }
    
    return x;
}

/* Another helper that takes addresses of built-ins */
__attribute__((noinline))
static void take_addresses(void) {
    /* Function pointers to force declaration generation */
    void (*trap_ptr)(void);
    void (*unreachable_ptr)(void);
    
    /* Take addresses without declarations */
    trap_ptr = __builtin_trap;
    unreachable_ptr = __builtin_unreachable;
    
    /* Use the pointers to prevent optimization */
    use((void*)trap_ptr);
    use((void*)unreachable_ptr);
    
#ifdef __x86_64__
    /* x86-specific built-in without declaration */
    unsigned long long (*rdtsc_ptr)(void);
    rdtsc_ptr = __builtin_ia32_rdtsc;
    use((void*)rdtsc_ptr);
    
    /* Call through pointer */
    volatile unsigned long long cycles = rdtsc_ptr();
    use((void*)(uintptr_t)cycles);
#endif

#ifdef __arm__
    /* ARM-specific built-in without declaration */
    unsigned int (*rbit_ptr)(unsigned int);
    rbit_ptr = __builtin_arm_rbit;
    use((void*)rbit_ptr);
#endif

#ifdef __aarch64__
    /* AArch64-specific built-in */
    unsigned long (*a64_rbit_ptr)(unsigned long);
    aarch64_rbit_ptr = __builtin_aarch64_rbit;
    use((void*)aarch64_rbit_ptr);
#endif
}

/* Use inline assembly to reference built-in names */
static void asm_references(void) {
    /* Reference built-in names in assembly constraints */
    asm volatile(
        "# Force reference to builtins\n"
        "1:\n"
        : 
        : "i"(__builtin_trap), "i"(__builtin_unreachable)
        : "memory"
    );
    
#ifdef __x86_64__
    asm volatile(
        ""
        : 
        : "i"(__builtin_ia32_rdtsc)
        : "memory"
    );
#endif
}

int main(void) {
    volatile int state = 0;
    volatile int iterations = 5;
    int checksum = 0;
    
    /* Loop with complex control flow */
    for (volatile int i = 0; i < iterations; i++) {
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Different paths based on volatile state */
        if (state & 0x1) {
            /* Call helper which references built-ins */
            checksum += helper_function(state & 0xFF);
        } else if (state & 0x2) {
            /* Direct built-in call without declaration */
            if ((state & 0xFF) > 200) {
                __builtin_trap();
            } else {
                __builtin_unreachable();
            }
        } else if (state & 0x4) {
            /* Use __builtin_popcount without declaration */
            checksum += __builtin_popcount(state);
        } else {
            /* Use __builtin_ffs without declaration */
            checksum += __builtin_ffs(state);
        }
        
        /* Take addresses of built-ins periodically */
        if ((state & 0x3) == 0) {
            take_addresses();
        }
        
        /* Use assembly references */
        if ((state & 0x7) == 0) {
            asm_references();
        }
        
        /* Target-specific built-in calls */
#ifdef __x86_64__
        if (state & 0x8) {
            /* Call x86 built-in without declaration */
            volatile unsigned long long tsc = __builtin_ia32_rdtsc();
            checksum ^= (int)(tsc & 0xFFFFFFFF);
        }
#endif
        
#ifdef __arm__
        if (state & 0x8) {
            /* Call ARM built-in without declaration */
            volatile unsigned int val = __builtin_arm_rbit(state);
            checksum ^= val;
        }
#endif
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* Use __builtin_abs without declaration */
    checksum = __builtin_abs(checksum);
    
    /* Final output to prevent elimination */
    printf("Checksum: %d\n", checksum);
    
    /* One more built-in reference at the end */
    if (checksum < 0) {
        __builtin_trap();
    }
    
    return checksum == 0 ? 0 : 1;
}
