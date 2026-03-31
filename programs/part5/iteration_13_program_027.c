/* Built-in function declaration stress test to trigger default_builtin_extdecl */
#include <stdio.h>
#include <stdlib.h>

/* Forward declare nothing - we want undeclared built-in usage */

/* Helper function that references multiple built-ins without declarations */
__attribute__((noinline))
static int use_builtins(int x) {
    volatile int result = 0;
    
    /* Reference generic built-ins without prototypes */
    if (x > 100) {
        /* __builtin_expect without declaration */
        if (__builtin_expect(x > 200, 0)) {
            /* __builtin_trap without declaration */
            __builtin_trap();
        }
        
        /* __builtin_constant_p without declaration */
        if (!__builtin_constant_p(x)) {
            result += 1;
        }
    } else {
        /* __builtin_unreachable without declaration */
        if (x < 0) {
            __builtin_unreachable();
        }
    }
    
    /* Architecture-specific built-ins */
#ifdef __x86_64__
    /* __builtin_ia32_rdtsc without declaration */
    unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    result += (int)(rdtsc_ptr() & 0xFF);
    
    /* Use in inline assembly to create additional references */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif

#ifdef __arm__
    /* __builtin_arm_rbit without declaration */
    unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    result += rbit_ptr(0x12345678) & 0xFF;
    
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif

#ifdef __aarch64__
    /* __builtin_aarch64_rbit without declaration */
    unsigned long (*rbit64_ptr)(unsigned long) = __builtin_aarch64_rbit;
    result += rbit64_ptr(0x123456789ABCDEF0) & 0xFF;
#endif

    /* More generic built-ins */
    {
        /* __builtin_popcount without declaration */
        int (*popcount_ptr)(unsigned int) = __builtin_popcount;
        result += popcount_ptr(x) & 1;
    }
    
    return result;
}

/* Another helper with different built-in usage pattern */
__attribute__((noinline))
static void more_builtin_refs(volatile int *state) {
    /* Function pointer to __builtin_cpu_supports */
    int (*cpu_supports_ptr)(const char *) = __builtin_cpu_supports;
    
    if (*state & 1) {
        /* Call through pointer */
        if (cpu_supports_ptr("avx")) {
            *state += 1;
        }
    }
    
    /* __builtin_prefetch without declaration */
    void (*prefetch_ptr)(const void *, ...) = __builtin_prefetch;
    prefetch_ptr(state, 0, 3);
    
    /* __builtin_return_address without declaration */
    void *(*return_addr_ptr)(unsigned int) = __builtin_return_address;
    asm volatile("" : : "r"(return_addr_ptr(0)));
}

int main(void) {
    volatile int state = 0;
    volatile int checksum = 0;
    
    /* Loop with complex control flow around built-in calls */
    for (int i = 0; i < 10; i++) {
        state += i;
        
        /* Different paths call different undeclared built-ins */
        switch (state % 4) {
            case 0:
                /* Direct call to undeclared __builtin_trap in dead code */
                if (state > 1000) {
                    __builtin_trap();
                }
                break;
                
            case 1:
                /* Reference __builtin_unreachable */
                if (state < 0) {  /* Always false */
                    __builtin_unreachable();
                }
                break;
                
            case 2:
                /* Use __builtin_expect */
                if (__builtin_expect(state % 3 == 0, 1)) {
                    checksum += 1;
                }
                break;
                
            case 3:
                /* __builtin_ffs without declaration */
                int (*ffs_ptr)(int) = __builtin_ffs;
                checksum += ffs_ptr(state) & 1;
                break;
        }
        
        /* Call helper that uses more built-ins */
        checksum += use_builtins(state);
        
        /* Call second helper */
        more_builtin_refs(&state);
        
        /* Additional architecture-specific built-in usage */
#ifdef __GNUC__
        {
            /* __builtin_clz without declaration */
            int (*clz_ptr)(unsigned int) = __builtin_clz;
            checksum ^= clz_ptr(state | 1);
            
            /* __builtin_ctz without declaration */
            int (*ctz_ptr)(unsigned int) = __builtin_ctz;
            checksum ^= ctz_ptr(state | 1);
        }
#endif
        
        /* Prevent optimization of function pointer usage */
        volatile void *dummy;
        
#ifdef __x86_64__
        dummy = __builtin_ia32_rdtsc;
        asm volatile("" : : "r"(dummy));
#endif
        
#ifdef __powerpc__
        /* __builtin_ppc_mtfsf without declaration */
        void (*mtfsf_ptr)(unsigned int, unsigned int) = __builtin_ppc_mtfsf;
        dummy = mtfsf_ptr;
        asm volatile("" : : "r"(dummy));
#endif
    }
    
    /* Compute final checksum with more built-in references */
    {
        /* __builtin_parity without declaration */
        int (*parity_ptr)(unsigned int) = __builtin_parity;
        checksum += parity_ptr(state);
        
        /* __builtin_bswap32 without declaration */
        unsigned int (*bswap32_ptr)(unsigned int) = __builtin_bswap32;
        checksum += bswap32_ptr(state) & 0xFF;
    }
    
    printf("Result: %d (state: %d)\n", checksum, state);
    
    /* Final trap if checksum is suspicious */
    if (checksum == 0xBAD) {
        __builtin_trap();
    }
    
    return checksum & 0x7F;
}
