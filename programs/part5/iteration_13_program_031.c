/* Built-in function stress test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile int force_builtin_use = 1;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void use_builtin_variants(int selector) {
    volatile static int local_state = 0;
    
    /* Reference built-ins without prototypes */
    if (selector & 1) {
        /* Force reference to common built-ins */
        if (local_state == 0) {
            /* Call undeclared built-in */
            __builtin_expect(global_counter > 100, 0);
            local_state = 1;
        }
    }
    
    if (selector & 2) {
        /* Another undeclared built-in call */
        int is_const = __builtin_constant_p(selector);
        (void)is_const; /* Suppress unused warning */
    }
    
    /* Take address of built-in functions */
    void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
    void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
    
    /* Store pointers in volatile to prevent optimization */
    volatile void *volatile ptr_storage[2];
    ptr_storage[0] = (void *)trap_ptr;
    ptr_storage[1] = (void *)unreachable_ptr;
    
    /* Use inline assembly to reference built-in names */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
}

/* Architecture-specific built-in usage */
__attribute__((noinline))
static void use_arch_builtins(void) {
#ifdef __x86_64__
    /* x86 specific built-ins */
    unsigned long long (*rdtsc_ptr)(void) = 
        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    
    /* Prevent optimization */
    volatile unsigned long long (*volatile rdtsc_volatile)(void) = rdtsc_ptr;
    
    /* Inline assembly reference */
    asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    
#elif defined(__arm__) || defined(__aarch64__)
    /* ARM specific built-ins */
    unsigned int (*rbit_ptr)(unsigned int) = 
        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
    
    volatile unsigned int (*volatile rbit_volatile)(unsigned int) = rbit_ptr;
    
    /* Inline assembly reference */
    asm volatile("" : : "i"(__builtin_arm_rbit));
#endif

    /* Generic built-in with complex control flow */
    for (volatile int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            /* Conditional built-in usage */
            __builtin_prefetch(&global_counter, 0, 3);
        } else {
            /* Different built-in on alternate path */
            int popcount = __builtin_popcount(i);
            (void)popcount;
        }
    }
}

/* Function that takes built-in addresses as parameters */
__attribute__((noinline))
static void indirect_builtin_call(void (*fn1)(void), void (*fn2)(void)) {
    volatile int should_call = force_builtin_use;
    
    if (should_call > 0) {
        /* Create complex condition to use built-ins */
        switch (global_counter % 4) {
            case 0:
                /* Reference via function pointer */
                if (fn1) {
                    /* This would trap if called, so we just take address */
                    volatile void *addr = (void *)fn1;
                    (void)addr;
                }
                break;
            case 1:
                if (fn2) {
                    volatile void *addr = (void *)fn2;
                    (void)addr;
                }
                break;
            case 2:
                /* Direct undeclared built-in reference */
                __builtin_unreachable();
                break;
            case 3:
                /* Another direct reference */
                __builtin_trap();
                break;
        }
    }
}

int main(void) {
    int checksum = 0;
    
    printf("Starting built-in function reference test...\n");
    
    /* Initialize volatile state */
    global_counter = 1;
    force_builtin_use = 1;
    
    /* Main loop with built-in references */
    for (volatile int iteration = 0; iteration < 10; iteration++) {
        /* Update volatile state */
        global_counter++;
        
        /* Use helper functions with built-in references */
        use_builtin_variants(iteration);
        
        /* Use architecture-specific built-ins */
        use_arch_builtins();
        
        /* Create function pointers to undeclared built-ins */
        void (*trap_func)(void) = (void (*)(void))__builtin_trap;
        void (*unreachable_func)(void) = (void (*)(void))__builtin_unreachable;
        
        /* Call indirect function with built-in addresses */
        indirect_builtin_call(trap_func, unreachable_func);
        
        /* More built-in references in loop body */
        if (iteration % 3 == 0) {
            /* Conditional built-in usage */
            int is_power_of_two = __builtin_popcount(iteration) == 1;
            checksum += is_power_of_two;
        }
        
        if (iteration % 5 == 0) {
            /* Use expect built-in */
            if (__builtin_expect(iteration < 100, 1)) {
                checksum += 2;
            }
        }
        
        /* Reference CPU built-in without prototype */
#ifdef __GNUC__
        /* This built-in often requires declaration */
        int cpu_supports_sse = 0;
        /* Note: We're NOT declaring __builtin_cpu_supports */
        /* The compiler will need to create external declaration */
        asm volatile("" : : "i"(__builtin_cpu_supports));
#endif
        
        /* Additional architecture-specific references */
#if defined(__i386__) || defined(__x86_64__)
        asm volatile("" : : "i"(__builtin_ia32_pause));
#elif defined(__arm__)
        asm volatile("" : : "i"(__builtin_arm_dmb));
#endif
    }
    
    /* Final built-in references outside loop */
    volatile int final_check = __builtin_constant_p(checksum);
    (void)final_check;
    
    /* Use sync built-ins */
    int old_val = __sync_fetch_and_add(&global_counter, 1);
    checksum += old_val;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum == 0 ? 0 : 1;
}
