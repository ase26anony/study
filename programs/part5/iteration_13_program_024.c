/* Built-in function declaration stress test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of critical variables */
static volatile int global_counter = 0;
static volatile void* volatile_func_ptr = NULL;

/* Helper function to create additional declaration contexts */
static void __attribute__((noinline)) 
use_builtin_indirectly(volatile int selector) {
    /* Reference built-ins without declaration in nested scope */
    if (selector & 1) {
        /* Force reference to __builtin_expect without prototype */
        long (*fp1)(long, long) = (long (*)(long, long))__builtin_expect;
        if (fp1) global_counter++;
    }
    
    if (selector & 2) {
        /* Reference __builtin_constant_p without prototype */
        int (*fp2)(int) = (int (*)(int))__builtin_constant_p;
        if (fp2) global_counter += 2;
    }
    
#ifdef __GNUC__
    /* Architecture-specific built-in references */
#ifdef __x86_64__
    if (selector & 4) {
        /* x86 specific built-in */
        unsigned long long (*fp3)(void) = 
            (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        if (fp3) global_counter += 4;
    }
#endif
    
#ifdef __arm__
    if (selector & 8) {
        /* ARM specific built-in */
        unsigned int (*fp4)(unsigned int) = 
            (unsigned int (*)(unsigned int))__builtin_arm_rbit;
        if (fp4) global_counter += 8;
    }
#endif
    
#ifdef __aarch64__
    if (selector & 16) {
        /* AArch64 specific built-in */
        unsigned long long (*fp5)(unsigned long long) = 
            (unsigned long long (*)(unsigned long long))__builtin_aarch64_rbit;
        if (fp5) global_counter += 16;
    }
#endif
#endif
}

/* Another helper with different control flow */
static int __attribute__((noinline, noclone))
complex_builtin_usage(volatile int x) {
    int result = 0;
    
    /* Loop with built-in references in different paths */
    for (int i = 0; i < 3; i++) {
        volatile int path = x + i;
        
        if (path % 3 == 0) {
            /* Call __builtin_trap without declaration */
            void (*trap_fn)(void) = (void (*)(void))__builtin_trap;
            if (trap_fn && (path % 6 == 0)) {
                /* This might not return, but we're testing declaration creation */
                result += 1;
            }
        } 
        else if (path % 3 == 1) {
            /* Call __builtin_unreachable without declaration */
            void (*unreach_fn)(void) = (void (*)(void))__builtin_unreachable;
            if (unreach_fn) {
                result += 2;
            }
        }
        else {
            /* Use __builtin_expect with inline assembly reference */
            long test_val = path;
            long expected = __builtin_expect(test_val, 0);
            
            /* Inline asm that references the built-in symbol */
            __asm__ volatile (
                "# Force reference to builtin\n"
                : "+r" (expected)
                : "i" (__builtin_expect)
            );
            
            result += expected;
        }
        
        /* Mix with target-specific built-in function pointers */
#ifdef __x86_64__
        if (path % 5 == 0) {
            /* Take address of x86 built-in */
            unsigned long long (*rdtsc_fn)(void) = 
                (unsigned long long (*)(void))__builtin_ia32_rdtsc;
            volatile_func_ptr = (void*)rdtsc_fn;
            
            /* Call through function pointer */
            if (rdtsc_fn) {
                unsigned long long ts = rdtsc_fn();
                result += (ts & 0xFF);
            }
        }
#endif
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    volatile int state = 1;
    
    printf("Starting built-in declaration stress test...\n");
    
    /* Phase 1: Direct references without prototypes */
    for (int i = 0; i < 10; i++) {
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        if (state % 7 == 0) {
            /* Direct call to undeclared __builtin_trap */
            __builtin_trap();
        } 
        else if (state % 7 == 1) {
            /* Direct call to undeclared __builtin_unreachable */
            __builtin_unreachable();
        }
        else if (state % 7 == 2) {
            /* Use __builtin_expect without prototype */
            long val = __builtin_expect(state, 0);
            checksum += val;
        }
        else if (state % 7 == 3) {
            /* Use __builtin_constant_p without prototype */
            int is_const = __builtin_constant_p(state);
            checksum += is_const;
        }
#ifdef __GNUC__
        else if (state % 7 == 4) {
#ifdef __x86_64__
            /* x86 specific built-in call */
            unsigned long long ts = __builtin_ia32_rdtsc();
            checksum += (ts & 0xFF);
#endif
#ifdef __arm__
            /* ARM specific built-in call */
            unsigned int reversed = __builtin_arm_rbit(state);
            checksum += reversed;
#endif
        }
#endif
        
        /* Call helper function that references built-ins indirectly */
        use_builtin_indirectly(state);
        
        /* Complex usage pattern */
        checksum += complex_builtin_usage(state);
        
        /* Prevent dead code elimination */
        if (checksum > 1000000) checksum = checksum % 1000;
    }
    
    /* Phase 2: Function pointer manipulation */
    {
        /* Array of function pointers to various built-ins */
        void* builtin_functions[] = {
            (void*)__builtin_trap,
            (void*)__builtin_unreachable,
            (void*)__builtin_expect,
            (void*)__builtin_constant_p,
#ifdef __x86_64__
            (void*)__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
            (void*)__builtin_arm_rbit,
#endif
            NULL
        };
        
        /* Iterate through function pointers */
        for (int i = 0; builtin_functions[i] != NULL; i++) {
            checksum += ((unsigned long)builtin_functions[i] & 0xFF);
        }
    }
    
    /* Phase 3: Inline assembly with built-in references */
    {
        volatile int asm_input = checksum;
        volatile long asm_output = 0;
        
        /* Inline asm that references multiple built-ins */
        __asm__ volatile (
            "# Multiple built-in references\n"
            "mov %[out], %[in]\n\t"
            : [out] "=r" (asm_output)
            : [in] "r" (asm_input),
              "i" (__builtin_expect),
              "i" (__builtin_constant_p)
#ifdef __x86_64__
              , "i" (__builtin_ia32_rdtsc)
#endif
            : "memory"
        );
        
        checksum = asm_output;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
