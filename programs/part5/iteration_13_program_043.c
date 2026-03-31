/* 
 * Program to trigger default_builtin_extdecl in targhooks.cc
 * Forces generation of artificial external declarations for built-in functions
 */

#ifdef __GNUC__

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference various built-ins without prototypes */
    if (*state & 1) {
        /* Call undeclared generic built-ins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    }
    
    if (*state & 2) {
        /* Call undeclared control flow built-ins */
        if (*state > 100) {
            __builtin_unreachable();
        } else if (*state < 0) {
            __builtin_trap();
        }
    }
    
    /* Take address of built-ins - forces declaration creation */
    void (*fp1)(void) = (void (*)(void))__builtin_trap;
    void (*fp2)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(fp1), "r"(fp2));
}

/* Another helper with architecture-specific built-ins */
static void __attribute__((noinline)) use_arch_builtins(volatile int *state) {
#ifdef __x86_64__
    /* x86 specific built-ins */
    unsigned long long (*rdtsc_ptr)(void) = 
        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    
    if (*state & 4) {
        unsigned long long cycles = rdtsc_ptr();
        *state ^= (cycles & 0xFF);
    }
    
    /* Inline asm referencing built-in */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif

#ifdef __arm__
    /* ARM specific built-ins */
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    
    if (*state & 8) {
        unsigned int val = rbit_ptr(*state);
        *state ^= val;
    }
#endif

#ifdef __aarch64__
    /* AArch64 specific built-ins */
    unsigned long (*clz_ptr)(unsigned long) = 
        (unsigned long (*)(unsigned long))__builtin_aarch64_clz;
    
    if (*state & 16) {
        unsigned long count = clz_ptr(*state);
        *state += count;
    }
#endif
}

/* Main function with complex control flow */
int main(void) {
    volatile int state = 42;
    volatile int counter = 0;
    volatile int checksum = 0;
    
    /* Function pointer to undeclared built-in */
    void (*volatile builtin_fp)(void) = 0;
    
    /* Loop with multiple paths referencing built-ins */
    for (counter = 0; counter < 10; counter++) {
        /* Different paths based on volatile state */
        if (state & (1 << (counter % 5))) {
            /* Path 1: Call undeclared built-in directly */
            if (state > 50) {
                __builtin_unreachable();
            } else {
                /* This creates a reference to __builtin_trap */
                builtin_fp = (void (*)(void))__builtin_trap;
            }
        } else {
            /* Path 2: Use __builtin_expect without declaration */
            int likely = __builtin_expect(state & 1, 1);
            state ^= likely;
        }
        
        /* Call helper functions */
        use_builtins(&state);
        use_arch_builtins(&state);
        
        /* Mix with inline assembly */
        asm volatile(
            "# Reference built-ins in assembly\n\t"
            : 
            : "i"(__builtin_expect), "i"(__builtin_constant_p)
            : "memory"
        );
        
        /* Update checksum */
        checksum ^= state;
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final built-in reference in unreachable code */
    if (checksum == 0xDEADBEEF) {
        __builtin_trap();
    }
    
    /* Use function pointer */
    if (builtin_fp && (checksum & 0x100)) {
        builtin_fp();
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(checksum));
    
    /* Return checksum as observable side effect */
    return checksum & 0xFF;
}

#else
/* Non-GCC compilers */
int main(void) {
    return 0;
}
#endif
