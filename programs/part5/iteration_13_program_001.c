/* Built-in function declaration stress test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int state = 0;
static volatile long counter = 0;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void use_builtin_variants(int selector) {
    volatile int local_state = selector;
    
    /* Take addresses of various built-ins without declarations */
    void (*func_ptr)(void);
    
#ifdef __x86_64__
    if (local_state & 1) {
        /* Reference x86-specific built-in without prototype */
        unsigned long long (*rdtsc_ptr)(void) = (unsigned long long (*)(void))__builtin_ia32_rdtsc;
        counter += (long)rdtsc_ptr;
    }
#endif

#ifdef __arm__
    if (local_state & 2) {
        /* Reference ARM-specific built-in without prototype */
        unsigned int (*rbit_ptr)(unsigned int) = (unsigned int (*)(unsigned int))__builtin_arm_rbit;
        counter += (long)rbit_ptr;
    }
#endif

    /* Reference generic built-ins without prototypes */
    if (local_state & 4) {
        int (*expect_ptr)(long, int) = (int (*)(long, int))__builtin_expect;
        counter += (long)expect_ptr;
    }
    
    if (local_state & 8) {
        int (*const_p_ptr)(int) = (int (*)(int))__builtin_constant_p;
        counter += (long)const_p_ptr;
    }
    
    /* Use inline assembly to reference built-in names */
    asm volatile("" : : "i"(__builtin_trap), "i"(__builtin_unreachable));
}

/* Another helper with different control flow */
__attribute__((noinline, noclone))
static int call_through_pointer(int val) {
    volatile int result = 0;
    
    /* Complex control flow around built-in usage */
    for (int i = 0; i < 3; i++) {
        switch ((val + i) % 5) {
            case 0:
                /* Call built-in without declaration */
                __builtin_trap();
                break;
            case 1:
                /* Another built-in call without declaration */
                __builtin_unreachable();
                break;
            case 2:
#ifdef __GNUC__
                /* GCC-specific built-in */
                result += __builtin_popcount(val);
#endif
                break;
            case 3:
                /* Use built-in in expression */
                if (__builtin_expect(val > 100, 0)) {
                    result += 10;
                }
                break;
            case 4:
                /* Constant detection built-in */
                if (__builtin_constant_p(val)) {
                    result += 5;
                }
                break;
        }
        
        /* Mix with target-specific built-ins */
#ifdef __x86_64__
        if (i == 1) {
            /* Call x86 built-in without prototype */
            unsigned long long tsc = __builtin_ia32_rdtsc();
            result += (int)(tsc & 0xFF);
        }
#endif
        
#ifdef __arm__
        if (i == 2) {
            /* Call ARM built-in without prototype */
            unsigned int reversed = __builtin_arm_rbit(val);
            result += reversed & 1;
        }
#endif
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize volatile state */
    state = 1;
    
    /* Loop with complex control flow */
    for (volatile int i = 0; i < 10; i++) {
        int selector = (i * 7) % 16;
        
        /* Call helper that references built-ins */
        use_builtin_variants(selector);
        
        /* Call through function pointer to built-in */
        if (selector & 1) {
            /* Create function pointer to undeclared built-in */
            void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
            
            /* Store in volatile pointer to prevent optimization */
            volatile void (*volatile vtrap_ptr)(void) = trap_ptr;
            
            /* Reference the pointer */
            checksum += (long)vtrap_ptr & 1;
            
            /* Similar for unreachable */
            void (*unreachable_ptr)(void) = (void (*)(void))__builtin_unreachable;
            checksum += (long)unreachable_ptr & 1;
        }
        
        /* Call function with built-in references */
        checksum += call_through_pointer(i);
        
        /* More direct built-in calls without declarations */
        if (selector & 2) {
            /* These calls have no prototypes in scope */
            __builtin_trap();
        } else if (selector & 4) {
            __builtin_unreachable();
        }
        
        /* Use built-ins in conditional expressions */
        int use_fancy = __builtin_expect(selector > 5, 1);
        if (use_fancy) {
#ifdef __GNUC__
            checksum += __builtin_ffs(selector);
#endif
        }
        
        /* Reference built-in via inline assembly constraint */
        asm volatile(
            "nop\n\t"
            : 
            : "i"(__builtin_trap), "i"(__builtin_unreachable),
              "i"(__builtin_expect), "i"(__builtin_constant_p)
        );
        
        state++;
        counter += i;
    }
    
    /* Final built-in references in different contexts */
    {
        /* Nested block with built-in address taken */
        long (*expect_addr)(long, int) = (long (*)(long, int))__builtin_expect;
        checksum += (long)expect_addr & 3;
        
        /* Built-in in ternary operator */
        int maybe_const = __builtin_constant_p(checksum) ? 100 : 200;
        checksum += maybe_const;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (state=%d, counter=%ld)\n", 
           checksum, state, counter);
    
    return checksum == 0 ? 0 : 1;
}

/* Additional global references to built-ins */
static void (*global_trap_ref)(void) = (void (*)(void))__builtin_trap;
static void (*global_unreachable_ref)(void) = (void (*)(void))__builtin_unreachable;

#ifdef __x86_64__
static unsigned long long (*global_rdtsc_ref)(void) = 
    (unsigned long long (*)(void))__builtin_ia32_rdtsc;
#endif

#ifdef __arm__
static unsigned int (*global_rbit_ref)(unsigned int) = 
    (unsigned int (*)(unsigned int))__builtin_arm_rbit;
#endif
