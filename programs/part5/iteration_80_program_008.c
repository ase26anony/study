/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays with different types to create varied RTL */
volatile int g_volatile_seed = 42;
int g_array_int[1024];
unsigned int g_array_uint[1024];
short g_array_short[2048];
float g_array_float[512];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func1(int x, int y) {
    asm volatile("" : "+r"(x), "+r"(y) : : "memory");
    return x * 3 + y * 7;
}

__attribute__((noinline)) unsigned noinline_func2(unsigned a, unsigned b) {
    asm volatile("" : "+r"(a), "+r"(b) : : "memory");
    return (a ^ b) + (a & b);
}

__attribute__((noinline)) short noinline_func3(short x, short y) {
    asm volatile("" : "+r"(x), "+r"(y) : : "memory");
    return (short)(x - y * 2);
}

/* Pure function for loop-variant calls */
__attribute__((const)) int pure_func(int x, int y) {
    return (x * x + y * y) % 256;
}

/* Helper with mixed operations */
__attribute__((noinline)) int complex_helper(int idx, int mod) {
    int temp = idx;
    if (temp % 2 == 0) {
        temp = noinline_func1(temp, mod);
    } else {
        temp = pure_func(temp, mod);
    }
    
    /* Memory barrier to create scheduling boundary */
    asm volatile("" ::: "memory");
    
    return temp;
}

/* Core computation with nested loops */
__attribute__((noinline)) 
int compute_nested_loops(int outer_limit, int inner_limit, int step) {
    int result = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int outer_var = i * step;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (outer_limit / 3), 0)) {
            /* First inner loop with short counter */
            for (short j = 0; j < (short)(inner_limit / 2); ++j) {
                /* Loop-carried dependency */
                reg_acc += g_array_int[i] + g_array_short[j];
                
                /* Memory operation with potential aliasing */
                g_array_uint[j] = (unsigned)(reg_acc ^ outer_var);
                
                /* Conditional branch inside innermost loop */
                if (i % 4 == 0) {
                    stack_acc += noinline_func2(g_array_uint[j], outer_var);
                } else if (i % 3 == 0) {
                    stack_acc += pure_func(j, outer_var);
                }
                
                /* Optimization barrier */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Second inner loop with unsigned counter */
        for (unsigned k = 0; k < (unsigned)(inner_limit); ++k) {
            /* Different data type computations */
            int temp = complex_helper(i, k);
            
            /* Multiple uses of same variable */
            reg_acc = reg_acc * 2 - temp;
            stack_acc = stack_acc + temp / 2;
            
            /* Memory access with different index calculation */
            g_array_float[k % 512] = (float)(reg_acc + stack_acc) * 0.5f;
            
            /* Nested conditional with function call */
            if (k % 8 == 0) {
                short s_temp = noinline_func3((short)k, (short)i);
                g_array_short[k] = s_temp;
                
                /* Another scheduling boundary */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Variable with different scope and lifetime */
        {
            int block_scoped = reg_acc + stack_acc;
            result += block_scoped;
            
            /* Conditional based on outer loop index */
            if (i > (outer_limit / 2)) {
                /* Additional computation for later iterations */
                for (int m = 0; m < 3; ++m) {
                    result += noinline_func1(block_scoped, m);
                }
            }
        }
    }
    
    return result;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline))
void warm_up_computation(void) {
    int warm_result = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < 100; ++i) {
        warm_result += pure_func(i, i * 2);
        asm volatile("" ::: "memory");
    }
    
    /* Prevent optimization */
    g_array_int[0] = warm_result;
}

/* Initialize arrays with pseudo-random values */
void init_arrays(void) {
    /* Simple LCG for pseudo-random values */
    uint32_t lcg_state = g_volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        g_array_int[i] = (int)(lcg_state % 1000);
        g_array_uint[i] = lcg_state;
        
        if (i < 512) {
            g_array_float[i] = (float)(lcg_state % 100) * 0.01f;
        }
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        g_array_short[i] = (short)(lcg_state % 1000);
    }
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent optimization of loop bounds */
    volatile int outer_bound = (argc > 1) ? atoi(argv[1]) : 50;
    volatile int inner_bound = (argc > 2) ? atoi(argv[2]) : 100;
    volatile int step_size = (argc > 3) ? atoi(argv[3]) : 3;
    
    printf("Initializing arrays...\n");
    init_arrays();
    
    printf("Warming up...\n");
    warm_up_computation();
    
    printf("Starting main computation...\n");
    printf("Parameters: outer=%d, inner=%d, step=%d\n", 
           outer_bound, inner_bound, step_size);
    
    /* Main computation with nested loops */
    int final_result = compute_nested_loops(outer_bound, inner_bound, step_size);
    
    /* Additional verification computation */
    int verify_sum = 0;
    for (int i = 0; i < 100; ++i) {
        verify_sum += g_array_int[i % 1024];
        verify_sum += g_array_short[i % 2048];
    }
    
    printf("Final result: %d\n", final_result);
    printf("Verification sum: %d\n", verify_sum);
    printf("Checksum: %d\n", final_result + verify_sum);
    
    return 0;
}
