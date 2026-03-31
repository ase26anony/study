/* sel_sched_test.c - Main test program for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays with different types for varied RTL */
static int global_arr_int[1024];
static unsigned short global_arr_short[2048];
static volatile int volatile_seed = 42;

/* Non-inlineable functions */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline, const)) int pure_func(int a, int b) {
    return (a * a) + (b * b);
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* arr, int n) {
    register int acc = 0;
    for (int i = 0; i < n; ++i) {
        acc = arr[i] + acc;
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            acc ^= pure_func(i, arr[i]);
        }
    }
    return acc;
}

/* Secondary file functions (simulated with extern) */
extern void init_arrays(void);
extern int external_compute(int start, int end);

/* Memory barrier macro */
#define MEMORY_BARRIER() asm volatile("" ::: "memory")

/* Core computation with nested loops */
static int __attribute__((noinline)) 
core_computation(int outer_limit, int inner_limit, int threshold) {
    int result = 0;
    unsigned short us_counter;
    int i, j;
    
    /* Outer loop with volatile-dependent bound */
    for (i = 0; i < outer_limit + (volatile_seed & 0x3); ++i) {
        int temp = i * 7;
        
        /* Conditional inner loop execution */
        if (i > threshold) {
            /* Inner loop with different counter type */
            for (us_counter = 0; us_counter < (inner_limit & 0xFF); ++us_counter) {
                /* Mix of operations */
                int idx = (i * 32 + us_counter) % 1024;
                int val = global_arr_int[idx];
                
                /* Loop-carried dependency */
                result = result ^ noinline_func(val, temp);
                
                /* Memory operation with barrier */
                global_arr_short[us_counter] = (result & 0xFFFF);
                MEMORY_BARRIER();
                
                /* Conditional branch */
                if (__builtin_expect((us_counter % 8) == 0, 1)) {
                    result += pure_func(i, us_counter);
                }
                
                /* Another memory access */
                temp += global_arr_int[(idx + 1) % 1024];
            }
        } else {
            /* Different path with simpler loop */
            for (j = 0; j < (inner_limit >> 1); ++j) {
                result -= global_arr_int[(i * 16 + j) % 1024];
                if (j % 4 == 0) {
                    MEMORY_BARRIER();
                    result = noinline_func(result, j);
                }
            }
        }
        
        /* Function call with loop-variant arguments */
        if (i % 16 == 0) {
            result += external_compute(i, i + 8);
        }
    }
    
    return result;
}

/* Warm-up function */
static void warm_up(void) {
    int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += noinline_func(i, i + 1);
        MEMORY_BARRIER();
    }
    /* Use dummy to prevent optimization */
    if (dummy == 0) printf("Warm-up complete\n");
}

int main(int argc, char** argv) {
    int outer_lim = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_lim = (argc > 2) ? atoi(argv[2]) : 100;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 1024; ++i) {
        global_arr_int[i] = (i * 1103515245 + 12345) & 0x7FFF;
    }
    for (int i = 0; i < 2048; ++i) {
        global_arr_short[i] = (i * 1664525 + 1013904223) & 0xFFFF;
    }
    
    /* Warm-up execution */
    warm_up();
    
    /* Main computation */
    int checksum = core_computation(outer_lim, inner_lim, threshold);
    
    /* Additional verification computation */
    int verify = compute_checksum(global_arr_int, 1024);
    
    printf("Result: %d (verify: %d)\n", checksum, verify);
    return 0;
}
