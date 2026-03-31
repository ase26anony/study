/* 
 * This program is designed to trigger GCC's default_builtin_extdecl target hook
 * to generate artificial external declarations for built-in functions without
 * prototypes, aiming to cover the flag-setting block in targhooks.cc.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical variables */
static volatile int g_volatile_state = 0;
static volatile int g_volatile_counter = 0;

/* Helper function to create additional scope for built-in references */
static void __attribute__((noinline)) 
use_builtins_without_prototype(int selector) {
    /* Reference architecture-specific built-ins without declarations */
#ifdef __x86_64__
    /* x86 specific built-in - will need external declaration */
    if (selector & 1) {
        void (*fp)(void) = (void (*)(void))__builtin_ia32_rdtsc;
        /* Call through function pointer */
        if (g_volatile_state > 100) {
            /* This branch unlikely but prevents dead code elimination */
            fp();
        }
    }
#endif

#ifdef __arm__
    /* ARM specific built-in */
    if (selector & 2) {
        void (*fp)(void) = (void (*)(void))__builtin_arm_rbit;
        if (g_volatile_state > 200) {
            fp();
        }
    }
#endif

#ifdef __aarch64__
    /* ARM64 specific built-in */
    if (selector & 4) {
        void (*fp)(void) = (void (*)(void))__builtin_aarch64_get_fp;
        if (g_volatile_state > 300) {
            fp();
        }
    }
#endif

    /* Generic built-ins without prototypes */
    if (selector & 8) {
        /* __builtin_expect without prototype */
        long (*exp_fn)(long, long) = (long (*)(long, long))__builtin_expect;
        if (exp_fn(g_volatile_state, 0)) {
            g_volatile_counter++;
        }
    }

    if (selector & 16) {
        /* __builtin_constant_p without prototype */
        int (*const_fn)(int) = (int (*)(int))__builtin_constant_p;
        if (const_fn(g_volatile_state)) {
            g_volatile_counter--;
        }
    }
}

/* Another helper with inline assembly references */
static void __attribute__((noinline))
use_builtins_in_asm(void) {
    /* Use inline assembly to reference built-in names */
#ifdef __x86_64__
    asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
#endif
    
    /* Reference __builtin_trap in assembly context */
    asm volatile("" : : "i"(__builtin_trap) : "memory");
    
    /* Reference __builtin_unreachable */
    asm volatile("" : : "i"(__builtin_unreachable) : "memory");
}

int main(void) {
    int i, result = 0;
    
    /* Initialize volatile state from environment or random source */
    g_volatile_state = (getenv("TEST_SEED") ? atoi(getenv("TEST_SEED")) : 42);
    
    /* Complex loop with multiple built-in references */
    for (i = 0; i < 100; i++) {
        g_volatile_state += i;
        
        /* Different paths trigger different built-in references */
        if (g_volatile_state % 3 == 0) {
            /* Call __builtin_trap without prototype */
            void (*trap_fn)(void) = (void (*)(void))__builtin_trap;
            if (g_volatile_state > 1000) {
                trap_fn();  /* Unlikely to execute */
            }
        } 
        else if (g_volatile_state % 5 == 0) {
            /* Call __builtin_unreachable without prototype */
            void (*unreachable_fn)(void) = (void (*)(void))__builtin_unreachable;
            if (g_volatile_state > 2000) {
                unreachable_fn();  /* Unlikely to execute */
            }
        }
        else if (g_volatile_state % 7 == 0) {
            /* Reference __builtin_cpu_supports without prototype */
            int (*cpu_supports_fn)(const char*) = 
                (int (*)(const char*))__builtin_cpu_supports;
            if (cpu_supports_fn("avx") && g_volatile_state > 500) {
                g_volatile_counter += 2;
            }
        }
        
        /* Use helper functions with built-in references */
        use_builtins_without_prototype(g_volatile_state);
        
        if (i % 13 == 0) {
            use_builtins_in_asm();
        }
        
        /* Mix with other generic built-ins */
        if (g_volatile_state % 11 == 0) {
            /* __builtin_popcount without prototype */
            int (*popcount_fn)(unsigned int) = 
                (int (*)(unsigned int))__builtin_popcount;
            result += popcount_fn(g_volatile_state);
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(g_volatile_state) : : "memory");
    }
    
    /* Compute checksum to ensure side effects */
    result += g_volatile_counter;
    
    /* Print result to prevent elimination */
    printf("Result: %d (State: %d, Counter: %d)\n", 
           result, g_volatile_state, g_volatile_counter);
    
    return result == 0 ? 0 : 1;
}
