/* 
 * This program is designed to trigger GCC's default_builtin_extdecl target hook
 * to generate artificial external declarations for built-in functions,
 * specifically aiming to cover the flag-setting block in targhooks.cc lines 981-990.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Helper function that references undeclared built-ins in complex control flow */
__attribute__((noinline))
static int helper_function(volatile int *state) {
    int result = 0;
    
    /* Complex conditional logic */
    if (*state & 1) {
        /* Reference generic built-in without prototype */
        result = __builtin_expect(*state, 0);
        
        /* Take address of built-in function */
        void (*fp)(void) = (void (*)(void))__builtin_trap;
        use((void*)fp);
        
#ifdef __x86_64__
        /* x86-specific built-in reference */
        unsigned long long (*rdtsc_fn)(void) = 
            (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        use((void*)rdtsc_fn);
#endif
        
#ifdef __arm__
        /* ARM-specific built-in reference */
        unsigned int (*rbit_fn)(unsigned int) = 
            (unsigned int (*)(unsigned int))__builtin_arm_rbit;
        use((void*)rbit_fn);
#endif
    } else if (*state & 2) {
        /* Another built-in without prototype */
        int is_const = __builtin_constant_p(*state);
        result = is_const ? 100 : 200;
        
        /* Use inline assembly to reference built-in symbol */
        asm volatile("# builtin reference %0" : : "i"(__builtin_unreachable));
    } else {
        /* Call built-in through function pointer */
        void (*unreachable_fn)(void) = (void (*)(void))__builtin_unreachable;
        use((void*)unreachable_fn);
        
        /* Complex expression with built-in */
        result = __builtin_constant_p(42) ? 1 : 0;
    }
    
    /* Nested loop with built-in reference */
    for (int i = 0; i < 3; i++) {
        volatile int temp = *state + i;
        if (temp > 10) {
            /* Reference in loop body */
            int (*expect_fn)(long, long) = (int (*)(long, long))__builtin_expect;
            use((void*)expect_fn);
        }
    }
    
    return result;
}

int main(void) {
    volatile int state = 0;
    volatile int checksum = 0;
    
    /* Initialize volatile state from environment or random source */
    char *env = getenv("TEST_SEED");
    if (env) state = atoi(env) % 7;
    
    /* Loop with multiple paths referencing undeclared built-ins */
    for (int i = 0; i < 10; i++) {
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Different paths based on state */
        switch (state % 4) {
            case 0:
                /* Direct call to undeclared built-in */
                __builtin_trap();
                break;
                
            case 1:
                /* Call through function pointer */
                {
                    void (*trap_fn)(void) = (void (*)(void))__builtin_trap;
                    /* Don't actually call it - just take address */
                    use((void*)trap_fn);
                }
                break;
                
            case 2:
                /* Use in expression context */
                checksum += __builtin_constant_p(state) ? 1 : 0;
                break;
                
            case 3:
                /* Call helper with complex control flow */
                checksum += helper_function(&state);
                break;
        }
        
        /* Mix architecture-specific built-ins */
#ifdef __x86_64__
        if (state % 3 == 0) {
            /* x86 built-in without prototype */
            unsigned long long ts = __builtin_ia32_rdtsc();
            checksum ^= (ts & 0xFFFFFFFF);
        }
#endif
        
#ifdef __arm__
        if (state % 5 == 0) {
            /* ARM built-in without prototype */
            unsigned int val = __builtin_arm_rbit(state);
            checksum += val;
        }
#endif
        
        /* Prevent optimization of checksum */
        asm volatile("" : "+r"(checksum));
    }
    
    /* Use inline assembly to reference multiple built-ins */
    asm volatile(
        "# Built-in references\n\t"
        "# %0\n\t"
        "# %1\n\t"
        : 
        : "i"(__builtin_expect), "i"(__builtin_constant_p)
    );
    
    /* Final observable output */
    printf("Checksum: %d\n", checksum);
    
    /* One more built-in reference in cleanup path */
    if (checksum == 0x1234) {  /* Unlikely */
        __builtin_unreachable();
    }
    
    return checksum != 0 ? 0 : 1;
}
