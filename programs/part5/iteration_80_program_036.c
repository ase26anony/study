/* sel_sched_test.c - Test program for selective scheduling dump coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays with different types to create varied RTL */
volatile int g_volatile_seed = 42;
int g_array_int[1024];
unsigned short g_array_short[2048];
float g_array_float[512];
double g_array_double[256];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_compute(int a, int b) {
    return (a * b) ^ (a + b);
}

__attribute__((noinline, const)) int pure_compute(int x, int y) {
    return x * y - (x >> 2) + (y << 1);
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int process_chunk(int start, int end, int init) {
    register int acc = init;  /* Hint register allocation */
    int temp;
    
    for (int i = start; i < end; ++i) {
        /* Memory operations with potential aliasing */
        temp = g_array_int[i % 1024];
        
        /* Loop-carried dependency */
        acc = temp + acc;
        
        /* Conditional operation creating control flow */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            acc = pure_compute(acc, i);
        }
        
        /* Optimization barrier */
        asm volatile ("" ::: "memory");
    }
    
    return acc;
}

/* Core computation with nested loops */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K) {
    int result = 0;
    unsigned outer_counter;
    short inner_counter;
    
    /* Outer loop with unsigned counter */
    for (outer_counter = 0; outer_counter < (unsigned)N; ++outer_counter) {
        int temp_result = 0;
        
        /* Memory access pattern */
        g_array_int[outer_counter % 1024] = outer_counter * 3;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(outer_counter > (unsigned)K, 1)) {
            /* Inner loop with short counter */
            for (inner_counter = 0; inner_counter < (short)M; ++inner_counter) {
                /* Mixed operations with different data types */
                int idx = (outer_counter * 17 + inner_counter * 13) % 1024;
                short s_idx = (short)(idx % 2048);
                
                /* Access different global arrays */
                int val1 = g_array_int[idx];
                unsigned short val2 = g_array_short[s_idx];
                
                /* Arithmetic with type conversions */
                temp_result += (int)val2 * val1;
                
                /* Function call with loop-variant arguments */
                if (__builtin_expect((inner_counter & 0x7) == 0, 0)) {
                    temp_result = noinline_compute(temp_result, inner_counter);
                }
                
                /* Another optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Complex expression with multiple uses of same variable */
                temp_result = temp_result ^ (temp_result >> 3);
                temp_result = pure_compute(temp_result, idx);
            }
        } else {
            /* Different path with simpler computation */
            for (int j = 0; j < 4; ++j) {
                temp_result += g_array_float[(outer_counter + j) % 512] * 2.0f;
            }
        }
        
        /* Combine results with outer loop dependency */
        result ^= temp_result;
        
        /* Conditional based on outer loop index */
        if (outer_counter % 7 == 0) {
            result = process_chunk(0, 16, result);
        }
    }
    
    return result;
}

/* Warm-up function executed once */
__attribute__((noinline)) void warm_up_computation(void) {
    int warm_result = 0;
    
    for (int i = 0; i < 100; ++i) {
        warm_result += i * (i % 13);
        
        /* Create scheduling region with mixed operations */
        if (i % 19 == 0) {
            warm_result = noinline_compute(warm_result, i);
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Use result to prevent optimization */
    g_array_int[0] = warm_result;
}

/* Initialize data with pseudo-random values */
void initialize_data(void) {
    /* Simple LCG for pseudo-random values */
    int lcg = g_volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_int[i] = lcg % 1000;
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_short[i] = (unsigned short)(lcg % 65535);
    }
    
    for (int i = 0; i < 512; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_float[i] = (float)(lcg % 1000) / 100.0f;
    }
    
    for (int i = 0; i < 256; ++i) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_double[i] = (double)(lcg % 1000) / 50.0;
    }
}

int main(int argc, char *argv[]) {
    /* Make loop bounds depend on external inputs */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 30;
    int K = (argc > 3) ? atoi(argv[3]) : 10;
    
    /* Ensure non-zero bounds */
    if (N <= 0) N = 50;
    if (M <= 0) M = 30;
    if (K <= 0) K = 10;
    
    printf("Starting selective scheduling test with N=%d, M=%d, K=%d\n", N, M, K);
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up computation */
    printf("Warm-up computation...\n");
    warm_up_computation();
    
    /* Main computation with nested loops */
    printf("Main computation...\n");
    int final_result = nested_loop_computation(N, M, K);
    
    /* Additional computation with different loop structure */
    int alt_result = 0;
    for (int i = 0; i < N / 2; ++i) {
        for (int j = 0; j < M / 2; ++j) {
            for (int k = 0; k < 3; ++k) {
                alt_result += g_array_int[(i + j + k) % 1024];
                alt_result = pure_compute(alt_result, k);
            }
            asm volatile ("" ::: "memory");
        }
    }
    
    /* Combine results */
    final_result ^= alt_result;
    
    /* Print verifiable result */
    printf("Final checksum: %d (0x%08x)\n", final_result, final_result);
    
    return 0;
}
