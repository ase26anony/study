/* sel_sched_test.c - Test program for selective scheduling dump coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
static volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_multiply(int a, int b) {
    return a * b;
}

__attribute__((noinline, const)) int pure_square(int x) {
    return x * x;
}

__attribute__((noinline)) void noinline_side_effect(int* ptr) {
    *ptr += 1;
    asm volatile ("" ::: "memory");
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* data, int size) {
    register int acc = 0;
    for (int i = 0; i < size; ++i) {
        acc = data[i] + acc;
        if (__builtin_expect((acc & 1) == 0, 0)) {
            acc ^= 0x55555555;
        }
    }
    return acc;
}

/* Complex nested loop structure */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K) {
    int result = 0;
    unsigned short us_counter;
    int i, j, k;
    
    /* Outer loop with volatile dependency */
    for (i = 0; i < N + (volatile_seed & 3); ++i) {
        int inner_limit = M;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (K >> 1), 0)) {
            /* First inner loop with different data types */
            for (j = 0; j < inner_limit; j += 2) {
                int temp = noinline_multiply(i, j);
                result += temp;
                
                /* Memory operation with potential aliasing */
                global_array[j] = temp & 0xFF;
                global_short_array[i] = (short)(result & 0xFFFF);
                
                /* Optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Pure function call */
                result ^= pure_square(j);
            }
            
            /* Second inner loop with loop-carried dependency */
            int local_acc = 0;
            for (us_counter = 0; us_counter < (unsigned short)K; ++us_counter) {
                local_acc = global_array[us_counter % 1024] + local_acc;
                
                /* Conditional branch with varying frequency */
                if (us_counter % 7 == 0) {
                    noinline_side_effect(&result);
                } else if (us_counter % 13 == 0) {
                    result += pure_square(local_acc);
                }
                
                /* Mixed-width arithmetic */
                short s_val = (short)(local_acc & 0x7FFF);
                result += (int)s_val * (int)us_counter;
            }
            result += local_acc;
        }
        
        /* Always-executed inner loop with different pattern */
        for (k = i; k < M; k += 3) {
            /* Complex expression with multiple uses of same variable */
            int var = k * 3;
            result += var;
            var = noinline_multiply(var, i);
            result -= var;
            var = pure_square(var);
            result ^= var;
            
            /* Memory barrier */
            asm volatile ("" ::: "memory");
            
            /* Conditional with builtin expect */
            if (__builtin_expect((k & 15) == 0, 1)) {
                global_short_array[k % 2048] = (short)(result & 0xFFFF);
            }
        }
    }
    
    return result;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) void warm_up_computation(void) {
    int warm_data[64];
    for (int i = 0; i < 64; ++i) {
        warm_data[i] = i * 3;
    }
    int warm_sum = compute_checksum(warm_data, 64);
    printf("Warm-up checksum: %d\n", warm_sum);
}

int main(int argc, char* argv[]) {
    /* Initialize with pseudo-random values using LCG */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    for (int i = 0; i < 1024; ++i) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        global_array[i] = seed;
    }
    for (int i = 0; i < 2048; ++i) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        global_short_array[i] = (short)(seed & 0x7FFF);
    }
    
    /* Warm-up execution */
    warm_up_computation();
    
    /* Main computation with varying loop bounds */
    int N = (argc > 2) ? atoi(argv[2]) : 100;
    int M = (argc > 3) ? atoi(argv[3]) : 50;
    int K = (argc > 4) ? atoi(argv[4]) : 30;
    
    printf("Running with N=%d, M=%d, K=%d\n", N, M, K);
    
    int result = nested_loop_computation(N, M, K);
    
    /* Additional computation for more scheduling opportunities */
    int final_checksum = compute_checksum(global_array, 1024);
    final_checksum += compute_checksum((int*)global_short_array, 512); /* 2048 shorts = 512 ints */
    
    printf("Result: %d\n", result);
    printf("Final checksum: %d\n", final_checksum);
    printf("Total: %d\n", result + final_checksum);
    
    return 0;
}
