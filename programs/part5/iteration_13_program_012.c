/* Built-in function declaration stress test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile void *volatile_func_ptr = NULL;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline, cold))
static void use_builtin_variants(int selector) {
    volatile int local_volatile = selector;
    
    /* Reference various built-ins without declarations */
    if (local_volatile & 1) {
        /* Force reference to common built-ins */
        void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
        if (local_volatile > 100) {
            trap_ptr();  /* Call through pointer */
        }
    }
    
    if (local_volatile & 2) {
        /* Reference unreachable built-in */
        void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
        /* Don't actually call it to avoid UB during normal execution */
    }
    
    if (local_volatile & 4) {
        /* Reference expectation built-in */
        long (*expect_ptr)(long, long) = (long (*)(long, long))__builtin_expect;
        long result = expect_ptr(local_volatile, 0);
        (void)result;
    }
}

/* Another helper with architecture-specific built-ins */
__attribute__((noinline))
static void use_arch_builtins(void) {
    volatile int guard = global_counter;
    
#ifdef __x86_64__
    /* x86 specific built-ins */
    if (guard & 1) {
        unsigned long long (*rdtsc_ptr)(void) = 
            (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        /* Store pointer to prevent optimization */
        volatile_func_ptr = (void *)rdtsc_ptr;
    }
    
    if (guard & 2) {
        /* More x86 built-ins */
        void (*pause_ptr)(void) = (void (*)(void))__builtin_ia32_pause;
        /* Reference in asm to create additional pressure */
        asm volatile("" : : "i"(__builtin_ia32_pause));
    }
#endif

#ifdef __arm__
    /* ARM specific built-ins */
    if (guard & 4) {
        unsigned int (*rbit_ptr)(unsigned int) = 
            (unsigned int (*)(unsigned int))__builtin_arm_rbit;
        volatile_func_ptr = (void *)rbit_ptr;
    }
#endif

#ifdef __aarch64__
    /* AArch64 specific built-ins */
    if (guard & 8) {
        unsigned long (*clz_ptr)(unsigned long) = 
            (unsigned long (*)(unsigned long))__builtin_aarch64_clz;
        volatile_func_ptr = (void *)clz_ptr;
    }
#endif
}

/* Function that takes address of built-ins and calls them indirectly */
__attribute__((noinline, hot))
static int call_builtin_indirectly(int value) {
    volatile int result = value;
    
    /* Create function pointers to undeclared built-ins */
    int (*constant_p_ptr)(int) = (int (*)(int))__builtin_constant_p;
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    
    /* Use built-in constant check */
    if (constant_p_ptr(result)) {
        result *= 2;
    }
    
    /* Store pointers to prevent optimization */
    static void *stored_ptrs[4];
    static int ptr_index = 0;
    
    stored_ptrs[ptr_index++ & 3] = (void *)constant_p_ptr;
    stored_ptrs[ptr_index++ & 3] = (void *)trap_ptr;
    
    /* Conditional call through pointer */
    if (result > 1000) {
        /* This path shouldn't normally execute */
        trap_ptr();
    }
    
    return result;
}

/* Complex control flow with built-in references */
__attribute__((noinline))
static void builtin_control_flow(volatile int iterations) {
    for (volatile int i = 0; i < iterations; i++) {
        volatile int state = global_counter + i;
        
        /* Different paths reference different built-ins */
        switch (state & 7) {
            case 0:
                /* Reference popcount built-in */
                {
                    int (*popcount_ptr)(unsigned int) = 
                        (int (*)(unsigned int))__builtin_popcount;
                    int count = popcount_ptr(state);
                    (void)count;
                }
                break;
                
            case 1:
                /* Reference bswap built-in */
                {
                    unsigned int (*bswap_ptr)(unsigned int) = 
                        (unsigned int (*)(unsigned int))__builtin_bswap32;
                    unsigned int swapped = bswap_ptr(state);
                    (void)swapped;
                }
                break;
                
            case 2:
                /* Reference prefetch built-in */
                {
                    void (*prefetch_ptr)(const void *, ...) = 
                        (void (*)(const void *, ...))__builtin_prefetch;
                    prefetch_ptr(&state, 0, 3);
                }
                break;
                
            case 3:
                /* Use inline asm to reference built-in */
                asm volatile("" : : "i"(__builtin_unreachable));
                break;
                
            case 4:
                /* Reference ffs built-in */
                {
                    int (*ffs_ptr)(int) = (int (*)(int))__builtin_ffs;
                    int pos = ffs_ptr(state);
                    (void)pos;
                }
                break;
                
            case 5:
                /* Reference clz built-in */
                {
                    int (*clz_ptr)(unsigned int) = 
                        (int (*)(unsigned int))__builtin_clz;
                    int leading_zeros = clz_ptr(state);
                    (void)leading_zeros;
                }
                break;
                
            case 6:
                /* Reference ctz built-in */
                {
                    int (*ctz_ptr)(unsigned int) = 
                        (int (*)(unsigned int))__builtin_ctz;
                    int trailing_zeros = ctz_ptr(state);
                    (void)trailing_zeros;
                }
                break;
                
            case 7:
                /* Reference parity built-in */
                {
                    int (*parity_ptr)(unsigned int) = 
                        (int (*)(unsigned int))__builtin_parity;
                    int parity = parity_ptr(state);
                    (void)parity;
                }
                break;
        }
        
        /* Mix with architecture-specific built-ins */
        use_arch_builtins();
        
        /* Update global state */
        global_counter += (state & 1);
    }
}

int main(void) {
    volatile int iterations = 100;
    volatile int checksum = 0;
    
    printf("Starting built-in declaration stress test...\n");
    
    /* Phase 1: Direct and indirect built-in references */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call helper with built-in function pointers */
        use_builtin_variants(i);
        
        /* Call built-ins indirectly */
        checksum += call_builtin_indirectly(i);
        
        /* Reference target-specific built-ins via asm */
#ifdef __x86_64__
        asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
        
#ifdef __arm__
        asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
        
        /* Reference generic built-ins in asm */
        asm volatile("" : : "i"(__builtin_expect));
        asm volatile("" : : "i"(__builtin_constant_p));
        
        /* Occasionally take address of trap built-in */
        if ((i & 15) == 0) {
            void (*trap_addr)(void) = (void (*)(void))__builtin_trap;
            volatile_func_ptr = (void *)trap_addr;
        }
    }
    
    /* Phase 2: Complex control flow with built-in references */
    builtin_control_flow(iterations / 2);
    
    /* Phase 3: Additional architecture-specific built-in references */
#ifdef __GNUC__
    {
        /* Create array of built-in function pointers */
        void *builtin_ptrs[] = {
            (void *)__builtin_trap,
            (void *)__builtin_unreachable,
            (void *)__builtin_expect,
            (void *)__builtin_constant_p,
            (void *)__builtin_popcount,
            (void *)__builtin_bswap32,
#ifdef __x86_64__
            (void *)__builtin_ia32_rdtsc,
            (void *)__builtin_ia32_pause,
#endif
#ifdef __arm__
            (void *)__builtin_arm_rbit,
#endif
        };
        
        /* Use the pointers to prevent optimization */
        for (size_t j = 0; j < sizeof(builtin_ptrs)/sizeof(builtin_ptrs[0]); j++) {
            checksum += (long)builtin_ptrs[j] & 0xFF;
        }
    }
#endif
    
    /* Final checksum computation and output */
    checksum += global_counter;
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* One final reference to an uncommon built-in */
    {
        float (*fabsf_ptr)(float) = (float (*)(float))__builtin_fabsf;
        float result = fabsf_ptr(-3.14f);
        (void)result;
    }
    
    return checksum != 0 ? 0 : 1;
}
