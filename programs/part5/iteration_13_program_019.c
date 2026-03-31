/* Built-in function reference test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
volatile int global_counter = 0;
volatile int use_builtin_trap = 0;
volatile int use_builtin_unreachable = 0;
volatile int use_target_builtin = 0;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void reference_builtins_internal(int selector) {
    volatile static int internal_state = 0;
    
    /* Reference various built-ins without prototypes */
    if (selector & 1) {
        /* Force reference to __builtin_expect without declaration */
        if (__builtin_expect(internal_state > 100, 0)) {
            /* This should never execute, but creates the reference */
            internal_state = 0;
        }
    }
    
    if (selector & 2) {
        /* Reference __builtin_constant_p without declaration */
        int is_const = __builtin_constant_p(selector);
        internal_state += is_const ? 1 : 0;
    }
    
    /* Use inline assembly to create additional references */
#ifdef __GNUC__
    /* Reference built-in names in assembly constraints */
    asm volatile("# Builtin references" : : 
                 "i"(__builtin_expect), 
                 "i"(__builtin_constant_p));
#endif
}

/* Another helper with function pointer manipulation */
__attribute__((noinline))
static void use_builtin_pointers(void) {
    /* Create function pointers to undeclared built-ins */
    volatile void (*trap_ptr)(void) = __builtin_trap;
    volatile void (*unreachable_ptr)(void) = __builtin_unreachable;
    
#ifdef __x86_64__
    /* x86-specific built-in */
    volatile unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
    
    /* Call through pointer if conditions are met */
    if (use_target_builtin > 1000) {
        unsigned long long cycles = rdtsc_ptr();
        global_counter += (int)(cycles & 0xFF);
    }
#endif

#ifdef __arm__
    /* ARM-specific built-in */
    volatile unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
    
    if (use_target_builtin > 1000) {
        unsigned int reversed = rbit_ptr(0x12345678);
        global_counter += reversed & 0xFF;
    }
#endif

#ifdef __aarch64__
    /* AArch64-specific built-in */
    volatile unsigned long (*clz_ptr)(unsigned long) = __builtin_aarch64_clz_si;
    
    if (use_target_builtin > 1000) {
        unsigned long leading_zeros = clz_ptr(0xFFFFFFFF);
        global_counter += leading_zeros & 0xFF;
    }
#endif

    /* Store pointers to prevent optimization */
    asm volatile("" : : "r"(trap_ptr), "r"(unreachable_ptr));
}

int main(void) {
    int i, result = 0;
    
    /* Initialize volatile state */
    global_counter = 1;
    use_builtin_trap = 0;
    use_builtin_unreachable = 0;
    use_target_builtin = 42;
    
    /* Complex loop with multiple built-in references */
    for (i = 0; i < 100; i++) {
        global_counter++;
        
        /* Different paths reference different built-ins */
        if (global_counter % 3 == 0) {
            /* Path 1: Reference __builtin_trap without declaration */
            if (use_builtin_trap) {
                __builtin_trap();  /* No prototype provided */
            }
            reference_builtins_internal(1);
        }
        else if (global_counter % 3 == 1) {
            /* Path 2: Reference __builtin_unreachable without declaration */
            if (use_builtin_unreachable) {
                __builtin_unreachable();  /* No prototype provided */
            }
            reference_builtins_internal(3);
        }
        else {
            /* Path 3: Use function pointers to built-ins */
            use_builtin_pointers();
            reference_builtins_internal(7);
        }
        
        /* Mix in some generic built-in references */
        if (i % 7 == 0) {
            /* __builtin_popcount without declaration */
            int popcnt = __builtin_popcount(i);
            result += popcnt;
        }
        
        if (i % 11 == 0) {
            /* __builtin_ffs without declaration */
            int ffs_result = __builtin_ffs(i);
            result += ffs_result;
        }
        
        /* Architecture-specific built-in references */
#ifdef __x86_64__
        if (i % 13 == 0) {
            /* Direct call to x86 built-in without declaration */
            unsigned long long tsc = __builtin_ia32_rdtsc();
            result += (int)(tsc & 0xFF);
        }
#endif
        
#ifdef __arm__
        if (i % 13 == 0) {
            /* Direct call to ARM built-in without declaration */
            unsigned int value = __builtin_arm_rbit(i);
            result += value & 0xFF;
        }
#endif
        
        /* Prevent loop optimization */
        asm volatile("" : "+r"(result));
    }
    
    /* Final built-in references in different contexts */
    {
        /* Nested block with built-in reference */
        volatile int (*expect_ptr)(long, int) = __builtin_expect;
        volatile int (*const_p_ptr)(int) = __builtin_constant_p;
        
        /* Use assembly to reference the symbols */
        asm volatile("# Final references\n\t"
                     : : "i"(__builtin_trap), 
                         "i"(__builtin_unreachable),
                         "i"(__builtin_popcount),
                         "i"(__builtin_ffs));
        
#ifdef __GNUC__
        /* More architecture-specific references */
#ifdef __x86_64__
        asm volatile("" : : "i"(__builtin_ia32_rdtsc));
#endif
#ifdef __arm__
        asm volatile("" : : "i"(__builtin_arm_rbit));
#endif
#ifdef __aarch64__
        asm volatile("" : : "i"(__builtin_aarch64_clz_si));
#endif
#endif
    }
    
    /* Compute checksum to prevent dead code elimination */
    result += global_counter;
    printf("Result: %d\n", result);
    
    /* One more built-in reference at the end */
    if (result > 10000) {
        /* This should never happen, but references the built-in */
        __builtin_unreachable();
    }
    
    return result & 0xFF;
}

/* Additional global references to built-ins */
static volatile void (*global_trap_ref)(void) = __builtin_trap;
static volatile void (*global_unreachable_ref)(void) = __builtin_unreachable;

#ifdef __x86_64__
static volatile unsigned long long (*global_rdtsc_ref)(void) = __builtin_ia32_rdtsc;
#endif

#ifdef __arm__
static volatile unsigned int (*global_rbit_ref)(unsigned int) = __builtin_arm_rbit;
#endif
