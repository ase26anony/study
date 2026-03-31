/* 
 * Program to trigger default_builtin_extdecl in targhooks.cc
 * Forces generation of artificial external declarations for built-in functions
 */

#ifdef __GNUC__

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference builtins without declaration */
    if (*state & 1) {
        /* Force reference to common builtins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    } else {
        /* Reference trap/unreachable builtins */
        if (*state == 0) {
            __builtin_trap();  /* Undeclared reference */
        } else {
            __builtin_unreachable();  /* Undeclared reference */
        }
    }
}

/* Function pointer manipulation to force declaration creation */
static void __attribute__((noinline)) take_builtin_address(volatile int *state) {
    /* Take address of various builtins without declaration */
    void (*volatile fn_ptr)(void);
    
#ifdef __x86_64__
    /* x86-specific builtin */
    fn_ptr = (void (*)(void))__builtin_ia32_rdtsc;
    if (*state & 2) {
        /* Call through function pointer */
        ((unsigned long long (*)(void))fn_ptr)();
    }
#endif

#ifdef __arm__
    /* ARM-specific builtin */
    fn_ptr = (void (*)(void))__builtin_arm_rbit;
    if (*state & 4) {
        /* Call through function pointer */
        ((unsigned int (*)(unsigned int))fn_ptr)(*state);
    }
#endif

#ifdef __aarch64__
    /* ARM64-specific builtin */
    fn_ptr = (void (*)(void))__builtin_aarch64_rbit;
    if (*state & 8) {
        /* Call through function pointer */
        ((unsigned long (*)(unsigned long))fn_ptr)(*state);
    }
#endif

    /* Store function pointer to prevent optimization */
    *state = (int)(long)fn_ptr;
}

/* Inline assembly to create additional references */
static void __attribute__((noinline)) asm_builtin_references(volatile int *state) {
    /* Use builtin names in inline assembly constraints */
#ifdef __x86_64__
    asm volatile("" 
                 : /* no outputs */
                 : "i"(__builtin_ia32_rdtsc), "r"(*state)
                 : "memory");
#endif

#ifdef __arm__
    asm volatile(""
                 : /* no outputs */
                 : "i"(__builtin_arm_rbit), "r"(*state)
                 : "memory");
#endif

    /* Generic builtin references in asm */
    asm volatile(""
                 : /* no outputs */
                 : "i"(__builtin_trap), "i"(__builtin_unreachable)
                 : "memory");
}

#endif /* __GNUC__ */

int main(void) {
    volatile int state = 0;
    volatile int checksum = 0;
    
#ifdef __GNUC__
    /* Complex control flow with builtin references */
    for (int i = 0; i < 10; i++) {
        state = i;
        
        /* Path 1: Call helper functions */
        if (i % 3 == 0) {
            use_builtins(&state);
        }
        /* Path 2: Take addresses */
        else if (i % 3 == 1) {
            take_builtin_address(&state);
        }
        /* Path 3: Use inline assembly */
        else {
            asm_builtin_references(&state);
        }
        
        /* Direct undeclared builtin calls in main */
        if (state & 1) {
            /* Reference CPU support builtin without declaration */
            if (__builtin_cpu_supports("sse2")) {
                state++;
            }
        } else {
            /* Reference other common builtins */
            (void)__builtin_abs(state);
            (void)__builtin_clz(state);
        }
        
        /* Mix with function pointers in main */
        void (*volatile ptr)(void) = (void (*)(void))__builtin_expect;
        if (ptr != 0) {
            checksum += state;
        }
        
        /* Additional architecture-specific references */
#ifdef __x86_64__
        /* Multiple references to same builtin */
        (void)__builtin_ia32_rdtsc;
        asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
        
#ifdef __arm__
        (void)__builtin_arm_rbit(state);
        asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
        
        checksum += i;
    }
    
    /* Final builtin reference outside loop */
    if (checksum > 100) {
        __builtin_trap();
    } else if (checksum < 0) {
        __builtin_unreachable();
    }
    
    /* Compute observable result */
    checksum += __builtin_popcount(state);
    
#endif /* __GNUC__ */
    
    /* Ensure observable side effect */
    printf("Result: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}
