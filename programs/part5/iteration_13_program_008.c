/* 
 * Program to trigger default_builtin_extdecl in targhooks.cc
 * Forces generation of artificial built-in function declarations
 */

#ifdef __GNUC__

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference built-ins without prototypes */
    if (*state & 1) {
        /* Use generic built-ins */
        int result = __builtin_expect(*state, 0);
        (void)result;
        
        /* Try to take address of built-in */
        void (*fn_ptr)(void) = (void (*)(void))__builtin_trap;
        if (*state % 3 == 0) {
            fn_ptr();  /* Call through pointer */
        }
    }
    
    /* Architecture-specific built-ins */
#if defined(__x86_64__) || defined(__i386__)
    /* x86 specific built-in without prototype */
    unsigned long long (*rdtsc_fn)(void) = 
        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    if (*state % 5 == 0) {
        unsigned long long ts = rdtsc_fn();
        *state ^= (int)(ts & 0xFFFFFFFF);
    }
    
    /* Inline assembly referencing built-in */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif

#if defined(__arm__) || defined(__aarch64__)
    /* ARM specific built-in */
    unsigned int (*rbit_fn)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    if (*state % 7 == 0) {
        unsigned int val = rbit_fn(*state);
        *state ^= val;
    }
#endif
}

/* Another helper with different control flow */
static int __attribute__((noinline)) check_builtins(volatile int x) {
    int result = 0;
    
    /* Complex conditional with built-in calls */
    for (int i = 0; i < 3; i++) {
        switch (x % 4) {
            case 0:
                /* Call built-in without declaration */
                if (__builtin_constant_p(x)) {
                    result |= 1;
                }
                break;
            case 1:
                /* Reference __builtin_unreachable */
                if (x < 0) {
                    __builtin_unreachable();
                }
                result |= 2;
                break;
            case 2:
                /* Use __builtin_trap in dead code */
                if (x > 1000) {  /* Likely false */
                    __builtin_trap();
                }
                result |= 4;
                break;
            case 3:
                /* Mixed built-in usage */
                result = __builtin_expect(result, 0) ? 
                         __builtin_ffs(x) : __builtin_clz(x);
                break;
        }
        x = (x * 13 + 7) & 0xFF;
    }
    
    return result;
}

#endif  /* __GNUC__ */

int main(void) {
    volatile int state = 42;
    volatile int checksum = 0;
    
#ifdef __GNUC__
    /* Function pointer declarations for built-ins (no prototypes!) */
    void (*trap_fn)(void);
    void (*unreachable_fn)(void);
    
    /* Initialize function pointers with built-in addresses */
    trap_fn = (void (*)(void))__builtin_trap;
    unreachable_fn = (void (*)(void))__builtin_unreachable;
    
    /* Main loop with built-in references */
    for (volatile int i = 0; i < 10; i++) {
        /* Call helper functions */
        use_builtins(&state);
        checksum ^= check_builtins(state);
        
        /* Direct built-in calls based on state */
        if (state % 11 == 0) {
            /* This should trigger external decl generation */
            __builtin_trap();
        }
        
        if (state % 13 == 0) {
            __builtin_unreachable();
        }
        
        /* Call through function pointers */
        if (state % 17 == 0 && trap_fn) {
            trap_fn();
        }
        
        if (state % 19 == 0 && unreachable_fn) {
            unreachable_fn();
        }
        
        /* More built-in references */
        int is_const = __builtin_constant_p(state);
        int expected = __builtin_expect(state, 0);
        
        /* Use results to affect state */
        state = (state * 31 + (is_const ? 1 : 0) + expected) & 0xFFFF;
        
        /* Additional architecture-specific references */
#if defined(__x86_64__) || defined(__i386__)
        /* Take address of x86 built-in */
        unsigned long long (*cpu_supports_fn)(const char*) = 
            (unsigned long long (*)(const char*))__builtin_cpu_supports;
        if (cpu_supports_fn && (state % 23 == 0)) {
            unsigned long long supports = cpu_supports_fn("avx");
            state ^= (int)(supports & 0xFF);
        }
#endif
        
#if defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
        /* PowerPC built-in */
        unsigned int (*ppc_fn)(void) = 
            (unsigned int (*)(void))__builtin_ppc_mftb;
        if (ppc_fn && (state % 29 == 0)) {
            unsigned int tb = ppc_fn();
            state ^= tb;
        }
#endif
    }
    
    /* Final built-in reference in different context */
    {
        volatile int final_check = __builtin_ffs(checksum);
        checksum += final_check;
    }
#endif  /* __GNUC__ */
    
    /* Print result to prevent elimination */
    printf("Result: %d (checksum: %d)\n", state, checksum);
    
    return state == 0 ? 0 : 1;
}
