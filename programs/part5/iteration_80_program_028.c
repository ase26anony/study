/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global arrays with different types to create varied RTL */
static int global_arr_int[1024];
static unsigned short global_arr_short[2048];
static volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int compute_value(int x, int y) {
    return (x * 3 + y * 7) & 0xFF;
}

__attribute__((noinline)) unsigned short transform_short(unsigned short val) {
    return (val ^ 0x55AA) + 1;
}

/* Pure function for loop-invariant computation */
__attribute__((const)) int pure_mult(int a, int b) {
    return a * b;
}

/* External function declaration (defined in another file) */
extern void process_chunk(int* data, int size, int factor);

/* Memory barrier function */
static inline void memory_barrier(void) {
    asm volatile ("" ::: "memory");
}

/* Loop with complex control flow and dependencies */
static long long complex_nested_loops(int outer_limit, int inner_limit, 
                                      int threshold, int* checksum) {
    long long total = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int outer_mod = i % 4;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > threshold, 0)) {
            /* Inner loop with different counter type */
            for (unsigned j = 0; j < (unsigned)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int idx = (i * 31 + j * 17) & 1023;
                short s_idx = (short)(j & 2047);
                
                /* Loop-carried dependency */
                reg_acc = global_arr_int[idx] + reg_acc;
                
                /* Memory operation with potential aliasing */
                global_arr_short[s_idx] = transform_short(global_arr_short[s_idx]);
                
                /* Conditional branch creating control flow */
                if ((i * j) % 8 == 0) {
                    int temp = compute_value(i, j);
                    stack_acc += temp;
                    
                    /* Use of pure function */
                    int prod = pure_mult(i, j);
                    total += prod;
                }
                
                /* Optimization barrier */
                memory_barrier();
                
                /* Multiple uses of same variable */
                reg_acc = reg_acc ^ (reg_acc >> 3);
                reg_acc += global_arr_int[(idx + 1) & 1023];
            }
        } else {
            /* Different path with its own loop */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                int val = compute_value(i, k);
                total += val;
                
                /* Array access with different stride */
                global_arr_int[k * 3] = val;
            }
        }
        
        /* Function call that might not be inlined */
        if (i % 16 == 0) {
            process_chunk(global_arr_int, 64, i);
        }
    }
    
    *checksum = reg_acc + stack_acc;
    return total;
}

/* Another loop structure with different characteristics */
static double floating_point_loop(int iterations, double* result) {
    double acc = 0.0;
    volatile double vol_acc = 0.0;
    
    /* Loop with floating point operations */
    for (int i = 0; i < iterations; ++i) {
        double x = (double)i / 100.0;
        
        /* Create FP dependencies */
        for (int j = 0; j < 10; ++j) {
            x = x * 1.1 + 0.5;
            
            /* Conditional FP computation */
            if (i % (j + 2) == 0) {
                acc += x;
            } else {
                acc -= x * 0.5;
            }
            
            /* Volatile access to prevent optimization */
            vol_acc = x;
        }
        
        /* Mix integer and FP */
        int int_part = (int)x;
        global_arr_int[i & 1023] = int_part;
    }
    
    *result = acc + vol_acc;
    return acc;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) static void warm_up_computation(void) {
    int dummy_sum = 0;
    long long dummy_total = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 50; ++j) {
            dummy_total += compute_value(i, j);
            dummy_sum += global_arr_int[(i + j) & 1023];
        }
    }
    
    /* Prevent dead code elimination */
    if (dummy_sum > 1000000) {
        printf("Unexpected warm-up result\n");
    }
}

/* Initialize data with pseudo-random values */
static void initialize_data(void) {
    unsigned int lcg = volatile_seed;
    
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr_int[i] = (int)(lcg & 0x7FFF);
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr_short[i] = (unsigned short)(lcg & 0xFFFF);
    }
}

int main(int argc, char* argv[]) {
    /* Use arguments to make loop bounds non-constant */
    int outer_loops = (argc > 1) ? atoi(argv[1]) : 100;
    int inner_loops = (argc > 2) ? atoi(argv[2]) : 50;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    
    if (outer_loops <= 0) outer_loops = 100;
    if (inner_loops <= 0) inner_loops = 50;
    if (threshold < 0) threshold = 25;
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up to potentially trigger different compilation paths */
    warm_up_computation();
    
    /* Main computation with nested loops */
    int checksum1 = 0;
    long long result1 = complex_nested_loops(outer_loops, inner_loops, 
                                            threshold, &checksum1);
    
    /* Second computation with different characteristics */
    double fp_result = 0.0;
    double result2 = floating_point_loop(outer_loops / 2, &fp_result);
    
    /* Combine results to prevent optimization */
    long long final_result = result1 + (long long)result2 + checksum1;
    
    printf("Computation complete\n");
    printf("Result1: %lld, Checksum1: %d\n", result1, checksum1);
    printf("Result2: %f, FP Result: %f\n", result2, fp_result);
    printf("Final combined: %lld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
