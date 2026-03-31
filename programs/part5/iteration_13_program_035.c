/* Built-in function reference test to trigger default_builtin_extdecl hook */
/* Compile with: gcc -O2 -fno-builtin -march=native -fdump-tree-all test.c */

#ifdef __GNUC__

/* Forward declarations - intentionally omitted to force built-in declaration generation */
/* DO NOT declare these functions - let the compiler create artificial declarations */

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference generic built-ins without declaration */
    if (*state & 1) {
        /* Force reference to common built-ins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    }
    
    /* Architecture-specific built-in references */
#ifdef __x86_64__
    /* x86 specific built-in - compiler should create external decl */
    void (*rdtsc_ptr)(void) = (void (*)(void))__builtin_ia32_rdtsc;
    if (*state & 2) {
        /* Call through function pointer */
        ((void (*)(void))rdtsc_ptr)();
    }
#endif

#ifdef __arm__
    /* ARM specific built-in */
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    if (*state & 4) {
        unsigned int val = *state;
        val = ((unsigned int (*)(unsigned int))rbit_ptr)(val);
        *state ^= val;
    }
#endif

#ifdef __aarch64__
    /* AArch64 specific built-in */
    unsigned long long (*clz_ptr)(unsigned long long) = 
        (unsigned long long (*)(unsigned long long))__builtin_aarch64_clzdi;
    if (*state & 8) {
        unsigned long long val = *state;
        val = ((unsigned long long (*)(unsigned long long))clz_ptr)(val);
        *state ^= (int)val;
    }
#endif
}

/* Another helper with different control flow */
static int __attribute__((noinline,noipa)) 
manipulate_with_builtins(volatile int x) {
    int result = x;
    
    /* Complex conditional with built-in references */
    for (int i = 0; i < 3; i++) {
        switch (result & 3) {
            case 0:
                /* Reference trap built-in - may cause declaration */
                if (result < 0) __builtin_trap();
                break;
            case 1:
                /* Reference unreachable built-in */
                if (result > 1000) __builtin_unreachable();
                break;
            case 2:
                /* Use assembly to reference built-in symbol */
                asm volatile("" : : "i"(__builtin_popcount));
                result ^= __builtin_ffs(result);
                break;
            case 3:
                /* Multiple built-in references in one expression */
                result = __builtin_bswap32(result) ^ 
                         __builtin_clz(result | 1);
                break;
        }
        
        /* Mix with CPU features detection if available */
#ifdef __builtin_cpu_supports
        if (i == 1) {
            /* This should trigger external declaration */
            int supports = __builtin_cpu_supports("sse2");
            result += supports;
        }
#endif
    }
    
    return result;
}

#endif /* __GNUC__ */

int main(void) {
    volatile int state = 0x12345678;
    volatile int checksum = 0;
    
#ifdef __GNUC__
    /* Take addresses of built-ins without declaration */
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Complex loop with built-in references */
    for (int i = 0; i < 10; i++) {
        /* Modify state to create varying control flow */
        state ^= i * 0x11111111;
        
        /* Call helper that references built-ins */
        use_builtins(&state);
        
        /* Direct built-in calls based on state */
        if (state & 0x100) {
            /* These calls should force declaration generation */
            __builtin_prefetch(&state, 0, 0);
        }
        
        if ((state & 0xFF) == 0) {
            /* Call through function pointer */
            ((void (*)(void))trap_ptr)();
        }
        
        if (state < 0) {
            ((void (*)(void))unreachable_ptr)();
        }
        
        /* More architecture-specific references */
#ifdef __x86_64__
        /* Inline assembly referencing built-in */
        asm volatile("/* ref: %0 */" : : "i"(__builtin_ia32_rdtsc));
        
        /* Function pointer usage */
        unsigned long long (*rdtsc_ptr)(void) = 
            (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        if (state & 0x800) {
            unsigned long long ts = ((unsigned long long (*)(void))rdtsc_ptr)();
            state ^= (int)(ts & 0xFFFFFFFF);
        }
#endif
        
        /* Call function with complex built-in usage */
        int modified = manipulate_with_builtins(state);
        checksum += modified;
        
        /* Prevent optimization */
        asm volatile("" : "+r"(state));
    }
    
    /* Final built-in references */
    checksum += __builtin_parity(checksum);
    checksum ^= __builtin_ctz(checksum | 1);
    
#endif /* __GNUC__ */
    
    /* Observable output */
    printf("Result: %d\n", checksum);
    
    return checksum != 0;
}
