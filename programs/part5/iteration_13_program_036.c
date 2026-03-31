/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#ifdef __GNUC__

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference various built-ins without declaration */
    if (*state & 1) {
        /* Force reference to common built-ins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    } else {
        /* Reference trap/unreachable built-ins */
        if (*state < 0) {
            __builtin_trap();  /* No prototype declared */
        } else if (*state > 100) {
            __builtin_unreachable();  /* No prototype declared */
        }
    }
    
    /* Architecture-specific built-in references */
    #ifdef __x86_64__
    /* x86 specific built-in - rdtsc */
    {
        /* Take address and call through function pointer */
        unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
        volatile unsigned long long ts = rdtsc_ptr();
        *state ^= (int)(ts & 0xFFFFFFFF);
    }
    #endif
    
    #ifdef __arm__
    /* ARM specific built-in - rbit */
    {
        unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
        volatile unsigned int val = rbit_ptr(*state);
        *state ^= val;
    }
    #endif
    
    #ifdef __aarch64__
    /* AArch64 specific built-in */
    {
        unsigned long long (*cnt_ptr)(void) = __builtin_aarch64_cntd;
        volatile unsigned long long cnt = cnt_ptr();
        *state ^= (int)(cnt & 0xFF);
    }
    #endif
}

/* Another helper with different control flow */
static int __attribute__((noinline)) process_with_builtins(volatile int val) {
    int result = 0;
    
    /* Complex loop with built-in references */
    for (int i = 0; i < 3; i++) {
        /* Mix of built-in usage patterns */
        switch (val & 3) {
            case 0:
                result += __builtin_popcount(val);  /* No prototype */
                break;
            case 1:
                result += __builtin_ffs(val);  /* No prototype */
                break;
            case 2:
                /* Use inline assembly to reference built-in */
                asm volatile("" : : "i"(__builtin_clz) : "memory");
                result += 1;
                break;
            case 3:
                /* Function pointer to built-in */
                {
                    int (*clz_ptr)(int) = __builtin_clz;
                    result += clz_ptr(val);
                }
                break;
        }
        
        /* More architecture-specific references */
        #if defined(__i386__) || defined(__x86_64__)
        /* x86 built-in for CPU feature detection */
        if (val & 0x10) {
            volatile int supports_sse = __builtin_cpu_supports("sse");
            result ^= supports_sse;
        }
        #endif
        
        val = (val * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

#endif /* __GNUC__ */

int main(void) {
    volatile int state = 42;
    volatile int checksum = 0;
    
    #ifdef __GNUC__
    /* Initial volatile state to prevent optimization */
    volatile int *state_ptr = &state;
    
    /* Loop with multiple built-in references */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Call helper that references undeclared built-ins */
        use_builtins(state_ptr);
        
        /* Process with different built-ins */
        checksum += process_with_builtins(state);
        
        /* Direct built-in calls without prototypes */
        if (state & 0x100) {
            __builtin_prefetch(state_ptr, 0, 3);  /* No prototype */
        }
        
        /* More function pointer usage */
        {
            void (*trap_ptr)(void) = __builtin_trap;
            if (state < -1000) {
                trap_ptr();  /* Call through pointer */
            }
        }
        
        /* Use assembly to create additional references */
        #ifdef __x86_64__
        asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
        #endif
        
        #ifdef __arm__
        asm volatile("" : : "i"(__builtin_arm_rbit) : "memory");
        #endif
        
        /* Modify state to change control flow */
        state = (state * 1664525 + 1013904223) & 0x7FFFFFFF;
        if (state & 0x8000000) {
            state = -state;
        }
    }
    
    /* Final built-in reference */
    checksum ^= __builtin_bswap32(state);  /* No prototype */
    
    #endif /* __GNUC__ */
    
    /* Observable side effect */
    printf("Result: %d (state: %d)\n", checksum, state);
    
    return checksum != 0 ? 0 : 1;
}
