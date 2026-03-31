/* Built-in function reference test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile void *volatile_func_ptr = NULL;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static int use_builtins_internal(int selector) {
    volatile int local_result = 0;
    
    /* Reference various built-ins without declarations */
    if (selector & 1) {
        /* Force reference to __builtin_expect without prototype */
        local_result += (int)__builtin_expect(selector > 0, 1);
    }
    
    if (selector & 2) {
        /* Reference __builtin_constant_p */
        local_result += __builtin_constant_p(selector) ? 1 : 0;
    }
    
    if (selector & 4) {
        /* Reference __builtin_unreachable - creates interesting control flow */
        if (selector == 999) {  /* Never true, but compiler doesn't know */
            __builtin_unreachable();
        }
    }
    
    /* Take address of built-in functions */
    void *ptr1 = (void*)__builtin_expect;
    void *ptr2 = (void*)__builtin_constant_p;
    
    /* Use inline assembly to reference built-in names */
    asm volatile("" : : "i"(__builtin_expect), "i"(__builtin_constant_p));
    
    return local_result + ((long)ptr1 & 1) + ((long)ptr2 & 1);
}

/* Another helper for architecture-specific built-ins */
__attribute__((noinline))
static void use_arch_builtins(void) {
#ifdef __x86_64__
    /* x86 specific built-ins */
    volatile long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    
    /* Call through function pointer */
    if (global_counter & 1) {
        volatile long long result = rdtsc_ptr();
        global_counter += (int)(result & 1);
    }
#endif

#ifdef __arm__
    /* ARM specific built-ins */
    volatile unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    asm volatile("" : : "i"(__builtin_arm_rbit));
    
    if (global_counter & 2) {
        volatile unsigned int result = rbit_ptr(0x12345678);
        global_counter += (result & 1);
    }
#endif

#ifdef __aarch64__
    /* AArch64 specific built-ins */
    volatile unsigned long (*aarch64_rbit_ptr)(unsigned long) = __builtin_aarch64_rbit;
    asm volatile("" : : "i"(__builtin_aarch64_rbit));
#endif

#ifdef __powerpc__
    /* PowerPC specific built-ins */
    volatile unsigned long (*ppc_mftb_ptr)(void) = __builtin_ppc_mftb;
    asm volatile("" : : "i"(__builtin_ppc_mftb));
#endif
}

int main(void) {
    volatile int i, result = 0;
    volatile void * volatile_ptrs[4] = {0};
    
    /* Create function pointers to undeclared built-ins */
    volatile void (*trap_ptr)(void) = __builtin_trap;
    volatile void (*unreachable_ptr)(void) = __builtin_unreachable;
    volatile long (*expect_ptr)(long, long) = __builtin_expect;
    
    /* Store pointers in volatile array to prevent optimization */
    volatile_ptrs[0] = (void*)trap_ptr;
    volatile_ptrs[1] = (void*)unreachable_ptr;
    volatile_ptrs[2] = (void*)expect_ptr;
    
    /* Complex loop with multiple built-in references */
    for (i = 0; i < 100; i++) {
        global_counter++;
        
        /* Call helper that references built-ins */
        result += use_builtins_internal(i);
        
        /* Call architecture-specific built-in helper */
        use_arch_builtins();
        
        /* Conditional built-in calls based on volatile state */
        if (global_counter % 7 == 0) {
            /* Reference __builtin_trap without declaration */
            if (i == 99) {  /* Will never be true when i < 100 */
                __builtin_trap();
            }
        }
        
        if (global_counter % 13 == 0) {
            /* Call through function pointer */
            if (volatile_ptrs[0] && (i & 1)) {
                ((void (*)(void))volatile_ptrs[0])();
            }
        }
        
        /* Reference more generic built-ins */
        if (global_counter % 3 == 0) {
            /* __builtin_popcount without declaration */
            volatile int popcnt = __builtin_popcount(i);
            result += popcnt & 1;
        }
        
        if (global_counter % 5 == 0) {
            /* __builtin_ffs without declaration */
            volatile int ffs_result = __builtin_ffs(i);
            result += ffs_result & 1;
        }
        
        /* Use inline assembly with built-in references */
        asm volatile(
            "/* Force reference to builtins */"
            : 
            : "i"(__builtin_trap), 
              "i"(__builtin_unreachable),
              "i"(__builtin_popcount),
              "i"(__builtin_ffs)
        );
    }
    
    /* Final reference to ensure all built-ins are used */
    volatile int final_check = 
        __builtin_constant_p(result) +
        __builtin_expect(result > 0, 1);
    
    /* Mix in some floating point built-ins if available */
#ifdef __GNUC__
    volatile double (*sqrt_ptr)(double) = __builtin_sqrt;
    volatile float (*sqrtf_ptr)(float) = __builtin_sqrtf;
    
    asm volatile("" : : "i"(__builtin_sqrt), "i"(__builtin_sqrtf));
    
    if (result & 1) {
        volatile double d = sqrt_ptr(2.0);
        volatile float f = sqrtf_ptr(2.0f);
        final_check += (int)d + (int)f;
    }
#endif
    
    printf("Result: %d (counter: %d)\n", result + final_check, global_counter);
    
    /* One more path that could use __builtin_unreachable */
    if (result < 0) {
        __builtin_unreachable();
    }
    
    return (result + final_check) > 0 ? 0 : 1;
}
