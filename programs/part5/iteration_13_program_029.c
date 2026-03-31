/* Built-in function stress test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile void *volatile_func_ptr = NULL;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline, cold))
static void use_builtin_variants(int selector) {
    volatile int local_volatile = selector;
    
    /* Reference multiple built-ins without prototypes */
    if (local_volatile & 1) {
        /* Force reference to common built-ins */
        (void)__builtin_expect(local_volatile, 0);
        (void)__builtin_constant_p(local_volatile);
    }
    
    if (local_volatile & 2) {
        /* Reference trap/unreachable built-ins */
        if (local_volatile > 1000) {
            __builtin_trap();  /* No prototype */
        } else if (local_volatile < 0) {
            __builtin_unreachable();  /* No prototype */
        }
    }
    
    /* Architecture-specific built-in references */
#ifdef __x86_64__
    if (local_volatile & 4) {
        /* x86 specific built-in without prototype */
        unsigned long long (*rdtsc_ptr)(void) = __builtin_ia32_rdtsc;
        if (rdtsc_ptr) {
            /* Call through function pointer */
            unsigned long long cycles = rdtsc_ptr();
            local_volatile = (int)(cycles & 0xFF);
        }
    }
#endif

#ifdef __arm__
    if (local_volatile & 8) {
        /* ARM specific built-in without prototype */
        unsigned int (*rbit_ptr)(unsigned int) = __builtin_arm_rbit;
        if (rbit_ptr) {
            unsigned int reversed = rbit_ptr(local_volatile);
            local_volatile = reversed & 0xFF;
        }
    }
#endif

    /* Use inline assembly to reference built-in names */
    asm volatile("# Builtin reference marker" : : "i"(__builtin_trap));
    asm volatile("# Another reference" : : "i"(__builtin_unreachable));
}

/* Another helper with different built-in usage pattern */
__attribute__((noinline, hot))
static int builtin_through_pointer(int input) {
    volatile int result = input;
    
    /* Create function pointers to undeclared built-ins */
    void (*trap_ptr)(void) = __builtin_trap;
    void (*unreachable_ptr)(void) = __builtin_unreachable;
    
    /* Store pointers in volatile to prevent optimization */
    volatile_func_ptr = trap_ptr;
    
    /* Conditional call through pointer */
    if (result % 7 == 0 && trap_ptr) {
        /* This creates a use of the built-in symbol */
        asm volatile("" : : "r"(trap_ptr));
    }
    
    if (result % 13 == 0 && unreachable_ptr) {
        /* Another use through different pointer */
        asm volatile("" : : "r"(unreachable_ptr));
    }
    
    /* Reference CPU built-ins without prototypes */
#ifdef __GNUC__
    if (result % 3 == 0) {
        /* __builtin_cpu_supports without prototype */
        int (*cpu_supports_ptr)(const char*) = __builtin_cpu_supports;
        if (cpu_supports_ptr) {
            int has_sse = cpu_supports_ptr("sse");
            result ^= has_sse ? 0x55 : 0xAA;
        }
    }
    
    if (result % 5 == 0) {
        /* __builtin_popcount without prototype */
        int (*popcount_ptr)(unsigned int) = __builtin_popcount;
        if (popcount_ptr) {
            result += popcount_ptr(result);
        }
    }
#endif
    
    return result;
}

int main(void) {
    volatile int state = 0;
    int checksum = 0;
    
    printf("Starting built-in function stress test...\n");
    
    /* Loop with complex control flow around built-in calls */
    for (int i = 0; i < 100; ++i) {
        global_counter++;
        state ^= i;
        
        /* Different paths trigger different built-in references */
        if (state & 0x01) {
            /* Path 1: Direct built-in calls without prototypes */
            (void)__builtin_expect(state, 0);
            (void)__builtin_constant_p(state);
        }
        
        if (state & 0x02) {
            /* Path 2: Trap/unreachable on certain conditions */
            if (state > 50) {
                /* Reference without call to avoid actual trap */
                void (*trap_ref)(void) = __builtin_trap;
                asm volatile("" : : "r"(trap_ref));
            } else if (state < 10) {
                void (*unreachable_ref)(void) = __builtin_unreachable;
                asm volatile("" : : "r"(unreachable_ref));
            }
        }
        
        /* Call helper functions that reference built-ins */
        use_builtin_variants(state);
        state = builtin_through_pointer(state);
        
        /* Architecture-specific path */
#ifdef __x86_64__
        if (state & 0x04) {
            /* Take address of x86 specific built-in */
            unsigned long long (*rdtsc_fptr)(void) = __builtin_ia32_rdtsc;
            volatile_func_ptr = rdtsc_fptr;
            
            /* Use in inline assembly constraint */
            asm volatile("" : : "i"(__builtin_ia32_rdtsc));
        }
#endif
        
#ifdef __aarch64__
        if (state & 0x08) {
            /* ARM64 specific built-in */
            void (**clrex_ptr)(void) = __builtin_arm_clrex;
            asm volatile("" : : "r"(clrex_ptr));
        }
#endif
        
        /* Mix with other common built-ins */
        if (state % 11 == 0) {
            /* __builtin_ffs without prototype */
            int (*ffs_ptr)(int) = __builtin_ffs;
            if (ffs_ptr) {
                state += ffs_ptr(state);
            }
        }
        
        if (state % 17 == 0) {
            /* __builtin_clz without prototype */
            int (*clz_ptr)(unsigned int) = __builtin_clz;
            if (clz_ptr) {
                state ^= clz_ptr(state);
            }
        }
        
        checksum += state;
    }
    
    /* Final forced references in different context */
    {
        /* Create array of function pointers to built-ins */
        void *builtin_refs[] = {
            (void*)__builtin_trap,
            (void*)__builtin_unreachable,
#ifdef __x86_64__
            (void*)__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
            (void*)__builtin_arm_rbit,
#endif
            (void*)__builtin_expect,
            (void*)__builtin_constant_p
        };
        
        /* Use the array to prevent optimization */
        asm volatile("" : : "r"(builtin_refs));
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum == 0 ? 0 : 1;
}
