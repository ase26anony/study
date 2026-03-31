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
    
    /* Reference built-in functions without declarations */
    if (selector & 1) {
        /* Force reference to common built-ins */
        local_result += (int)__builtin_expect(selector > 0, 1);
    }
    
    if (selector & 2) {
        /* Reference constant detection built-in */
        local_result += __builtin_constant_p(selector) ? 1 : 0;
    }
    
    if (selector & 4) {
        /* Reference trap built-in - will create external declaration */
        void (*trap_fn)(void) = (void (*)(void))__builtin_trap;
        if (selector == 7) {
            trap_fn(); /* This might actually trap at runtime */
        }
    }
    
    return local_result;
}

/* Another helper with architecture-specific built-ins */
__attribute__((noinline))
static void use_arch_builtins(void) {
    volatile unsigned long long timestamp = 0;
    
#if defined(__x86_64__) || defined(__i386__)
    /* x86 specific built-in - rdtsc */
    void (*rdtsc_fn)(void) = (void (*)(void))__builtin_ia32_rdtsc;
    volatile_func_ptr = (void *)rdtsc_fn;
    
    /* Use inline assembly to reference the built-in */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif

#if defined(__arm__) || defined(__aarch64__)
    /* ARM specific built-in - reverse bits */
    unsigned int (*rbit_fn)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    volatile_func_ptr = (void *)rbit_fn;
    
    /* Reference through inline assembly */
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif

#if defined(__GNUC__) && !defined(__clang__)
    /* GCC-specific built-ins */
    int (*popcount_fn)(unsigned int) = 
        (int (*)(unsigned int))__builtin_popcount;
    if (popcount_fn) {
        timestamp = popcount_fn(0x12345678);
    }
#endif
}

/* Function that takes address of various built-ins */
__attribute__((noinline))
static void take_builtin_addresses(void) {
    /* Create function pointers to undeclared built-ins */
    void (*unreachable_fn)(void) = (void (*)(void))__builtin_unreachable;
    void (*trap_fn)(void) = (void (*)(void))__builtin_trap;
    int (*clz_fn)(unsigned int) = (int (*)(unsigned int))__builtin_clz;
    
    /* Store them in volatile memory to prevent optimization */
    volatile void *volatile_ptr1 = (void *)unreachable_fn;
    volatile void *volatile_ptr2 = (void *)trap_fn;
    volatile void *volatile_ptr3 = (void *)clz_fn;
    
    /* Use inline assembly to reference the symbols */
    asm volatile("" : : "i"(__builtin_unreachable), "i"(__builtin_trap));
    
    (void)volatile_ptr1;
    (void)volatile_ptr2;
    (void)volatile_ptr3;
}

int main(void) {
    int result = 0;
    volatile int i, j;
    
    printf("Starting built-in reference test...\n");
    
    /* Loop with complex control flow to process built-in references */
    for (i = 0; i < 10; i++) {
        global_counter++;
        
        /* Different paths reference different built-ins */
        if (i % 3 == 0) {
            /* Path 1: Use generic built-ins */
            result += use_builtins_internal(i);
            
            /* Reference __builtin_unreachable without declaration */
            if (i > 100) {  /* Condition never true */
                __builtin_unreachable();
            }
        } 
        else if (i % 3 == 1) {
            /* Path 2: Use architecture-specific built-ins */
            use_arch_builtins();
            
            /* Take addresses of built-ins */
            take_builtin_addresses();
        }
        else {
            /* Path 3: Direct built-in references */
            result += __builtin_constant_p(i) ? 0 : 1;
            
            /* Reference __builtin_trap conditionally */
            if (global_counter > 1000) {
                __builtin_trap();
            }
        }
        
        /* Nested loop for additional complexity */
        for (j = 0; j < 2; j++) {
            /* Reference built-in in nested context */
            result += (int)__builtin_expect(j == 0, 1);
            
#if defined(__GNUC__) && !defined(__clang__)
            /* More GCC-specific built-ins */
            result += __builtin_ffs(i + j);
#endif
        }
    }
    
    /* Final reference to ensure all paths are considered */
    if (result == 0) {
        /* This should never happen, reference unreachable */
        __builtin_unreachable();
    }
    
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    return result > 0 ? 0 : 1;
}
