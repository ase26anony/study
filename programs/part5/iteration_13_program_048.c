/* Built-in function declaration pressure test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile void *volatile_func_ptr = NULL;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline, cold))
static int use_builtins_internal(int selector) {
    volatile int local_result = 0;
    
    /* Reference built-ins without declarations */
    if (selector & 1) {
        /* Force reference to common built-ins */
        local_result += (int)__builtin_expect(selector > 0, 1);
    }
    
    if (selector & 2) {
        /* Reference constant detection built-in */
        local_result += __builtin_constant_p(selector) ? 10 : 20;
    }
    
#ifdef __x86_64__
    if (selector & 4) {
        /* x86-specific built-in - will need declaration */
        /* Take address to force symbol reference */
        void (*rdtsc_ptr)(void) = (void (*)(void))__builtin_ia32_rdtsc;
        volatile_func_ptr = (void *)rdtsc_ptr;
        
        /* Also reference in inline asm */
        asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    }
#endif

#ifdef __arm__
    if (selector & 4) {
        /* ARM-specific built-in */
        void (*rbit_ptr)(void) = (void (*)(void))__builtin_arm_rbit;
        volatile_func_ptr = (void *)rbit_ptr;
        
        /* Inline asm reference */
        asm volatile("" : : "i"(__builtin_arm_rbit));
    }
#endif

#ifdef __aarch64__
    if (selector & 8) {
        /* AArch64-specific built-in */
        asm volatile("" : : "i"(__builtin_aarch64_get_fpcr));
    }
#endif

    return local_result;
}

/* Another helper with different built-in usage pattern */
__attribute__((noinline, hot))
static void call_through_pointer(void) {
    /* Create function pointer to undeclared built-in */
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    
    /* Store in volatile to prevent optimization */
    volatile void *volatile_ptr = (void *)trap_ptr;
    
    /* Reference unreachable built-in */
    if (global_counter > 1000) {
        __builtin_unreachable();
    }
    
    /* Reference in asm */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
}

/* Function that mixes various built-in references */
__attribute__((noinline))
static int builtin_stress_test(int iterations) {
    volatile int result = 0;
    volatile int i = 0;
    
    /* Function pointer array for various built-ins */
    void *(*func_ptrs[4])(void);
    
    /* Take addresses of undeclared built-ins */
    func_ptrs[0] = (void *(*)(void))__builtin_return_address;
    func_ptrs[1] = (void *(*)(void))__builtin_frame_address;
    func_ptrs[2] = (void *(*)(void))__builtin_extract_return_addr;
    
#ifdef __GNUC__
    /* GCC-specific built-ins */
    func_ptrs[3] = (void *(*)(void))__builtin_cpu_init;
    
    /* Reference CPU support built-in without declaration */
    if (__builtin_cpu_supports("sse")) {
        result += 1;
    }
#endif
    
    for (i = 0; i < iterations; i++) {
        int selector = (global_counter + i) & 0xF;
        
        /* Call helper that references built-ins */
        result += use_builtins_internal(selector);
        
        /* Alternate between different control flow paths */
        switch (selector % 3) {
            case 0:
                /* Path that might use trap */
                if (selector == 0) {
                    /* Reference trap built-in */
                    asm volatile("" : : "i"(__builtin_trap));
                }
                break;
                
            case 1:
                /* Path with unreachable */
                if (result > 100) {
                    /* Reference will force declaration */
                    asm volatile("" : : "i"(__builtin_unreachable));
                }
                break;
                
            case 2:
                /* Path with expect */
                result += (int)__builtin_expect(result < 50, 1);
                break;
        }
        
        /* Periodically call through function pointer */
        if ((i % 7) == 0) {
            call_through_pointer();
        }
        
        /* Mix in some math built-ins */
        result += __builtin_ffs(selector | 1);
        result += __builtin_popcount(selector);
        
        /* Reference abs built-in without declaration */
        result += __builtin_abs(result);
        
        global_counter++;
    }
    
    return result;
}

int main(void) {
    volatile int seed = 0;
    
    /* Initialize with some volatile reads */
    asm volatile("" : "=r"(seed) : : "memory");
    
    int iterations = 100 + (seed & 0xFF);
    printf("Starting built-in stress test with %d iterations\n", iterations);
    
    /* Run the test */
    int result = builtin_stress_test(iterations);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (global counter: %d)\n", result, global_counter);
    
    /* Final references to ensure all built-ins are touched */
    asm volatile("" : : 
        "i"(__builtin_trap),
        "i"(__builtin_unreachable),
        "i"(__builtin_expect),
        "i"(__builtin_constant_p),
        "i"(__builtin_ffs),
        "i"(__builtin_popcount),
        "i"(__builtin_abs)
    );
    
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
    
#ifdef __arm__
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
    
    return (result > 0) ? 0 : 1;
}
