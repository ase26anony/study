/* Test program to trigger selective scheduling dump logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays with different types to create varied RTL */
extern int global_arr_int[1024];
extern short global_arr_short[2048];
extern unsigned global_arr_unsigned[512];

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed = 42;
volatile int g_loop_control = 100;

/* Non-inlineable functions */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline)) unsigned non_inline_unsigned(unsigned a, unsigned b) {
    return (a * b) + (a >> 3) - (b << 2);
}

/* Pure functions for scheduling complexity */
__attribute__((const)) int pure_func(int x) {
    return (x * 3) / 2 + 7;
}

__attribute__((const)) short pure_func_short(short x) {
    return (x * 5) % 256;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* arr, int size) {
    register int acc = 0;  /* Hint at register allocation */
    int i;
    
    /* Loop with varying trip count */
    for (i = 0; i < size; ++i) {
        /* Create scheduling boundaries */
        asm volatile ("" ::: "memory");
        
        /* Loop-carried dependency */
        acc = arr[i] + acc;
        
        /* Conditional execution */
        if (__builtin_expect((i & 3) == 0, 0)) {
            acc ^= pure_func(i);
        }
    }
    return acc;
}

/* Core computation with nested loops */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K) {
    int result = 0;
    unsigned outer_acc = 0;
    short inner_acc = 0;
    
    /* Outer loop with different counter type */
    for (unsigned i = 0; i < (unsigned)N; ++i) {
        /* Variable scope inside outer loop */
        int temp = non_inline_func(i, g_volatile_seed);
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (unsigned)K, 0)) {
            /* First inner loop with short counter */
            for (short j = 0; j < (short)M; ++j) {
                /* Memory operations with potential aliasing */
                global_arr_short[j] = (short)(pure_func_short(j) + temp);
                
                /* Multiple uses of same variable */
                inner_acc = (inner_acc + global_arr_short[j]) % 1000;
                inner_acc = inner_acc ^ pure_func_short((short)i);
                
                /* Scheduling barrier */
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Second inner loop with different characteristics */
        for (int j = 0; j < M / 2; ++j) {
            /* Complex expression with mixed types */
            int idx = (i * j) % 512;
            global_arr_unsigned[idx] = non_inline_unsigned(i, j);
            
            /* Loop-carried dependency across outer loop */
            outer_acc += global_arr_unsigned[idx];
            
            /* Conditional branch inside innermost loop */
            if (__builtin_expect((j % 7) == 0, 1)) {
                result += pure_func(j) * (int)outer_acc;
            } else {
                result -= inner_acc;
            }
            
            /* Another scheduling boundary */
            asm volatile ("" ::: "memory");
        }
        
        /* Third conditional loop inside outer loop */
        if (i % 5 == 0) {
            register int reg_var = result;  /* Register hint */
            for (int k = 0; k < 10; ++k) {
                reg_var = non_inline_func(reg_var, k);
                result ^= reg_var;
            }
        }
    }
    
    return result + (int)outer_acc + (int)inner_acc;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warm_up_computation(void) {
    int warm_up_data[64];
    int i;
    
    /* Simple warm-up loop */
    for (i = 0; i < 64; ++i) {
        warm_up_data[i] = i * 3;
    }
    
    /* Call compute_checksum once */
    int warm_result = compute_checksum(warm_up_data, 64);
    
    /* Use result to prevent dead code elimination */
    if (warm_result > 0) {
        g_volatile_seed = warm_result & 255;
    }
}

/* Initialize global arrays with pseudo-random values */
void init_globals(void) {
    /* Simple LCG for pseudo-random values */
    uint32_t lcg_state = g_volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        global_arr_int[i] = (int)(lcg_state % 1000);
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        global_arr_short[i] = (short)(lcg_state % 256);
    }
    
    for (int i = 0; i < 512; ++i) {
        lcg_state = lcg_state * 1103515245 + 12345;
        global_arr_unsigned[i] = lcg_state % 10000;
    }
}

int main(int argc, char** argv) {
    /* Use command line arguments for loop bounds */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 30;
    int K = (argc > 3) ? atoi(argv[3]) : 10;
    
    /* Initialize data */
    init_globals();
    
    /* Warm-up execution */
    warm_up_computation();
    
    /* Main computation with nested loops */
    int final_result = nested_loop_computation(N, M, K);
    
    /* Additional verification computation */
    int checksum = compute_checksum(global_arr_int, 256);
    
    /* Print verifiable results */
    printf("Final result: %d\n", final_result);
    printf("Checksum: %d\n", checksum);
    printf("Combined: %d\n", final_result + checksum);
    
    return 0;
}
