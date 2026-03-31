/* 
 * This program is designed to trigger the default_builtin_extdecl target hook
 * in GCC's targhooks.cc, specifically the uncovered lines that set special
 * tree flags on an artificial built-in function declaration.
 *
 * Compilation suggestions:
 *   gcc -O2 -fno-builtin -march=native -o test_builtins test_builtins.c
 *   gcc -O0 -fdump-tree-original -fdump-tree-all -o test_builtins test_builtins.c
 *   gcc -O3 -flto -fno-omit-frame-pointer -o test_builtins test_builtins.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent compiler from optimizing away our volatile variables */
static void use_result(int res) {
    volatile int sink = res;
    (void)sink;
}

/* Helper function to create additional scopes for built-in references */
__attribute__((noinline))
static int helper_function(volatile int *state) {
    int local_sum = 0;
    
    /* Reference builtins without prototypes */
#ifdef __GNUC__
    /* Take address of target-specific built-ins */
    void (*fp1)(void) = (void (*)(void))__builtin_ia32_rdtsc;
    void (*fp2)(unsigned int) = (void (*)(unsigned int))__builtin_arm_rbit;
    
    /* Use volatile function pointers to prevent optimization */
    volatile void (*vol_fp1)(void) = fp1;
    volatile void (*vol_fp2)(unsigned int) = fp2;
    
    /* Call through function pointers */
    if (*state & 1) {
        /* This creates a use of the symbol that needs resolution */
        if (vol_fp1) {
            /* Inline asm that references the built-in as operand */
            asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
        }
    }
    
    /* Reference generic built-ins without prototypes */
    if (*state & 2) {
        /* __builtin_expect without prototype */
        int result = __builtin_expect(*state, 0);
        local_sum += result;
    }
    
    if (*state & 4) {
        /* __builtin_constant_p without prototype */
        int is_const = __builtin_constant_p(*state);
        local_sum += is_const;
    }
#endif
    
    return local_sum;
}

int main(void) {
    volatile int state = 0;
    volatile int iteration = 0;
    int checksum = 0;
    
    /* Initialize volatile state */
    state = rand() % 256;
    
    /* Loop with complex control flow around built-in calls */
    for (iteration = 0; iteration < 10; iteration++) {
        int path = (state + iteration) % 8;
        
        switch (path) {
            case 0:
            case 1:
                /* Call __builtin_trap without prototype */
                if (state > 128) {
                    /* Reference in inline asm */
                    asm volatile("" : : "i"(__builtin_trap) : "memory");
                    checksum += 1;
                }
                break;
                
            case 2:
            case 3:
                /* Call __builtin_unreachable without prototype */
                if (state < 64) {
                    /* Reference through function pointer */
                    void (*fp)(void) = (void (*)(void))__builtin_unreachable;
                    volatile void (*vol_fp)(void) = fp;
                    (void)vol_fp;
                    
                    /* Also reference directly in asm */
                    asm volatile("" : : "i"(__builtin_unreachable));
                    checksum += 2;
                }
                break;
                
            case 4:
            case 5:
                /* Use __builtin_cpu_supports without prototype */
                {
                    int supports_sse = 0;
                    /* This built-in requires a string argument */
                    supports_sse = __builtin_cpu_supports("sse");
                    checksum += supports_sse;
                }
                break;
                
            case 6:
            case 7:
                /* Mix architecture-specific built-ins */
#ifdef __x86_64__
                /* __builtin_ia32_rdtsc without prototype */
                {
                    unsigned long long tsc = 0;
                    /* Take address and call */
                    unsigned long long (*rdtsc_ptr)(void) = 
                        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
                    if (rdtsc_ptr) {
                        /* Reference in asm operand */
                        asm volatile("" : : "i"(__builtin_ia32_rdtsc) : "memory");
                        checksum += 3;
                    }
                }
#elif defined(__arm__) || defined(__aarch64__)
                /* __builtin_arm_rbit without prototype */
                {
                    unsigned int (*rbit_ptr)(unsigned int) = 
                        (unsigned int (*)(unsigned int))__builtin_arm_rbit;
                    volatile unsigned int (*vol_rbit)(unsigned int) = rbit_ptr;
                    (void)vol_rbit;
                    
                    /* Reference in asm */
                    asm volatile("" : : "i"(__builtin_arm_rbit));
                    checksum += 4;
                }
#endif
                break;
        }
        
        /* Call helper function which references more built-ins */
        checksum += helper_function(&state);
        
        /* Modify state to change control flow */
        state = (state * 13 + 17) % 256;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(state) : : "memory");
    }
    
    /* Additional references to built-ins in different contexts */
    {
        /* Array of function pointers to built-ins */
        void* builtin_functions[] = {
            (void*)__builtin_trap,
            (void*)__builtin_unreachable,
#ifdef __x86_64__
            (void*)__builtin_ia32_rdtsc,
#elif defined(__arm__) || defined(__aarch64__)
            (void*)__builtin_arm_rbit,
#endif
            (void*)__builtin_expect,
            (void*)__builtin_constant_p
        };
        
        /* Use the array to create additional references */
        for (int i = 0; i < (int)(sizeof(builtin_functions)/sizeof(builtin_functions[0])); i++) {
            checksum += (builtin_functions[i] != NULL);
        }
    }
    
    /* Final checksum computation and output */
    checksum = (checksum * 31) ^ state;
    printf("Result: %d\n", checksum);
    use_result(checksum);
    
    return 0;
}
