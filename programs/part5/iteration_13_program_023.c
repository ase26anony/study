/* 
 * This program is designed to trigger GCC's default_builtin_extdecl target hook
 * to generate artificial external declarations for built-in functions with
 * specific tree flags set (TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, etc.)
 * as seen in targhooks.cc lines 981-990.
 */

#ifdef __GNUC__

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference architecture-specific built-ins without prototypes */
    #ifdef __x86_64__
    /* x86 specific built-in - will need external declaration */
    void (*x86_fn)(void) = (void (*)(void))__builtin_ia32_rdtsc;
    if (*state & 1) {
        /* Call through function pointer */
        ((void (*)(void))x86_fn)();
    }
    #endif
    
    #ifdef __arm__
    /* ARM specific built-in */
    void (*arm_fn)(void) = (void (*)(void))__builtin_arm_rbit;
    if (*state & 2) {
        ((void (*)(void))arm_fn)();
    }
    #endif
    
    /* Reference generic built-ins without prototypes */
    if (*state & 4) {
        /* __builtin_expect without prototype */
        int (*expect_fn)(long, int) = (int (*)(long, int))__builtin_expect;
        volatile long val = *state;
        volatile int result = ((int (*)(long, int))expect_fn)(val, 0);
        *state += result;
    }
}

/* Another helper to increase declaration pressure */
static void __attribute__((noinline)) more_builtin_references(volatile int *state) {
    /* Take address of built-in constant checker */
    int (*const_p_fn)(void) = (int (*)(void))__builtin_constant_p;
    
    /* Use inline assembly to reference built-in names */
    #ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    #endif
    
    /* Call built-in through pointer with complex control flow */
    for (int i = 0; i < 3; i++) {
        if ((*state >> i) & 1) {
            volatile int res = ((int (*)(void))const_p_fn)();
            *state ^= res;
        }
    }
}

#endif /* __GNUC__ */

int main(void) {
    volatile int state = 0x1234;
    volatile int checksum = 0;
    
    #ifdef __GNUC__
    /* Loop with complex control flow around built-in calls */
    for (volatile int i = 0; i < 10; i++) {
        /* Different paths call different undeclared built-ins */
        if (state & (1 << (i % 8))) {
            /* Call __builtin_trap without prototype */
            void (*trap_fn)(void) = (void (*)(void))__builtin_trap;
            ((void (*)(void))trap_fn)();
        } else if (state & (1 << ((i + 3) % 8))) {
            /* Call __builtin_unreachable without prototype */
            void (*unreachable_fn)(void) = (void (*)(void))__builtin_unreachable;
            ((void (*)(void))unreachable_fn)();
        } else {
            /* Use helper functions that reference more built-ins */
            use_builtins(&state);
        }
        
        /* More built-in references in nested scope */
        {
            volatile int local_state = state ^ i;
            if (local_state > 1000) {
                more_builtin_references(&state);
            }
        }
        
        /* Function pointer to target-specific built-in */
        #ifdef __x86_64__
        volatile void (*rdtsc_ptr)(void) = (void (*)(void))__builtin_ia32_rdtsc;
        if (state & 0x80) {
            ((void (*)(void))rdtsc_ptr)();
        }
        #endif
        
        /* Update volatile state to prevent optimization */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        checksum ^= state;
    }
    
    /* Final built-in reference in conditional */
    if (checksum != 0) {
        /* __builtin_popcount without prototype */
        int (*popcount_fn)(int) = (int (*)(int))__builtin_popcount;
        checksum = ((int (*)(int))popcount_fn)(checksum);
    }
    #endif /* __GNUC__ */
    
    /* Observable side effect to prevent dead code elimination */
    volatile int output = checksum;
    
    /* Simple output to ensure code has effect */
    return output & 0xFF;
}
