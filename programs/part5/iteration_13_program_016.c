/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#ifdef __GNUC__

/* Forward declare helper functions to create scopes */
static void use_builtins_1(volatile int *state) __attribute__((noinline));
static void use_builtins_2(volatile int *state) __attribute__((noinline));

/* Helper function 1 - creates references to generic built-ins */
static void use_builtins_1(volatile int *state) {
    /* Reference built-ins without declaration */
    if (*state & 1) {
        /* Call undeclared built-in directly */
        __builtin_trap();
    } else {
        /* Another undeclared built-in on different path */
        __builtin_unreachable();
    }
    
    /* Use __builtin_expect without prototype */
    volatile int x = *state;
    if (__builtin_expect(x > 100, 0)) {
        x = __builtin_abs(x);
    }
    
    /* __builtin_constant_p reference */
    volatile int y = __builtin_constant_p(x) ? x : x * 2;
    *state = y;
}

/* Helper function 2 - creates references to target-specific built-ins */
static void use_builtins_2(volatile int *state) {
    /* Function pointer to undeclared built-in */
    void (*builtin_ptr)(void) = 0;
    
    /* Take address of various built-ins without declaration */
#ifdef __x86_64__
    /* x86-specific built-in */
    builtin_ptr = (void (*)(void))__builtin_ia32_rdtsc;
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM-specific built-in */
    builtin_ptr = (void (*)(void))__builtin_arm_rbit;
#else
    /* Generic built-in as fallback */
    builtin_ptr = (void (*)(void))__builtin_return_address;
#endif
    
    /* Call through function pointer */
    if (*state & 2) {
        /* This creates additional pressure for declaration */
        void (*volatile fp)(void) = builtin_ptr;
        (void)fp;
    }
    
    /* Inline assembly referencing built-in names */
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#elif defined(__arm__) || defined(__aarch64__)
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
    
    /* More generic built-in references */
    volatile int z = *state;
    z = __builtin_popcount(z);
    *state = z;
}

/* Main function with complex control flow */
int main(void) {
    volatile int state = 0;
    volatile int checksum = 0;
    
    /* Loop with multiple built-in references */
    for (int i = 0; i < 10; i++) {
        state += i;
        
        /* Conditional calls to undeclared built-ins */
        if (state % 3 == 0) {
            /* Direct call without declaration */
            __builtin_prefetch(&state, 0, 3);
        } else if (state % 3 == 1) {
            /* Another direct call */
            __builtin_expect(state, 0);
        } else {
            /* Function pointer approach */
            long (*rdtsc_fn)(void) = (long (*)(void))__builtin_ia32_rdtsc;
            if (rdtsc_fn) {
                state += 1;
            }
        }
        
        /* Call helper functions */
        if (i % 2 == 0) {
            use_builtins_1(&state);
        } else {
            use_builtins_2(&state);
        }
        
        /* More built-in references in switch */
        switch (state % 4) {
            case 0:
                __builtin_clz(state);
                break;
            case 1:
                __builtin_ctz(state);
                break;
            case 2:
                __builtin_ffs(state);
                break;
            case 3:
                __builtin_parity(state);
                break;
        }
        
        checksum ^= state;
    }
    
    /* Final built-in reference */
    volatile int result = __builtin_bswap32(checksum);
    
    /* Prevent optimization */
    asm volatile("" : "+r"(result));
    
    /* Return checksum to prevent dead code elimination */
    return result & 0xFF;
}

#else
/* Non-GCC compilers fallback */
int main(void) {
    return 0;
}
#endif
