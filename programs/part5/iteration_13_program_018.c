/* Built-in function stress test to trigger default_builtin_extdecl hook */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile void *volatile_func_ptr = NULL;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void use_builtins_in_scope(volatile int flag) {
    /* Reference built-ins without declaration */
    if (flag & 1) {
        /* Use generic built-ins */
        int result = __builtin_expect(global_counter > 100, 0);
        (void)result;
        
        /* This should trigger external declaration creation */
        if (global_counter == 0) {
            __builtin_trap();  /* No prototype declared */
        }
    }
    
    if (flag & 2) {
        /* Take address of built-in without declaration */
        void (*fp)(void) = (void (*)(void))__builtin_unreachable;
        volatile_func_ptr = (void*)fp;
        
        /* Use builtin_constant_p without declaration */
        int is_const = __builtin_constant_p(global_counter);
        (void)is_const;
    }
    
#ifdef __x86_64__
    if (flag & 4) {
        /* x86-specific built-in without prototype */
        unsigned long long (*rdtsc_ptr)(void) = 
            (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        volatile_func_ptr = (void*)rdtsc_ptr;
    }
#endif

#ifdef __arm__
    if (flag & 8) {
        /* ARM-specific built-in without prototype */
        unsigned int (*rbit_ptr)(unsigned int) = 
            (unsigned int (*)(unsigned int))__builtin_arm_rbit;
        volatile_func_ptr = (void*)rbit_ptr;
    }
#endif
}

/* Another helper with different control flow */
__attribute__((noinline, cold))
static void complex_builtin_usage(volatile int iter) {
    for (int i = 0; i < iter; i++) {
        /* Mix built-in calls in loop */
        switch (i % 4) {
            case 0:
                /* Reference __builtin_trap without declaration */
                if (global_counter < 0) {
                    __builtin_trap();
                }
                break;
            case 1:
                /* Use inline asm to reference built-in symbol */
                asm volatile("" : : "i"(__builtin_unreachable));
                break;
            case 2:
                /* Function pointer to undeclared built-in */
                {
                    void (*fp)(void) = (void (*)(void))__builtin_expect;
                    if (volatile_func_ptr) {
                        fp = (void (*)(void))volatile_func_ptr;
                    }
                }
                break;
            case 3:
                /* Conditional built-in usage */
                if (__builtin_constant_p(iter)) {
                    global_counter++;
                }
                break;
        }
    }
}

int main(void) {
    volatile int state = 0;
    int checksum = 0;
    
    /* Initialize with random-ish values */
    for (int i = 0; i < 256; i++) {
        state ^= (i * 37) & 0xFF;
    }
    
    /* Main loop with complex control flow around built-in calls */
    for (int i = 0; i < 100; i++) {
        global_counter = i;
        
        /* Vary which built-ins are referenced based on state */
        use_builtins_in_scope(state);
        
        /* More complex pattern */
        complex_builtin_usage(i % 10);
        
        /* Architecture-specific paths */
#ifdef __GNUC__
        /* Generic GNU built-ins without prototypes */
        if (i % 7 == 0) {
            /* These should trigger default_builtin_extdecl */
            int supports = __builtin_cpu_supports("sse2");
            (void)supports;
        }
        
        if (i % 11 == 0) {
            /* Another common built-in */
            void *ptr = __builtin_extract_return_addr(__builtin_return_address(0));
            (void)ptr;
        }
#endif
        
        /* Update state and checksum */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        checksum += (state & 0xFF);
        
        /* Prevent dead code elimination */
        if (volatile_func_ptr && (i % 13 == 0)) {
            /* Try to call through function pointer */
            void (*fp)(void) = (void (*)(void))volatile_func_ptr;
            /* Don't actually call trap/unreachable built-ins */
            if (fp != (void (*)(void))__builtin_trap && 
                fp != (void (*)(void))__builtin_unreachable) {
                /* Could call here if we had a proper function */
            }
        }
    }
    
    /* Use built-ins one more time in different context */
    {
        /* Force declaration in block scope */
        typeof(__builtin_ffs) *ffs_ptr = __builtin_ffs;
        (void)ffs_ptr;
        
        /* Use in inline asm constraint */
        asm volatile(
            "nop\n\t"
            : 
            : "i"(__builtin_clz), "i"(__builtin_ctz)
        );
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum == 0 ? 0 : 1;
}

/* Additional references in different compilation unit context */
__attribute__((constructor))
static void init_builtin_refs(void) {
    /* Reference built-ins in constructor without prototypes */
    volatile int x = __builtin_popcount(0x12345678);
    (void)x;
    
#ifdef __aarch64__
    /* ARM64 specific */
    asm volatile("" : : "i"(__builtin_aarch64_ldaxr));
#endif
    
#ifdef __powerpc__
    /* PowerPC specific */
    asm volatile("" : : "i"(__builtin_ppc_mtfsf));
#endif
}
