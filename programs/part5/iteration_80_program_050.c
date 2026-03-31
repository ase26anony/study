/* sel_sched_test.c - Test program to trigger selective scheduling dump logic */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern double global_double[512];
static volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int compute_value(int a, int b) {
    return (a * b) + (a >> 3) - (b << 2);
}

__attribute__((noinline, const)) int pure_compute(int x, int y) {
    return x * x + y * y - x * y;
}

__attribute__((noinline)) void memory_barrier() {
    asm volatile ("" ::: "memory");
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int process_chunk(int *data, int start, int end, int init) {
    int acc = init;
    register int i;
    
    /* First level loop with varying trip count */
    for (i = start; i < end; i += 2) {
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            /* Conditional inner loop */
            int j;
            for (j = 0; j < (i % 8) + 1; ++j) {
                acc += data[i] * j;
                /* Memory barrier to create scheduling boundary */
                asm volatile ("" ::: "memory");
            }
        } else {
            acc = compute_value(acc, data[i]);
        }
        
        /* Mix data types for varied RTL */
        short temp = (short)(acc & 0xFFFF);
        unsigned counter = (unsigned)i * 3;
        acc += (int)temp * (counter % 17);
    }
    
    return acc;
}

/* Complex nested loop structure */
__attribute__((noinline)) int nested_loops_computation(int N, int M, int *arr1, int *arr2) {
    int total = 0;
    int i, j, k;
    
    /* Outer loop with volatile-dependent bound */
    int outer_limit = N + (volatile_seed % 5);
    
    for (i = 0; i < outer_limit; ++i) {
        /* Loop with different counter type */
        unsigned u_i = (unsigned)i;
        
        /* Conditional execution of inner loops */
        if (u_i > (unsigned)(M / 2)) {
            /* First inner loop with register variable */
            register int inner_acc = 0;
            for (j = 0; j < M; j += 1 + (i % 3)) {
                inner_acc += arr1[j] * pure_compute(i, j);
                
                /* Memory operation with potential aliasing */
                if (j < 512) {
                    global_double[j] = (double)inner_acc * 0.5;
                }
                
                /* Create scheduling boundary */
                memory_barrier();
            }
            total += inner_acc;
        }
        
        /* Second inner loop with different structure */
        if (__builtin_expect((i % 7) == 0, 1)) {
            int k_limit = M - (i % 4);
            for (k = 0; k < k_limit; ++k) {
                /* Complex expression with multiple operations */
                int val = arr2[k] + (i * k) - (arr1[k] >> 2);
                
                /* Conditional with side effect */
                if (val > 0) {
                    total += val * compute_value(i, k);
                } else {
                    total -= pure_compute(val, k);
                }
                
                /* Array access with different index calculation */
                if ((i + k) < 1024) {
                    global_array[i + k] = val;
                }
            }
        }
        
        /* Innermost loop with short counter */
        for (short s = 0; s < (short)(i % 10); ++s) {
            /* Mixed-type operations */
            double d_temp = (double)total * 0.01 * (double)s;
            total += (int)d_temp;
            
            /* Function call with loop-variant arguments */
            total += compute_value(total, s);
        }
    }
    
    return total;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) void warm_up_computation() {
    int dummy_array[16];
    int i;
    
    /* Simple warm-up loop */
    for (i = 0; i < 16; ++i) {
        dummy_array[i] = i * i;
        asm volatile ("" ::: "memory");
    }
    
    /* Call to ensure function is not optimized away */
    volatile int result = process_chunk(dummy_array, 0, 16, 0);
    (void)result;
}

/* Main computation with rich loop structures */
__attribute__((noinline)) int main_computation(int seed) {
    int array1[256];
    int array2[256];
    int i, result = 0;
    
    /* Initialize with pseudo-random values using LCG */
    int lcg = seed;
    for (i = 0; i < 256; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7FFFFFFF;
        array1[i] = lcg % 1000;
        array2[i] = (lcg >> 8) % 1000;
    }
    
    /* Multiple loop structures with different characteristics */
    
    /* Loop 1: Simple accumulation with dependency */
    int acc = 0;
    for (i = 0; i < 100; ++i) {
        acc = array1[i] + acc;  /* Loop-carried dependency */
        if (i % 13 == 0) {
            acc += compute_value(acc, i);
        }
    }
    result += acc;
    
    /* Loop 2: Nested loops with conditional inner loop */
    int N = 50 + (seed % 10);
    int M = 30 + (seed % 8);
    result += nested_loops_computation(N, M, array1, array2);
    
    /* Loop 3: Process chunks with function calls */
    result += process_chunk(array1, 0, 64, result);
    result += process_chunk(array2, 32, 96, result);
    
    /* Loop 4: Mixed operations with memory barriers */
    for (i = 0; i < 128; i += 3) {
        int temp = array1[i] * array2[i];
        
        /* Conditional with function call */
        if (temp > 50000) {
            result += pure_compute(temp, i);
        } else {
            result -= compute_value(temp, i);
        }
        
        /* Memory barrier every 8 iterations */
        if ((i & 0x7) == 0) {
            asm volatile ("" ::: "memory");
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int seed = 12345;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Starting selective scheduling test with seed: %d\n", seed);
    
    /* Warm-up to potentially trigger different compilation paths */
    warm_up_computation();
    
    /* Main computation with rich loop structures */
    int result = main_computation(seed);
    
    /* Verification output */
    printf("Computation result: %d\n", result);
    printf("Result checksum: 0x%08X\n", (unsigned int)result);
    
    return 0;
}
