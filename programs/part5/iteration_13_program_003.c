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
    /* Take addresses of various built-ins without declarations */
    void *ptr1 = (void *)__builtin_trap;
    void *ptr2 = (void *)__builtin_unreachable;
    void *ptr3 = (void *)__builtin_expect;
    
    /* Target-specific built-ins */
#ifdef __x86_64__
    void *ptr4 = (void *)__builtin_ia32_rdtsc;
    void *ptr5 = (void *)__builtin_ia32_pause;
#endif
#ifdef __arm__
    void *ptr6 = (void *)__builtin_arm_rbit;
#endif
#ifdef __aarch64__
    void *ptr7 = (void *)__builtin_aarch64_ld64b;
#endif
    
    /* Store in volatile to prevent optimization */
    builtin_ptr = ptr1 ? ptr2 : ptr3;
    
    /* Call through function pointer - creates additional reference */
    if (counter & 1) {
        void (*trap_fn)(void) = (void (*)(void))ptr1;
        if (state > 100) trap_fn();
    }
}

/* Helper function that references built-ins in inline assembly */
static void reference_builtins_in_asm(void) {
    /* Reference built-ins as immediate operands in asm */
    asm volatile("# Builtin reference %0" : : "i"(__builtin_trap));
    asm volatile("# Builtin reference %0" : : "i"(__builtin_unreachable));
    asm volatile("# Builtin reference %0" : : "i"(__builtin_expect));
    asm volatile("# Builtin reference %0" : : "i"(__builtin_constant_p));
    
#ifdef __x86_64__
    asm volatile("# Builtin reference %0" : : "i"(__builtin_ia32_rdtsc));
    asm volatile("# Builtin reference %0" : : "i"(__builtin_ia32_cpuid));
#endif
#ifdef __arm__
    asm volatile("# Builtin reference %0" : : "i"(__builtin_arm_rbit));
#endif
}

int main(void) {
    int i;
    volatile int checksum = 0;
    
    /* Initialize volatile state */
    state = 42;
    counter = 0;
    
    /* Loop with complex control flow around built-in calls */
    for (i = 0; i < 100; i++) {
        counter++;
        
        /* Different paths call different undeclared built-ins */
        if (state & 0x01) {
            /* Call built-in without prototype */
            __builtin_trap();
        } else if (state & 0x02) {
            /* Another built-in without prototype */
            __builtin_unreachable();
        } else if (state & 0x04) {
            /* Use __builtin_expect without prototype */
            if (__builtin_expect(state > 50, 0)) {
                checksum += 1;
            }
        } else if (state & 0x08) {
            /* Use __builtin_constant_p without prototype */
            if (__builtin_constant_p(state)) {
                checksum += 2;
            }
        }
        
#ifdef __x86_64__
        /* x86-specific built-in without prototype */
        if (state & 0x10) {
            unsigned long long tsc = __builtin_ia32_rdtsc();
            checksum += (tsc & 0xFF);
        }
#endif
        
#ifdef __arm__
        /* ARM-specific built-in without prototype */
        if (state & 0x20) {
            unsigned int val = __builtin_arm_rbit(state);
            checksum += (val & 0xF);
        }
#endif
        
        /* Take address of built-in and call through pointer */
        use_builtins_via_pointer();
        
        /* Reference built-ins in assembly */
        if (i % 7 == 0) {
            reference_builtins_in_asm();
        }
        
        /* Modify state to change control flow */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Function pointer call to built-in */
        if (builtin_ptr && (i % 13 == 0)) {
            /* This creates additional pressure for declaration */
            void (*volatile fn)(void) = (void (*)(void))builtin_ptr;
            if (state % 19 == 0) {
                fn();
            }
        }
    }
    
    /* Additional references in different scopes */
    {
        /* Nested block with built-in reference */
        volatile int local_state = state;
        if (local_state > 1000) {
            __builtin_trap();
        }
    }
    
    /* Compute final checksum */
    checksum += (int)(counter & 0xFF);
    checksum += (state & 0xFF);
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", checksum);
    
    return checksum > 100 ? 0 : 1;
}

#else
/* Non-GCC compilers */
int main(void) {
    printf("This test requires GCC or compatible compiler\n");
    return 0;
}
#endif
