/* 
 * This program is designed to trigger GCC's default_builtin_extdecl target hook
 * to generate artificial external declarations for built-in functions with
 * specific tree flags set (TREE_STATIC, TREE_PUBLIC, DECL_EXTERNAL, etc.)
 * as seen in targhooks.cc lines 981-990.
 */

#ifdef __GNUC__

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference generic built-ins without prototypes */
    if (*state & 1) {
        /* This creates a use of __builtin_expect without declaration */
        if (__builtin_expect(*state > 100, 0)) {
            /* Reference __builtin_trap without declaration */
            __builtin_trap();
        }
    }
    
    /* Reference __builtin_constant_p without declaration */
    if (!__builtin_constant_p(*state)) {
        *state += 2;
    }
}

/* Another helper to take addresses of built-ins */
static void __attribute__((noinline)) take_builtin_address(volatile int *state) {
    /* Function pointer declarations without prior built-in declarations */
    void (*trap_ptr)(void);
    void (*unreachable_ptr)(void);
    
#if defined(__x86_64__) || defined(__i386__)
    unsigned long long (*rdtsc_ptr)(void);
#endif
    
#if defined(__arm__) || defined(__aarch64__)
    unsigned int (*rbit_ptr)(unsigned int);
#endif
    
    /* Take addresses of undeclared built-ins */
    trap_ptr = __builtin_trap;
    unreachable_ptr = __builtin_unreachable;
    
    /* Call through function pointers */
    if (*state & 4) {
        trap_ptr();
    }
    
    /* Use inline assembly to reference built-in names */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
    
#if defined(__x86_64__) || defined(__i386__)
    /* x86-specific built-in without prototype */
    rdtsc_ptr = __builtin_ia32_rdtsc;
    if (*state & 8) {
        unsigned long long tsc = rdtsc_ptr();
        *state ^= (tsc & 0xFF);
    }
    
    /* Inline assembly referencing x86 built-in */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif

#if defined(__arm__) || defined(__aarch64__)
    /* ARM-specific built-in without prototype */
    rbit_ptr = __builtin_arm_rbit;
    if (*state & 16) {
        unsigned int val = *state;
        unsigned int reversed = rbit_ptr(val);
        *state ^= (reversed & 0xFF);
    }
    
    /* Inline assembly referencing ARM built-in */
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
}

int main(void) {
    volatile int state = 0;
    volatile int i;
    
    /* Initialize with some non-deterministic seed */
    asm volatile("" : "=r"(state) : "0"(42));
    
    /* Loop with complex control flow around built-in calls */
    for (i = 0; i < 100; i++) {
        state ^= i;
        
        /* Call helper functions that reference built-ins */
        use_builtins(&state);
        
        /* Direct calls to undeclared built-ins in conditional paths */
        if (state % 3 == 0) {
            /* __builtin_unreachable without prototype */
            if (state > 1000) __builtin_unreachable();
        } else if (state % 3 == 1) {
            /* __builtin_trap without prototype */
            if (state < 0) __builtin_trap();
        } else {
            /* Reference __builtin_expect without prototype */
            if (__builtin_expect(state == 42, 0)) {
                state *= 2;
            }
        }
        
        /* Take addresses and call through pointers */
        if (i % 10 == 0) {
            take_builtin_address(&state);
        }
        
        /* Mix with other generic built-ins */
        int is_const = __builtin_constant_p(i);
        state += is_const ? 1 : 0;
        
        /* Prevent optimization */
        asm volatile("" : "+r"(state));
    }
    
    /* Compute checksum to ensure side effects */
    int checksum = 0;
    for (i = 0; i < 32; i++) {
        checksum ^= (state >> i) & 1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (checksum: %d)\n", state, checksum);
    
    return checksum;
}

#else
#error "This program requires GCC or compatible compiler"
#endif
