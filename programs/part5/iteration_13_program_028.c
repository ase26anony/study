/* Built-in function declaration stress test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static volatile int global_counter = 0;
static volatile int use_builtin = 1;

/* Helper function to create additional scope for built-in references */
__attribute__((noinline))
static void use_builtin_functions(int selector) {
    volatile static int local_state = 0;
    
    /* Take address of undeclared built-in functions */
    void (*builtin_ptr)(void);
    int (*builtin_int_ptr)(void);
    
    /* Architecture-specific built-in references without prototypes */
#ifdef __x86_64__
    /* x86 specific built-ins */
    if (selector & 1) {
        /* Reference __builtin_ia32_rdtsc without declaration */
        builtin_int_ptr = (int (*)(void))__builtin_ia32_rdtsc;
        local_state += (long)builtin_int_ptr;
    }
#endif

#ifdef __arm__
    /* ARM specific built-ins */
    if (selector & 2) {
        /* Reference __builtin_arm_rbit without declaration */
        builtin_int_ptr = (int (*)(void))__builtin_arm_rbit;
        local_state += (long)builtin_int_ptr;
    }
#endif

#ifdef __aarch64__
    /* ARM64 specific built-ins */
    if (selector & 4) {
        /* Reference __builtin_aarch64_get_fp without declaration */
        builtin_int_ptr = (int (*)(void))__builtin_aarch64_get_fp;
        local_state += (long)builtin_int_ptr;
    }
#endif

    /* Generic built-in references without prototypes */
    if (selector & 8) {
        /* Reference __builtin_trap without declaration */
        builtin_ptr = (void (*)(void))__builtin_trap;
        /* Store pointer to prevent optimization */
        asm volatile("" : : "r"(builtin_ptr));
    }
    
    if (selector & 16) {
        /* Reference __builtin_unreachable without declaration */
        builtin_ptr = (void (*)(void))__builtin_unreachable;
        asm volatile("" : : "r"(builtin_ptr));
    }
    
    /* Use inline assembly to reference built-in names */
    asm volatile(
        "# Force references to built-ins\n"
        "1:\n"
        ".ifdef __x86_64__\n"
        ".word %c0\n"
        ".endif\n"
        : : "i"(__builtin_ia32_rdtsc)
    );
    
    global_counter += local_state;
}

/* Another helper with different control flow */
__attribute__((noinline, cold))
static void conditional_builtin_use(int mode) {
    volatile int temp = mode;
    
    /* Complex control flow around built-in calls */
    for (int i = 0; i < 3; i++) {
        switch ((temp + i) % 5) {
            case 0:
                /* Call __builtin_expect without prototype */
                if (__builtin_expect(temp > 100, 0)) {
                    temp--;
                }
                break;
            case 1:
                /* Call __builtin_constant_p without prototype */
                if (__builtin_constant_p(temp)) {
                    temp += 2;
                }
                break;
            case 2:
                /* Reference __builtin_popcount without prototype */
                {
                    int (*popcnt_ptr)(int) = (int (*)(int))__builtin_popcount;
                    temp += popcnt_ptr(temp);
                }
                break;
            case 3:
                /* Reference __builtin_clz without prototype */
                {
                    int (*clz_ptr)(int) = (int (*)(int))__builtin_clz;
                    temp += clz_ptr(temp | 1);
                }
                break;
            case 4:
                /* Reference __builtin_ctz without prototype */
                {
                    int (*ctz_ptr)(int) = (int (*)(int))__builtin_ctz;
                    temp += ctz_ptr(temp | 1);
                }
                break;
        }
        
        /* Mix with inline assembly referencing built-ins */
        asm volatile(
            "/* Assembly constraint referencing built-in */\n"
            : "=r"(temp)
            : "0"(temp),
              "i"(__builtin_expect),
              "i"(__builtin_constant_p)
        );
    }
    
    global_counter += temp;
}

int main(void) {
    volatile int state = 0;
    int result = 0;
    
    printf("Starting built-in declaration stress test...\n");
    
    /* Loop with volatile condition to prevent optimization */
    for (int i = 0; i < 10; i++) {
        state = global_counter + i;
        
        /* Call helper functions that reference undeclared built-ins */
        use_builtin_functions(state);
        conditional_builtin_use(state);
        
        /* Direct calls to undeclared built-ins in main control flow */
        if (state & 1) {
            /* These calls have no prototypes - should trigger default_builtin_extdecl */
            int r = __builtin_ffs(state);
            result += r;
        }
        
        if (state & 2) {
            /* Another undeclared built-in call */
            int r = __builtin_parity(state);
            result ^= r;
        }
        
        if ((state & 4) && use_builtin) {
            /* Function pointer to undeclared built-in */
            void (*trap_ptr)(void) = (void (*)(void))__builtin_trap;
            /* Store but don't call (would terminate program) */
            asm volatile("" : : "r"(trap_ptr));
        }
        
        if (state & 8) {
            /* Reference __builtin_frame_address without prototype */
            void *frame = __builtin_frame_address(0);
            result += (long)frame & 0xFF;
        }
        
        /* Architecture-specific direct calls */
#ifdef __x86_64__
        if (state & 16) {
            /* x86 built-in without prototype */
            unsigned long long tsc = __builtin_ia32_rdtsc();
            result += tsc & 0xFF;
        }
#endif
        
#ifdef __arm__
        if (state & 32) {
            /* ARM built-in without prototype */
            unsigned int rbit = __builtin_arm_rbit(state);
            result += rbit & 0xFF;
        }
#endif
        
        /* Prevent infinite loops with __builtin_unreachable hint */
        if (i == 9) {
            /* Reference without prototype */
            if (__builtin_expect(result == 0, 0)) {
                /* This should never happen, but references __builtin_unreachable */
                void (*unreachable_ptr)(void) = 
                    (void (*)(void))__builtin_unreachable;
                asm volatile("" : : "r"(unreachable_ptr));
            }
        }
        
        /* Additional inline assembly pressure */
        asm volatile(
            "# Multiple built-in references in constraints\n"
            "add %0, %0, %1\n"
            : "+r"(result)
            : "r"(i),
              "i"(__builtin_ffs),
              "i"(__builtin_parity),
              "i"(__builtin_frame_address)
        );
    }
    
    /* Compute final checksum */
    result += global_counter & 0xFF;
    result += state & 0xFF;
    
    printf("Result checksum: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    /* Use __builtin_return_address without prototype */
    void *ret_addr = __builtin_return_address(0);
    printf("Return address hint: %p\n", ret_addr);
    
    return result & 0x7F;  /* Return non-zero to indicate execution */
}
