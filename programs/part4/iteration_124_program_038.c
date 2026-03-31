/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger selective scheduler debug dumping
 * in GCC's sel-sched-dump.cc, specifically lines 159-163:
 *   switch_dump (stderr);
 *   dump_insn_rtx_1 (insn, debug_insn_rtx_flags);
 *   sel_print ("\n");
 *   restore_dump ();
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops \
 *                -fdump-rtl-sched -fdump-rtl-sched2 -fdump-rtl-all \
 *                -fdump-noaddr -da test_sel_sched_dump.c -o test_sel_sched_dump
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent constant propagation and inlining */
#define NOINLINE __attribute__((noinline))
#define NOOPT __attribute__((optimize("O0")))
#define OPTIMIZE __attribute__((optimize("O2")))

/* Memory barrier to create scheduling boundaries */
#define SCHED_BARRIER() asm volatile("" : : : "memory")

/* Volatile to prevent dead code elimination */
static volatile int global_seed = 42;

/* Helper function to generate complex RTL patterns */
NOINLINE OPTIMIZE
int helper_compute(int a, int b) {
    /* Complex computation with multiple operation types */
    int t1 = a * b;
    int t2 = a + b;
    int t3 = t1 ^ t2;
    int t4 = t3 << 3;
    int t5 = t4 >> 1;
    
    /* Conditional move pattern */
    int result = (t1 > t2) ? t1 : t2;
    result = (result > t5) ? result : t5;
    
    SCHED_BARRIER();
    return result;
}

/* Function with mixed integer/floating point operations */
NOINLINE OPTIMIZE
float mixed_operations(int* arr, float* farr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int idx = i & 0xFF;
        int val = arr[idx] * i;
        isum += val;
        
        /* Floating point operations */
        float fval = farr[idx] * (float)i;
        fsum += fval;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 3 == 0) {
            isum += helper_compute(i, val);
            fsum *= 1.001f;
        } else if (i % 7 == 0) {
            isum -= val / 2;
            fsum /= 1.001f;
        }
        
        /* Builtin function call for complex RTL */
        if (i % 5 == 0) {
            int popcnt = __builtin_popcount(val);
            isum ^= popcnt;
        }
        
        SCHED_BARRIER();
    }
    
    return fsum + (float)isum;
}

/* Outer loop with pipelining opportunities */
NOINLINE OPTIMIZE __attribute__((target("arch=core2")))
int64_t outer_loop_pipelining(int64_t* data, int size) {
    int64_t sum = 0;
    int64_t prod = 1;
    
    /* Outer loop with complex data dependencies */
    for (int i = 0; i < size; i++) {
        int64_t val = data[i];
        
        /* Inner computation with multiple ILP opportunities */
        for (int j = 0; j < 8; j++) {
            int64_t tmp = val * j;
            sum += tmp;
            prod *= (tmp + 1);
            
            /* Memory access pattern */
            if (j % 2 == 0) {
                data[(i + j) % size] = tmp;
            }
        }
        
        /* Complex conditional with ternary operators */
        int64_t adjust = (val > 1000) ? (val >> 3) : 
                        (val > 100) ? (val >> 2) : 
                        (val >> 1);
        
        sum ^= adjust;
        prod |= adjust;
        
        SCHED_BARRIER();
    }
    
    return sum + prod;
}

/* Function with varied RTL instruction types */
NOINLINE OPTIMIZE
void varied_rtl_patterns(short* sarr, long* larr, double* darr, int n) {
    for (int i = 0; i < n; i++) {
        /* Different bit-width operations */
        short sval = sarr[i];
        long lval = larr[i];
        double dval = darr[i];
        
        /* 32-bit and 64-bit mixed operations */
        int ival = (int)sval * (int)(lval & 0xFFFFFFFF);
        long long llval = (long long)ival * lval;
        
        /* Floating point with conversion */
        double dresult = dval * (double)ival + (double)llval;
        
        /* Store results back */
        darr[i] = dresult;
        larr[i] = llval;
        sarr[i] = (short)(ival & 0xFFFF);
        
        /* Call to external function (printf) for call RTL */
        if (i % 100 == 0) {
            printf("Progress: %d\n", i);
        }
        
        SCHED_BARRIER();
    }
}

/* Main test function that combines all patterns */
NOINLINE OPTIMIZE
int64_t comprehensive_test(int iterations) {
    const int ARRAY_SIZE = 1024;
    
    /* Allocate and initialize arrays with volatile to prevent optimization */
    int* int_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_arr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int64_t* int64_arr = (int64_t*)malloc(ARRAY_SIZE * sizeof(int64_t));
    short* short_arr = (short*)malloc(ARRAY_SIZE * sizeof(short));
    long* long_arr = (long*)malloc(ARRAY_SIZE * sizeof(long));
    double* double_arr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        float_arr[i] = (float)int_arr[i] / 1000.0f;
        int64_arr[i] = (int64_t)int_arr[i] * 1000000LL;
        short_arr[i] = (short)(int_arr[i] & 0xFFFF);
        long_arr[i] = (long)int_arr[i];
        double_arr[i] = (double)int_arr[i] / 10000.0;
    }
    
    int64_t total_result = 0;
    
    /* Run multiple test patterns */
    for (int iter = 0; iter < iterations; iter++) {
        /* Test 1: Mixed operations */
        float mixed_result = mixed_operations(int_arr, float_arr, ARRAY_SIZE/4);
        total_result += (int64_t)mixed_result;
        
        SCHED_BARRIER();
        
        /* Test 2: Outer loop pipelining */
        int64_t pipeline_result = outer_loop_pipelining(int64_arr, ARRAY_SIZE/8);
        total_result ^= pipeline_result;
        
        SCHED_BARRIER();
        
        /* Test 3: Varied RTL patterns */
        varied_rtl_patterns(short_arr, long_arr, double_arr, ARRAY_SIZE/16);
        
        /* Update arrays to create data dependencies between iterations */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            int_arr[i] = (int_arr[i] * 1664525 + 1013904223) & 0x7FFFFFFF;
            float_arr[i] = (float)int_arr[i] / 1000.0f;
        }
        
        SCHED_BARRIER();
    }
    
    /* Use results to prevent dead code elimination */
    global_seed = (int)(total_result & 0x7FFFFFFF);
    
    free(int_arr);
    free(float_arr);
    free(int64_arr);
    free(short_arr);
    free(long_arr);
    free(double_arr);
    
    return total_result;
}

/* Simple main to run the test */
int main(int argc, char** argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
        if (iterations > 100) iterations = 100;
    }
    
    printf("Running selective scheduler test with %d iterations...\n", iterations);
    
    int64_t result = comprehensive_test(iterations);
    
    printf("Test completed. Result: %lld\n", (long long)result);
    printf("Global seed updated to: %d\n", global_seed);
    
    return 0;
}
