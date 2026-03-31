/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#ifdef __GNUC__

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference built-ins without declaration */
    if (*state & 1) {
        /* Force reference to common built-ins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    } else {
        /* Reference trap/unreachable built-ins */
        if (*state == 0) {
            __builtin_trap();
        } else {
            __builtin_unreachable();
        }
    }
}

/* Function pointer manipulation to pressure declaration generation */
static void __attribute__((noinline)) take_builtin_address(volatile int *state) {
    /* Take address of various built-ins without declaration */
    void (*volatile fn_ptr)(void);
    
#ifdef __x86_64__
    /* x86-specific built-in */
    fn_ptr = (void (*)(void))__builtin_ia32_rdtsc;
    if (*state & 2) {
        /* Call through function pointer */
        ((unsigned long long (*)(void))fn_ptr)();
    }
#endif

#ifdef __arm__
    /* ARM-specific built-in */
    fn_ptr = (void (*)(void))__builtin_arm_rbit;
    if (*state & 4) {
        /* Call through function pointer */
        ((unsigned int (*)(unsigned int))fn_ptr)(*state);
    }
#endif

#ifdef __aarch64__
    /* AArch64-specific built-in */
    fn_ptr = (void (*)(void))__builtin_aarch64_rbit;
    if (*state & 8) {
        ((unsigned long (*)(unsigned long))fn_ptr)(*state);
    }
#endif

    /* Generic built-in function pointer */
    fn_ptr = (void (*)(void))__builtin_unreachable;
    *state += (long)fn_ptr & 1;
}

/* Inline assembly to create additional references */
static void __attribute__((noinline)) asm_builtin_references(volatile int *state) {
    /* Use built-in names in inline assembly constraints */
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
#endif
    
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit) : "memory");
#endif
    
    /* Reference common built-ins in asm */
    asm volatile("" : : "i"(__builtin_expect) : "memory");
    asm volatile("" : : "i"(__builtin_constant_p) : "memory");
    
    *state += 1;
}

#endif /* __GNUC__ */

int main(void) {
    volatile int state = 0;
    volatile int checksum = 0;
    
#ifdef __GNUC__
    int i;
    
    /* Complex control flow with built-in references */
    for (i = 0; i < 10; i++) {
        state ^= i;
        
        /* Different paths referencing different built-ins */
        if (state & 1) {
            /* Reference CPU support built-in without declaration */
            if (__builtin_cpu_supports("sse2")) {
                checksum += 1;
            }
            
            /* Reference popcount built-in */
            checksum += __builtin_popcount(state);
        } else {
            /* Reference other built-ins */
            checksum += __builtin_ffs(state | 1);
            
            /* Take address of trap built-in */
            void (*volatile trap_fn)(void) = __builtin_trap;
            if (state == 0x55) {
                trap_fn();
            }
        }
        
        /* Call helper functions that reference built-ins */
        use_builtins(&state);
        take_builtin_address(&state);
        asm_builtin_references(&state);
        
        /* Reference optimization built-ins */
        if (__builtin_expect((state & 0xF) == 0, 0)) {
            checksum += __builtin_clz(state | 1);
        }
        
        /* Reference synchronization built-in */
        __sync_synchronize();
        
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final built-in reference outside loop */
    checksum += __builtin_constant_p(checksum) ? 1 : 0;
    
    /* Use return address built-in */
    void *ra = __builtin_return_address(0);
    checksum += (long)ra & 1;
    
#endif /* __GNUC__ */
    
    /* Observable side effect */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 1;
}
