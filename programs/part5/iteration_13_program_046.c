/* Built-in function declaration pressure test for targhooks.cc coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical variables */
volatile int state = 0;
volatile long counter = 0;

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) use_builtins(int selector) {
    /* Reference various built-ins without declarations */
    if (selector & 1) {
        /* Common built-ins */
        (void)__builtin_expect(state, 0);
        (void)__builtin_constant_p(state);
    }
    
    if (selector & 2) {
        /* Control flow built-ins */
        if (state > 100) {
            __builtin_unreachable();
        } else if (state < 0) {
            __builtin_trap();
        }
    }
    
    /* Architecture-specific built-ins */
#ifdef __x86_64__
    if (selector & 4) {
        /* x86 specific built-in without prototype */
        unsigned long long ts = __builtin_ia32_rdtsc();
        state = (int)(ts & 0x7FFFFFFF);
    }
#endif

#ifdef __arm__
    if (selector & 4) {
        /* ARM specific built-in without prototype */
        unsigned int val = __builtin_arm_rbit(state);
        state = (int)val;
    }
#endif

#ifdef __aarch64__
    if (selector & 4) {
        /* AArch64 specific built-in */
        unsigned long long val = __builtin_aarch64_rbit(state);
        state = (int)(val & 0xFFFFFFFF);
    }
#endif
}

/* Function that takes addresses of built-ins */
static void __attribute__((noinline)) take_builtin_addresses(void) {
    /* Create function pointers to undeclared built-ins */
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Store in volatile to prevent optimization */
    volatile void *volatile_trap_ptr = (void *)trap_ptr;
    volatile void *volatile_unreachable_ptr = (void *)unreachable_ptr;
    
    /* Use inline assembly to reference built-ins */
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    void (*rdtsc_ptr)(void) = (void (*)(void))__builtin_ia32_rdtsc;
    volatile void *volatile_rdtsc_ptr = (void *)rdtsc_ptr;
#endif
    
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit));
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    volatile void *volatile_rbit_ptr = (void *)rbit_ptr;
#endif
    
    /* Prevent unused variable warnings */
    (void)volatile_trap_ptr;
    (void)volatile_unreachable_ptr;
#ifdef __x86_64__
    (void)volatile_rdtsc_ptr;
#endif
#ifdef __arm__
    (void)volatile_rbit_ptr;
#endif
}

/* Another helper with different built-in usage pattern */
static int __attribute__((noinline)) builtin_control_flow(int x) {
    int result = 0;
    
    /* Use built-in in loop condition */
    for (int i = 0; i < 10; i++) {
        if (__builtin_expect(i > 5, 0)) {
            result += __builtin_popcount(i);
        }
        
        /* Reference architecture-specific built-in in loop */
#ifdef __x86_64__
        if (i == 3) {
            unsigned long long ts = __builtin_ia32_rdtsc();
            result += (int)(ts % 100);
        }
#endif
        
        /* Use built-in for overflow checking */
        if (__builtin_add_overflow(result, i, &result)) {
            __builtin_trap();
        }
    }
    
    /* Conditional unreachable */
    if (result > 1000) {
        __builtin_unreachable();
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile state */
    state = 42;
    counter = 0;
    
    /* Loop with multiple built-in references */
    for (int i = 0; i < 100; i++) {
        counter++;
        
        /* Vary which built-ins are used based on state */
        int selector = (state + i) % 8;
        
        /* Call helper that references undeclared built-ins */
        use_builtins(selector);
        
        /* Periodically take addresses of built-ins */
        if (i % 23 == 0) {
            take_builtin_addresses();
        }
        
        /* Use built-ins in control flow */
        if (__builtin_expect((i % 7) == 0, 0)) {
            checksum += builtin_control_flow(i);
        }
        
        /* Reference more built-ins directly */
        if (state > 50) {
            (void)__builtin_ffs(state);
            (void)__builtin_clz(state);
        }
        
        /* Change state based on built-in results */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Use built-in for bounds checking */
        if (__builtin_mul_overflow(state, 3, &state)) {
            __builtin_trap();
        }
    }
    
    /* Final built-in references */
    (void)__builtin_abs(checksum);
    (void)__builtin_parity(checksum);
    
    /* Use inline assembly to reference built-ins one more time */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
    
    printf("Result: %d (state: %d, counter: %ld)\n", 
           checksum, state, counter);
    
    return checksum != 0 ? 0 : 1;
}

/* Additional global references to built-ins */
#ifdef __GNUC__
/* Force external references at global scope */
static void (*global_builtin_refs[])(void) = {
    (void (*)(void))__builtin_trap,
    (void (*)(void))__builtin_unreachable,
#ifdef __x86_64__
    (void (*)(void))__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
    (void (*)(void))__builtin_arm_rbit,
#endif
    NULL
};
#endif
