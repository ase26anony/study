/* Built-in function declaration stress test
 * Designed to trigger default_builtin_extdecl in targhooks.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* No prototypes for built-ins - this is intentional! */
/* The compiler should create external declarations via the hook */

/* Helper function with complex control flow */
static void __attribute__((noinline)) 
use_builtins_heavily(volatile int *state) 
{
    volatile int local_state = *state;
    volatile void *func_ptr = NULL;
    
    /* Use architecture-specific built-ins without declaration */
#ifdef __x86_64__
    /* x86 specific built-in - compiler should create external decl */
    if (local_state & 1) {
        func_ptr = (void *)__builtin_ia32_rdtsc;
        /* Call through function pointer */
        unsigned long long (*fp)(void) = (unsigned long long (*)(void))func_ptr;
        if (fp) {
            /* Reference in inline asm too */
            asm volatile("" : : "i"(__builtin_ia32_rdtsc));
        }
    }
#endif

#ifdef __arm__
    /* ARM specific built-in */
    if (local_state & 2) {
        func_ptr = (void *)__builtin_arm_rbit;
        unsigned int (*fp)(unsigned int) = (unsigned int (*)(unsigned int))func_ptr;
        if (fp) {
            asm volatile("" : : "i"(__builtin_arm_rbit));
        }
    }
#endif

#ifdef __aarch64__
    /* AArch64 specific built-in */
    if (local_state & 4) {
        func_ptr = (void *)__builtin_aarch64_rbit;
        asm volatile("" : : "i"(__builtin_aarch64_rbit));
    }
#endif

    /* Generic built-ins without prototypes */
    if (local_state & 8) {
        /* __builtin_expect should trigger external declaration */
        int result = __builtin_expect(local_state > 100, 0);
        
        /* __builtin_constant_p reference */
        int is_const = __builtin_constant_p(local_state);
        
        /* Mix with inline assembly */
        asm volatile("" 
                     : "=r"(result) 
                     : "r"(local_state), "i"(__builtin_expect), "i"(__builtin_constant_p));
    }
    
    /* More aggressive built-in usage in nested conditions */
    for (int i = 0; i < 3; i++) {
        volatile int temp = local_state + i;
        
        if (temp % 5 == 0) {
            /* __builtin_unreachable without prototype */
            if (temp > 1000) {
                __builtin_unreachable();
            }
        } else if (temp % 3 == 0) {
            /* __builtin_trap without prototype */
            if (temp < 0) {
                __builtin_trap();
            }
        } else {
            /* __builtin_popcount without prototype */
            int popcnt = __builtin_popcount(temp);
            asm volatile("" : : "r"(popcnt), "i"(__builtin_popcount));
        }
    }
    
    /* Update state */
    *state = local_state ^ 0x55AA;
}

/* Another helper with different built-in usage pattern */
static int __attribute__((noinline, noipa))
compute_with_builtins(volatile int input) 
{
    volatile int result = input;
    volatile void *builtin_ptrs[5] = {0};
    
    /* Take addresses of various built-ins */
    builtin_ptrs[0] = (void *)__builtin_abs;
    builtin_ptrs[1] = (void *)__builtin_clz;
    builtin_ptrs[2] = (void *)__builtin_ctz;
    builtin_ptrs[3] = (void *)__builtin_ffs;
    builtin_ptrs[4] = (void *)__builtin_parity;
    
    /* Call through function pointers */
    for (int i = 0; i < 5; i++) {
        if (builtin_ptrs[i]) {
            /* Create complex dependency to prevent optimization */
            result += i * 7;
            
            /* Reference in inline asm to ensure symbol usage */
            switch (i) {
                case 0: asm volatile("" : : "i"(__builtin_abs)); break;
                case 1: asm volatile("" : : "i"(__builtin_clz)); break;
                case 2: asm volatile("" : : "i"(__builtin_ctz)); break;
                case 3: asm volatile("" : : "i"(__builtin_ffs)); break;
                case 4: asm volatile("" : : "i"(__builtin_parity)); break;
            }
        }
    }
    
    /* Use __builtin_expect with function pointer */
    int (*expect_fn)(long, int) = (int (*)(long, int))__builtin_expect;
    if (expect_fn) {
        result = expect_fn(result, 0);
    }
    
    /* Architecture-specific built-in function pointer */
#ifdef __x86_64__
    unsigned long long (*rdtsc_fn)(void) = 
        (unsigned long long (*)(void))__builtin_ia32_rdtsc;
    if (rdtsc_fn) {
        asm volatile("" : : "i"(__builtin_ia32_rdtsc));
    }
#endif
    
    return result;
}

