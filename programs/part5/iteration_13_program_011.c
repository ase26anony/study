/* Built-in function declaration stress test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile int should_trap = 0;
static volatile int use_unreachable = 0;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void call_builtin_via_pointer(void) {
    /* Take address of undeclared built-in functions */
    void (*trap_ptr)(void);
    void (*unreachable_ptr)(void);
    
    /* These built-ins have no prototype in this compilation unit */
    trap_ptr = (void (*)(void))__builtin_trap;
    unreachable_ptr = (void (*)(void))__builtin_unreachable;
    
    /* Store pointers in volatile to prevent optimization */
    volatile void *volatile_trap_ptr = (void *)trap_ptr;
    volatile void *volatile_unreachable_ptr = (void *)unreachable_ptr;
    
    /* Reference the pointers to ensure they're used */
    if (global_counter & 1) {
        asm volatile("" : : "r"(volatile_trap_ptr));
    } else {
        asm volatile("" : : "r"(volatile_unreachable_ptr));
    }
}

/* Another helper with architecture-specific built-ins */
__attribute__((noinline))
static void use_arch_builtins(void) {
#ifdef __x86_64__
    /* x86-specific built-in without prototype */
    unsigned long long (*rdtsc_ptr)(void);
    rdtsc_ptr = (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    
    /* Call through pointer if condition is met */
    if (global_counter % 3 == 0) {
        volatile unsigned long long result = rdtsc_ptr();
        asm volatile("" : : "r"(result));
    }
    
    /* Use in inline assembly constraint */
    asm volatile("movq %%rax, %0" : "=r"(global_counter) : "i"(__builtin_ia32_rdtsc));
#endif

#ifdef __arm__
    /* ARM-specific built-in without prototype */
    unsigned int (*rbit_ptr)(unsigned int);
    rbit_ptr = (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    
    if (global_counter % 5 == 0) {
        volatile unsigned int test_val = 0x12345678;
        volatile unsigned int reversed = rbit_ptr(test_val);
        asm volatile("" : : "r"(reversed));
    }
#endif
}

/* Function using generic built-ins without prototypes */
__attribute__((noinline))
static int use_generic_builtins(int x) {
    /* __builtin_expect without prototype */
    long (*expect_ptr)(long, long);
    expect_ptr = (long (*)(long, long))__builtin_expect;
    
    /* __builtin_constant_p without prototype */
    int (*constant_p_ptr)(...);
    constant_p_ptr = (int (*)(...))__builtin_constant_p;
    
    volatile int result = 0;
    
    /* Complex control flow with built-in references */
    for (int i = 0; i < 3; i++) {
        if (expect_ptr(x > 0, 1)) {
            result += constant_p_ptr(x) ? 1 : 0;
        }
        
        /* Mix with other undeclared built-ins */
        if (i == 1) {
            /* __builtin_popcount without prototype */
            int (*popcount_ptr)(unsigned int);
            popcount_ptr = (int (*)(unsigned int))__builtin_popcount;
            result += popcount_ptr(x);
        }
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile state */
    should_trap = (global_counter % 7 == 0);
    use_unreachable = (global_counter % 11 == 0);
    
    /* Complex loop with multiple built-in references */
    for (int i = 0; i < 10; i++) {
        global_counter++;
        
        /* Path with __builtin_trap */
        if (should_trap && (i % 2 == 0)) {
            /* Call undeclared built-in directly */
            __builtin_trap();
        }
        
        /* Path with __builtin_unreachable */
        if (use_unreachable && (i % 3 == 0)) {
            /* Another undeclared built-in call */
            __builtin_unreachable();
        }
        
        /* Use function pointers to built-ins */
        call_builtin_via_pointer();
        
        /* Use architecture-specific built-ins */
        use_arch_builtins();
        
        /* Use generic built-ins */
        checksum += use_generic_builtins(i);
        
        /* Additional direct calls to undeclared built-ins */
        if (i == 5) {
            /* __builtin_clz without prototype */
            int leading_zeros = __builtin_clz(i);
            checksum += leading_zeros;
        }
        
        /* Use built-in in inline assembly constraint */
        asm volatile(
            "addl %1, %0\n\t"
            : "+r"(checksum)
            : "i"(__builtin_constant_p(i) ? 1 : 0)
        );
        
        /* Toggle state variables */
        should_trap ^= 1;
        use_unreachable ^= 1;
    }
    
    /* More built-in references after the loop */
#ifdef __GNUC__
    /* __builtin_ffs without prototype */
    int (*ffs_ptr)(int);
    ffs_ptr = (int (*)(int))__builtin_ffs;
    checksum += ffs_ptr(checksum);
    
    /* __builtin_parity without prototype */
    int (*parity_ptr)(unsigned int);
    parity_ptr = (int (*)(unsigned int))__builtin_parity;
    checksum += parity_ptr(checksum);
#endif
    
    /* Final output to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* One more undeclared built-in for good measure */
    if (checksum > 1000) {
        __builtin_unreachable();
    }
    
    return checksum & 0xFF;
}
