/* test_sel_sched_dump.c
 * 
 * This test is designed to trigger GCC's selective scheduler debug dumping
 * logic, specifically targeting the dump_insn_rtx_1 calls in sel-sched-dump.cc
 * 
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops 
 *               -fdump-rtl-sched -fdump-rtl-sched2 -fdump-rtl-all -c test_sel_sched_dump.c
 */

#include <stdint.h>
#include <stdio.h>

/* Helper functions to prevent optimization and create diverse RTL patterns */
static volatile int vol_var = 0;
static int global_counter = 0;

/* Non-inline function to generate call RTL */
__attribute__((noinline, optimize("O2"))) 
int helper_compute(int a, int b) {
    return (a * b) + (a ^ b);
}

/* Function with mixed operations to create diverse instruction patterns */
__attribute__((noinline, optimize("O2")))
int mixed_operations_test(int* array, int size) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Create scheduling barriers with inline assembly */
    asm volatile("" : : : "memory");
    
    for (int i = 0; i < size; i++) {
        /* Integer arithmetic with data dependency */
        int val = array[i];
        sum += val * i;
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : (val * 2);
        sum += cond_val;
        
        /* Built-in function for complex RTL */
        sum += __builtin_popcount(val);
        
        /* Floating point operations mixed with integer */
        fsum += (float)val * 0.5f;
        dsum += (double)val * 0.25;
        
        /* Memory barrier to create scheduling regions */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
        
        /* Conditional branch creating multiple basic blocks */
        if (val % 3 == 0) {
            sum += helper_compute(val, i);
        } else if (val % 3 == 1) {
            /* Use volatile to prevent dead code elimination */
            sum += vol_var;
        } else {
            /* Complex expression with multiple operations */
            sum += ((val << 2) | (val >> 28)) & 0xFF;
        }
    }
    
    /* Final memory barrier */
    asm volatile("" : : : "memory");
    
    /* Mix float and int results */
    return sum + (int)fsum + (int)dsum;
}

/* Test with 32-bit and 64-bit operations */
__attribute__((noinline, optimize("O2")))
long long wide_operations_test(int* array, int size) {
    long long lsum = 0;
    int isum = 0;
    
    for (int i = 0; i < size; i++) {
        /* 64-bit operations */
        long long val64 = (long long)array[i] * i;
        lsum += val64;
        
        /* 32-bit operations mixed in */
        isum += array[i] ^ i;
        
        /* Conditional with both 32 and 64 bit */
        if (array[i] > 0) {
            lsum += (long long)isum * 2;
        } else {
            isum -= array[i];
        }
        
        /* Built-in for 64-bit popcount */
        lsum += __builtin_popcountll(val64);
    }
    
    return lsum + isum;
}

/* Function with pointer arithmetic and memory accesses */
__attribute__((noinline, optimize("O2")))
int pointer_heavy_test(int* data, int n) {
    int* ptr = data;
    int sum = 0;
    
    /* Unrolled loop to create more ILP opportunities */
    for (int i = 0; i < n; i += 4) {
        /* Multiple memory accesses */
        int a = ptr[0];
        int b = ptr[1];
        int c = ptr[2];
        int d = ptr[3];
        
        /* Complex dependency chain */
        int t1 = a * b;
        int t2 = c + d;
        int t3 = (t1 > t2) ? t1 : t2;
        sum += t3;
        
        /* Pointer update with arithmetic */
        ptr += 4;
        
        /* Scheduling barrier every 16 iterations */
        if (i % 16 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return sum;
}

/* Outer loop test for -fsel-sched-pipelining-outer-loops */
__attribute__((noinline, optimize("O3")))
int outer_loop_pipelining_test(int* arr1, int* arr2, int size) {
    int total = 0;
    
    /* Nested loops for outer loop pipelining */
    for (int i = 0; i < size; i++) {
        int inner_sum = 0;
        for (int j = 0; j < 16; j++) {
            /* Data-dependent computation */
            inner_sum += arr1[i] * j + arr2[j % size];
            
            /* Mix operations */
            if (j % 2 == 0) {
                inner_sum += __builtin_ctz(arr1[i] | 1);
            }
        }
        total += inner_sum;
        
        /* Volatile access to prevent optimization */
        global_counter++;
    }
    
    return total;
}

/* Test with SIMD-like operations using multiple accumulators */
__attribute__((noinline, optimize("O2")))
int multiple_accumulator_test(int* data, int size) {
    int sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < size; i += 4) {
        /* Multiple independent accumulators for ILP */
        sum0 += data[i] * i;
        sum1 += data[i + 1] * (i + 1);
        sum2 += data[i + 2] * (i + 2);
        sum3 += data[i + 3] * (i + 3);
        
        /* Cross-accumulator dependency to create complexity */
        if (i % 8 == 0) {
            sum0 += sum1;
            sum2 += sum3;
        }
    }
    
    /* Final reduction */
    return sum0 + sum1 + sum2 + sum3;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    int test_array[SIZE];
    int test_array2[SIZE];
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        test_array[i] = (i * 1103515245 + 12345) & 0x7FFF;
        test_array2[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    
    /* Run all tests to exercise different scheduling scenarios */
    int result1 = mixed_operations_test(test_array, SIZE);
    long long result2 = wide_operations_test(test_array, SIZE);
    int result3 = pointer_heavy_test(test_array, SIZE / 4);
    int result4 = outer_loop_pipelining_test(test_array, test_array2, SIZE);
    int result5 = multiple_accumulator_test(test_array, SIZE);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + (int)result2 + result3 + result4 + result5;
    
    /* Use volatile to ensure computation isn't optimized away */
    vol_var = final_result % 1000;
    
    printf("Test Results:\n");
    printf("Mixed ops: %d\n", result1);
    printf("Wide ops: %lld\n", result2);
    printf("Pointer heavy: %d\n", result3);
    printf("Outer loop: %d\n", result4);
    printf("Multi-accum: %d\n", result5);
    printf("Final combined: %d\n", final_result);
    printf("Volatile store: %d\n", vol_var);
    
    return (final_result > 0) ? 0 : 1;
}
