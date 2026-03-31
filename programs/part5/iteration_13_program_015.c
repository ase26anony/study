/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#ifdef __GNUC__
/* We intentionally do NOT include any headers that declare built-ins */

/* Helper function to create additional scopes for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference built-ins without declaration */
    if (*state & 1) {
        /* Force reference to common built-ins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    }
    
    /* Architecture-specific built-ins */
#ifdef __x86_64__
    /* x86 specific built-in - will need external declaration */
    if (*state & 2) {
        volatile unsigned long long ts;
        /* Call without declaration */
        ts = __builtin_ia32_rdtsc();
        *state ^= (int)(ts & 0xFFFFFFFF);
    }
#endif
    
#ifdef __arm__
    /* ARM specific built-in */
    if (*state & 4) {
        volatile unsigned int val = *state;
        /* Call without declaration */
        val = __builtin_arm_rbit(val);
        *state ^= val;
    }
#endif
    
#ifdef __aarch64__
    /* AArch64 specific built-in */
    if (*state & 8) {
        volatile unsigned long val = *state;
        /* Call without declaration */
        val = __builtin_aarch64_rbit(val);
        *state ^= (int)val;
    }
#endif
}

/* Another helper to take addresses of built-ins */
static void __attribute__((noinline)) take_builtin_address(volatile int *state) {
    /* Function pointers to undeclared built-ins */
    void (*trap_ptr)(void);
    void (*unreachable_ptr)(void);
    
    /* Take addresses without declarations */
    trap_ptr = (void (*)(void))__builtin_trap;
    unreachable_ptr = (void (*)(void))__builtin_unreachable;
    
    /* Store pointers in volatile to prevent optimization */
    volatile void *volatile ptr_storage[2];
    ptr_storage[0] = (void *)trap_ptr;
    ptr_storage[1] = (void *)unreachable_ptr;
    
    /* Mix with inline assembly referencing built-ins */
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
#endif
    
    /* Use state to prevent dead code elimination */
    *state ^= (int)((long)ptr_storage[0] ^ (long)ptr_storage[1]);
}

int main(void) {
    volatile int state = 0x12345678;
    volatile int counter = 0;
    volatile int checksum = 0;
    
    /* Function pointer for dynamic built-in calls */
    void (*volatile builtin_func)(void) = 0;
    
    /* Loop with complex control flow around built-in calls */
    for (counter = 0; counter < 10; counter++) {
        /* Different paths trigger different built-ins */
        if (state & (1 << (counter % 8))) {
            /* Call built-in without declaration */
            if (counter % 3 == 0) {
                __builtin_trap();  /* May not return, but that's OK */
            } else if (counter % 3 == 1) {
                __builtin_unreachable();
            }
        } else {
            /* Use helper functions */
            use_builtins(&state);
            take_builtin_address(&state);
            
            /* Target-specific built-in via function pointer */
#ifdef __x86_64__
            if (counter % 5 == 0) {
                /* Take address and call through pointer */
                unsigned long long (*rdtsc_ptr)(void);
                rdtsc_ptr = (unsigned long long (*)(void))__builtin_ia32_rdtsc;
                volatile unsigned long long ts = rdtsc_ptr();
                state ^= (int)(ts & 0xFFFF);
            }
#endif
            
            /* More inline assembly pressure */
            asm volatile("# Built-in reference marker %0" : : "i"(__builtin_expect) :);
            asm volatile("# Another marker %0" : : "i"(__builtin_constant_p) :);
        }
        
        /* Update checksum */
        checksum ^= state + counter;
        
        /* Prevent infinite loops if __builtin_trap is optimized */
        if (counter > 100) break;
    }
    
    /* Additional built-in references in different contexts */
    {
        /* Nested block with its own references */
        volatile int local_state = checksum;
        
        /* Reference multiple built-ins */
        (void)__builtin_popcount(local_state);
        (void)__builtin_ffs(local_state);
        (void)__builtin_clz(local_state);
        (void)__builtin_ctz(local_state);
        
        /* More architecture-specific */
#ifdef __GNUC__
        (void)__builtin_cpu_supports("sse2");
        (void)__builtin_cpu_supports("avx2");
#endif
        
        checksum += local_state;
    }
    
    /* Final observable side effect */
    volatile int result = checksum ^ state;
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    /* Simple output to verify execution */
    if (result != 0) {
        return 0;  /* Normal exit */
    }
    
    return 1;  /* Shouldn't reach here */
}

#else
/* Non-GCC compilers */
int main(void) {
    return 0;
}
#endif
