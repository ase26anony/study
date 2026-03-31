/* Built-in function declaration stress test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Helper function to create additional declaration contexts */
static void __attribute__((noinline)) 
builtin_stress_helper(volatile int *state) {
    /* Reference various built-ins without prototypes */
    if (*state & 1) {
        /* Force creation of __builtin_trap declaration */
        void (*trap_fn)(void) = __builtin_trap;
        if (*state & 2) {
            trap_fn();
        }
    }
    
    if (*state & 4) {
        /* Force creation of __builtin_unreachable declaration */
        void (*unreachable_fn)(void) = __builtin_unreachable;
        /* Use in asm to create additional reference */
        asm volatile("" : : "i"(__builtin_unreachable));
    }
}

/* Another helper for architecture-specific built-ins */
static void __attribute__((noinline))
arch_builtin_helper(volatile int *state) {
#ifdef __x86_64__
    if (*state & 8) {
        /* x86 specific built-in without prototype */
        unsigned long long (*rdtsc_fn)(void) = __builtin_ia32_rdtsc;
        volatile unsigned long long result = rdtsc_fn();
        use((void*)&result);
        
        /* Additional x86 built-ins */
        void (*pause_fn)(void) = __builtin_ia32_pause;
        asm volatile("" : : "i"(__builtin_ia32_pause));
    }
#endif

#ifdef __arm__
    if (*state & 16) {
        /* ARM specific built-in without prototype */
        unsigned int (*rbit_fn)(unsigned int) = __builtin_arm_rbit;
        volatile unsigned int val = 0x12345678;
        volatile unsigned int reversed = rbit_fn(val);
        use((void*)&reversed);
    }
#endif

#ifdef __aarch64__
    if (*state & 32) {
        /* AArch64 specific built-in */
        unsigned long (*clz_fn)(unsigned long) = __builtin_aarch64_clz;
        volatile unsigned long val = 0xFFFFFFFF;
        volatile unsigned long count = clz_fn(val);
        use((void*)&count);
    }
#endif
}

int main(void) {
    volatile int state = 0;
    volatile int iteration = 0;
    int checksum = 0;
    
    /* Initialize with random-ish value */
    state = (int)((long)&state & 0xFF);
    
    /* Loop with complex control flow */
    for (iteration = 0; iteration < 10; iteration++) {
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Different paths trigger different built-in references */
        if (state % 3 == 0) {
            /* Path 1: Use __builtin_expect without prototype */
            long (*expect_fn)(long, long) = __builtin_expect;
            volatile long pred = expect_fn(state, 0);
            
            /* Also reference __builtin_constant_p */
            int (*const_p_fn)(int) = __builtin_constant_p;
            volatile int is_const = const_p_fn(state);
            
            use((void*)&pred);
            use((void*)&is_const);
            
            /* Inline asm reference */
            asm volatile("" : : "i"(__builtin_expect), "i"(__builtin_constant_p));
        }
        else if (state % 3 == 1) {
            /* Path 2: Use __builtin_trap and __builtin_unreachable */
            builtin_stress_helper((volatile int*)&state);
            
            /* Additional direct references */
            if (state % 7 == 0) {
                void (*trap_ptr)(void) = __builtin_trap;
                asm volatile("" : : "i"(__builtin_trap));
            }
        }
        else {
            /* Path 3: Architecture-specific built-ins */
            arch_builtin_helper((volatile int*)&state);
            
            /* Generic built-ins on all paths */
            void (*prefetch_fn)(const void*, ...) = __builtin_prefetch;
            volatile int dummy = state;
            prefetch_fn(&dummy, 0, 3);
            
            /* __builtin_ffs without prototype */
            int (*ffs_fn)(int) = __builtin_ffs;
            volatile int ffs_result = ffs_fn(state);
            use((void*)&ffs_result);
        }
        
        /* Function pointer table to built-ins */
        void* builtin_fns[] = {
            (void*)__builtin_trap,
            (void*)__builtin_unreachable,
#ifdef __x86_64__
            (void*)__builtin_ia32_rdtsc,
#endif
            (void*)__builtin_expect,
            (void*)__builtin_constant_p,
            (void*)__builtin_ffs,
            (void*)__builtin_prefetch
        };
        
        /* Use function pointers to create additional references */
        for (int i = 0; i < (int)(sizeof(builtin_fns)/sizeof(builtin_fns[0])); i++) {
            if (state & (1 << (i % 16))) {
                checksum += (int)((long)builtin_fns[i] & 0xFF);
            }
        }
        
        /* Reference __builtin_popcount without prototype */
        if (state % 11 == 0) {
            int (*popcount_fn)(unsigned int) = __builtin_popcount;
            volatile int popcnt = popcount_fn(state);
            checksum += popcnt;
            
            /* Inline asm with constraint */
            asm volatile("" : "=r"(popcnt) : "0"(__builtin_popcount));
        }
    }
    
    /* Final checksum computation with more built-in references */
    {
        /* __builtin_abs without prototype */
        int (*abs_fn)(int) = __builtin_abs;
        checksum = abs_fn(checksum);
        
        /* __builtin_clz without prototype */
        int (*clz_fn)(unsigned int) = __builtin_clz;
        volatile int leading_zeros = clz_fn(checksum);
        
        /* __builtin_ctz without prototype */
        int (*ctz_fn)(unsigned int) = __builtin_ctz;
        volatile int trailing_zeros = ctz_fn(checksum);
        
        use((void*)&leading_zeros);
        use((void*)&trailing_zeros);
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* One more path that could trigger built-in generation */
    if (checksum == 0) {
        /* This should never happen, but references __builtin_unreachable */
        void (*unreachable_ptr)(void) = __builtin_unreachable;
        asm volatile("" : : "i"(__builtin_unreachable));
    }
    
    return checksum == 0 ? 0 : 1;
}
