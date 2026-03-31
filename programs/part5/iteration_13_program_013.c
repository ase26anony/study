/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#ifdef __GNUC__

/* Forward declarations for helper functions */
static void use_builtins_via_pointer(void) __attribute__((noinline));
static void reference_builtins_in_asm(void) __attribute__((noinline));

/* Volatile state to prevent optimization */
static volatile int state = 0;
static volatile long counter = 0;
static volatile void *builtin_ptr = 0;

/* Helper function that takes addresses of built-ins */
static void use_builtins_via_pointer(void) {
    /* Take addresses of various built-ins without declaring them */
    void *ptr1 = (void *)__builtin_trap;
    void *ptr2 = (void *)__builtin_unreachable;
    void *ptr3 = (void *)__builtin_expect;
    
    /* Target-specific built-ins */
#ifdef __x86_64__
    void *ptr4 = (void *)__builtin_ia32_rdtsc;
#endif
#ifdef __arm__
    void *ptr5 = (void *)__builtin_arm_rbit;
#endif
    
    /* Store in volatile to prevent optimization */
    builtin_ptr = ptr1 ? ptr2 : ptr3;
}

/* Helper function that references built-ins in inline assembly */
static void reference_builtins_in_asm(void) {
    /* Reference built-ins in asm operands to create additional uses */
    asm volatile("" : : "i"(__builtin_trap));
    asm volatile("" : : "i"(__builtin_unreachable));
    asm volatile("" : : "i"(__builtin_constant_p));
    
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
}

/* Main test function */
int main(void) {
    volatile int i, result = 0;
    
    /* Initialize volatile state */
    state = 1;
    counter = 0;
    
    /* Loop with complex control flow */
    for (i = 0; i < 10; i++) {
        counter++;
        
        /* Different paths call different undeclared built-ins */
        if (state & 1) {
            /* Call built-in without prototype */
            __builtin_trap();
        } else if (state & 2) {
            __builtin_unreachable();
        } else {
            /* Use __builtin_expect with undeclared prototype */
            if (__builtin_expect(counter > 5, 0)) {
                state = 2;
            }
        }
        
        /* Call built-in constant check without prototype */
        if (__builtin_constant_p(i)) {
            result++;
        }
        
        /* Use target-specific built-ins based on architecture */
#ifdef __x86_64__
        if (i == 3) {
            /* Take address and call through function pointer */
            unsigned long long (*rdtsc_fn)(void) = 
                (unsigned long long (*)(void))__builtin_ia32_rdtsc;
            if (rdtsc_fn) {
                result += (int)rdtsc_fn();
            }
        }
#endif
        
#ifdef __arm__
        if (i == 4) {
            /* ARM-specific built-in */
            unsigned int (*rbit_fn)(unsigned int) = 
                (unsigned int (*)(unsigned int))__builtin_arm_rbit;
            if (rbit_fn) {
                result += rbit_fn(i);
            }
        }
#endif
        
        /* Call helper functions that create additional references */
        if (i % 2 == 0) {
            use_builtins_via_pointer();
        }
        if (i % 3 == 0) {
            reference_builtins_in_asm();
        }
        
        /* Mix with other generic built-ins */
        switch (i % 4) {
            case 0:
                /* __builtin_ffs without prototype */
                result += __builtin_ffs(i);
                break;
            case 1:
                /* __builtin_clz without prototype */
                result += __builtin_clz(i);
                break;
            case 2:
                /* __builtin_ctz without prototype */
                result += __builtin_ctz(i);
                break;
            case 3:
                /* __builtin_popcount without prototype */
                result += __builtin_popcount(i);
                break;
        }
    }
    
    /* Additional references in different scopes */
    {
        /* Nested block with function pointer usage */
        void (* volatile fn_ptr)(void) = (void (*)(void))__builtin_trap;
        if (fn_ptr) {
            /* This creates a use of the symbol */
            asm volatile("" : : "r"(fn_ptr));
        }
    }
    
    /* Compute checksum to ensure code has side effects */
    int checksum = result + (int)counter + state;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(checksum));
    
    return checksum % 256;
}

#else
/* Non-GCC compilers */
int main(void) {
    return 0;
}
#endif
