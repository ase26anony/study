/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern volatile int volatile_seed;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline, const)) int pure_func(int x) {
    return (x * 3) / 2;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* data, int size) {
    int acc = 0;
    for (int i = 0; i < size; ++i) {
        acc = data[i] + (acc << 1);
    }
    return acc;
}

/* Function with nested loops and varied operations */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K, 
                                                     int* arr1, int* arr2) {
    int total = 0;
    unsigned short us_counter;
    register int reg_acc = 0;
    
    /* Outer loop with volatile dependency */
    for (int i = 0; i < N; ++i) {
        int inner_limit = M;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > K, 0)) {
            /* First inner loop with short counter */
            for (us_counter = 0; us_counter < (unsigned short)(M / 2); ++us_counter) {
                /* Mix of arithmetic operations */
                int temp = arr1[i] * us_counter;
                temp += pure_func(us_counter);
                
                /* Memory operation with potential aliasing */
                arr2[us_counter] = temp ^ arr2[i];
                
                /* Loop-carried dependency */
                reg_acc = reg_acc + temp;
                
                /* Optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Conditional branch inside innermost loop */
                if (us_counter % 7 == 0) {
                    reg_acc = non_inline_func(reg_acc, temp);
                }
            }
        }
        
        /* Second inner loop with different data type */
        for (unsigned int j = 0; j < (unsigned int)inner_limit; ++j) {
            /* Complex expression with multiple uses of variables */
            int idx = (i * 31 + j * 17) % 1024;
            int val = global_array[idx];
            
            /* Multiple operations on same variable */
            val = val * 2;
            val = val - 1;
            val = val ^ (i * j);
            
            /* Function call with loop-variant arguments */
            val = non_inline_func(val, j);
            
            /* Accumulate with dependency */
            total += val;
            
            /* Another optimization barrier */
            asm volatile ("" ::: "memory");
            
            /* Nested conditional */
            if (__builtin_expect((i + j) % 13 == 0, 1)) {
                total += pure_func(j);
                if (j % 3 == 0) {
                    total -= arr1[i % 256];
                }
            }
        }
        
        /* Variable with different scope */
        {
            int local_scoped = i * i;
            total += local_scoped;
            reg_acc ^= local_scoped;
        }
    }
    
    return total + reg_acc;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) void warm_up_computation(int iterations) {
    int dummy = 0;
    volatile int vol = volatile_seed;
    
    for (int i = 0; i < iterations; ++i) {
        /* Simple loop with barrier */
        dummy += i * vol;
        asm volatile ("" ::: "memory");
        
        /* Conditional execution */
        if (i % 100 == 0) {
            dummy = non_inline_func(dummy, i);
        }
    }
    
    /* Prevent optimization */
    if (dummy == 0x12345678) {
        printf("Impossible\n");
    }
}

/* Initialize arrays with pseudo-random values */
void init_arrays(int* arr1, int* arr2, int size) {
    /* Simple LCG for pseudo-random values */
    unsigned int seed = 123456789;
    
    for (int i = 0; i < size; ++i) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed % 1000);
        arr2[i] = (int)((seed >> 16) % 1000);
    }
}

int main(int argc, char** argv) {
    /* Use command line arguments for variability */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int M = (argc > 2) ? atoi(argv[2]) : 50;
    int K = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Ensure reasonable bounds */
    if (N <= 0) N = 100;
    if (M <= 0) M = 50;
    if (K <= 0) K = 25;
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(1024 * sizeof(int));
    int* arr2 = (int*)malloc(1024 * sizeof(int));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(arr1, arr2, 1024);
    
    /* Warm-up to potentially trigger different compilation paths */
    warm_up_computation(10);
    
    /* Main computation with nested loops */
    int result = nested_loop_computation(N, M, K, arr1, arr2);
    
    /* Additional computation with different loop structure */
    int checksum = compute_checksum(arr1, 256);
    result ^= checksum;
    
    /* Print verifiable result */
    printf("Result: %d (N=%d, M=%d, K=%d)\n", result, N, M, K);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    
    return 0;
}
