/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* No built-in prototypes included - we want undeclared references */

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(volatile int *state) {
    /* Reference various built-ins without declaration */
    if (*state & 1) {
        /* Force reference to common built-ins */
        (void)__builtin_expect(*state, 0);
        (void)__builtin_constant_p(*state);
    }
    
    if (*state & 2) {
        /* Reference trap/unreachable built-ins */
        /* These might not return, so conditionally reference them */
        if (*state == 0xFF) {
            __builtin_trap();
        }
        if (*state == 0xFE) {
            __builtin_unreachable();
        }
    }
}

/* Another helper to take addresses of built-ins */
static void __attribute__((noinline)) take_builtin_addresses(volatile int *state) {
    /* Function pointers to undeclared built-ins */
    void (*trap_ptr)(void);
    void (*unreachable_ptr)(void);
    
    /* Take addresses - compiler must create declarations */
    trap_ptr = __builtin_trap;
    unreachable_ptr = __builtin_unreachable;
    
    /* Store in volatile to prevent optimization */
    volatile void *volatile_ptr = trap_ptr;
    (void)volatile_ptr;
    
    /* Conditional call through pointer */
    if (*state == 0xFD) {
        (*trap_ptr)();
    }
}

/* Target-specific built-in usage */
#ifdef __x86_64__
static unsigned long long __attribute__((noinline)) use_x86_builtins(void) {
    /* Reference x86-specific built-in without declaration */
    unsigned long long result = __builtin_ia32_rdtsc();
    
    /* Also take its address */
    unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    
    /* Use inline assembly to reference the built-in symbol */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    
    /* Call through pointer */
    if (result & 1) {
        return (*rdtsc_ptr)();
    }
    
    return result;
}
#endif

#ifdef __arm__
static unsigned int __attribute__((noinline)) use_arm_builtins(unsigned int x) {
    /* Reference ARM-specific built-in without declaration */
    unsigned int result = __builtin_arm_rbit(x);
    
    /* Take address */
    unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    
    /* Use in inline assembly */
    asm volatile("" : : "i"(__builtin_arm_rbit));
    
    /* Call through pointer */
    if (x & 1) {
        return (*rbit_ptr)(x);
    }
    
    return result;
}
#endif

/* Main function with complex control flow */
int main(void) {
    volatile int state = 0;
    volatile int checksum = 0;
    
    /* Loop with multiple built-in references */
    for (int i = 0; i < 100; i++) {
        state = i;
        
        /* Call helper functions that reference built-ins */
        use_builtins(&state);
        take_builtin_addresses(&state);
        
        /* Target-specific built-in usage */
#ifdef __x86_64__
        if (i % 3 == 0) {
            unsigned long long ts = use_x86_builtins();
            checksum += (ts & 0xFF);
        }
#endif

#ifdef __arm__
        if (i % 5 == 0) {
            unsigned int rev = use_arm_builtins(i);
            checksum += (rev & 0xFF);
        }
#endif
        
        /* Reference more generic built-ins directly */
        if (i % 7 == 0) {
            /* __builtin_popcount without declaration */
            int popcnt = __builtin_popcount(i);
            checksum += popcnt;
        }
        
        if (i % 11 == 0) {
            /* __builtin_clz without declaration */
            int lz = __builtin_clz(i);
            checksum += lz;
        }
        
        /* Create function pointer to __builtin_ffs */
        int (*ffs_ptr)(int) = __builtin_ffs;
        if (i % 13 == 0) {
            checksum += (*ffs_ptr)(i);
        }
        
        /* Use inline assembly with built-in reference */
        asm volatile("" : : "i"(__builtin_bswap32), "r"(i));
    }
    
    /* Final built-in reference in different context */
    {
        /* Nested block with its own scope */
        volatile int final_state = checksum;
        
        /* Reference __builtin_abs without declaration */
        int abs_val = __builtin_abs(final_state);
        
        /* Take address of multiple built-ins */
        void *builtin_ptrs[] = {
            (void *)__builtin_trap,
            (void *)__builtin_unreachable,
            (void *)__builtin_expect,
            (void *)__builtin_constant_p,
#ifdef __x86_64__
            (void *)__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
            (void *)__builtin_arm_rbit,
#endif
        };
        
        /* Use the pointers to prevent optimization */
        for (size_t j = 0; j < sizeof(builtin_ptrs)/sizeof(builtin_ptrs[0]); j++) {
            checksum += ((unsigned long)builtin_ptrs[j] & 0xFF);
        }
        
        printf("Checksum: %d (abs: %d)\n", checksum, abs_val);
    }
    
    return checksum != 0 ? 0 : 1;
}

/* Additional global references to built-ins */
static void *global_builtin_refs[] = {
    (void *)__builtin_trap,
    (void *)__builtin_unreachable,
    (void *)__builtin_expect,
    (void *)__builtin_constant_p,
#ifdef __x86_64__
    (void *)__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
    (void *)__builtin_arm_rbit,
#endif
    (void *)__builtin_popcount,
    (void *)__builtin_clz,
    (void *)__builtin_ffs,
    (void *)__builtin_bswap32,
    (void *)__builtin_abs,
};

/* Constructor that references built-ins */
static void __attribute__((constructor)) init_builtin_refs(void) {
    volatile int dummy = 0;
    
    /* Force references in constructor context */
    (void)__builtin_expect(dummy, 0);
    
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
    
    /* Use global array */
    for (size_t i = 0; i < sizeof(global_builtin_refs)/sizeof(global_builtin_refs[0]); i++) {
        dummy += ((unsigned long)global_builtin_refs[i] & 1);
    }
}