int main(void) 
{
    volatile int state = 0x12345678;
    volatile int checksum = 0;
    
    printf("Starting built-in declaration stress test...\n");
    
    /* Loop with multiple paths using different built-ins */
    for (int iteration = 0; iteration < 10; iteration++) {
        volatile int path_selector = state & 0xF;
        
        switch (path_selector) {
            case 0:
            case 1:
                /* Path using __builtin_trap without declaration */
                if (state < 0) {
                    __builtin_trap();
                }
                use_builtins_heavily(&state);
                break;
                
            case 2:
            case 3:
                /* Path using __builtin_unreachable without declaration */
                if (state > 0x7FFFFFFF) {
                    __builtin_unreachable();
                }
                state = compute_with_builtins(state);
                break;
                
            case 4:
            case 5:
                /* Path using __builtin_expect and __builtin_constant_p */
                {
                    int optimized = __builtin_expect(state, 0);
                    int is_constant = __builtin_constant_p(iteration);
                    state = state ^ (optimized * is_constant);
                    
                    /* Inline asm with built-in references */
                    asm volatile("" 
                                : "+r"(state) 
                                : "i"(__builtin_expect), "i"(__builtin_constant_p));
                }
                break;
                
            default:
                /* Mixed built-in usage */
                {
                    /* Take address of __builtin_trap */
                    void (*trap_fn)(void) = (void (*)(void))__builtin_trap;
                    
                    /* Take address of __builtin_unreachable */
                    void (*unreachable_fn)(void) = (void (*)(void))__builtin_unreachable;
                    
                    /* Use them conditionally */
                    if (state & 0x100) {
                        if (trap_fn) {
                            /* Reference in asm */
                            asm volatile("" : : "i"(__builtin_trap));
                        }
                    } else {
                        if (unreachable_fn) {
                            asm volatile("" : : "i"(__builtin_unreachable));
                        }
                    }
                    
                    /* More architecture-specific built-ins */
#ifdef __GNUC__
                    /* GCC vector built-ins */
                    typedef int v4si __attribute__((vector_size(16)));
                    v4si a = {1, 2, 3, 4};
                    v4si b = {5, 6, 7, 8};
                    v4si c = __builtin_ia32_paddd128(a, b);
                    (void)c;
#endif
                }
                break;
        }
        
        /* Update checksum */
        checksum ^= state;
        
        /* Modify state for next iteration */
        state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final computation using built-in without prototype */
    checksum += __builtin_popcount(checksum);
    
    /* Use __builtin_ffs without prototype */
    int first_set = __builtin_ffs(checksum);
    
    printf("Checksum: %d (first bit set at position %d)\n", 
           checksum, first_set);
    
    /* One more built-in reference before exit */
    if (checksum == 0) {
        __builtin_unreachable();
    }
    
    return checksum != 0 ? 0 : 1;
}

/* Additional global references to built-ins */
static volatile void *global_builtin_refs[] = {
    (void *)__builtin_trap,
    (void *)__builtin_unreachable,
    (void *)__builtin_expect,
    (void *)__builtin_constant_p,
#ifdef __x86_64__
    (void *)__builtin_ia32_rdtsc,
#endif
#ifdef __arm__
    (void *)__builtin_arm_rbit,
#endif
    0 /* Sentinel */
};
