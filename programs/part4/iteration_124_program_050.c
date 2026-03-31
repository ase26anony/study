/* test_sel_sched_dump.c
 * 
 * This program is designed to trigger GCC's selective scheduler debug dumping
 * functionality, specifically targeting the dump_insn_rtx_1 calls in
 * sel-sched-dump.cc lines 159-163.
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops \
 *                -fdump-rtl-sched -fdump-rtl-sched2 -fdump-rtl-all \
 *                -fdump-noaddr -da test_sel_sched_dump.c -o test_sel_sched_dump
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent constant propagation and ensure code isn't optimized away */
static volatile int global_seed = 42;

/* Non-inline helper functions to generate call RTL */
int __attribute__((noinline, optimize("O2"))) helper_mul(int a, int b) {
    return a * b + global_seed;
}

float __attribute__((noinline, optimize("O2"))) helper_fmul(float a, float b) {
    return a * b + (float)global_seed;
}

/* Function with complex mixed operations to generate diverse RTL */
int __attribute__((noinline, optimize("O2"), target("arch=core2")))
test_mixed_operations(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    int i;
    
    /* Complex loop with mixed operations */
    for (i = 0; i < n; i++) {
        /* Memory access - generates mem RTL */
        int val = arr[i];
        
        /* Integer arithmetic with builtin - generates specific RTL pattern */
        int popcnt = __builtin_popcount(val);
        
        /* Floating point operation */
        float fval = (float)val * 1.5f;
        
        /* Conditional move/ternary - may generate cond_exec or if_then_else */
        int cond_val = (val > 100) ? val * 2 : val / 2;
        
        /* Mixed-type computation */
        sum += helper_mul(val, i) + popcnt + cond_val;
        fsum += helper_fmul(fval, (float)i);
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Another conditional with different operations */
        if (i % 3 == 0) {
            sum -= val * 3;
            fsum *= 0.99f;
        } else if (i % 3 == 1) {
            sum += val << 2;
            fsum += 1.5f;
        } else {
            /* Complex expression with multiple operations */
            sum = (sum * 17 + val) / 3;
            fsum = fsum / 2.0f + (float)val;
        }
        
        /* Another memory write */
        arr[i] = sum % 256;
    }
    
    /* Final mixed computation */
    return sum + (int)fsum;
}

/* Function with nested loops for outer loop pipelining */
int __attribute__((noinline, optimize("O3"), target("default")))
test_nested_loops(int size) {
    int total = 0;
    int i, j;
    
    /* Outer loop that should trigger -fsel-sched-pipelining-outer-loops */
    for (i = 0; i < size; i++) {
        int inner_sum = 0;
        
        /* Inner loop with data-dependent computations */
        for (j = 0; j < 100; j++) {
            /* Complex expression with multiple dependencies */
            int a = i * j + global_seed;
            int b = (i << 3) | (j & 0xF);
            
            /* Conditional with arithmetic */
            int c = (a > b) ? a - b : b - a;
            
            /* Use builtin */
            c += __builtin_ffs(c | 1);
            
            /* Floating point in inner loop */
            float fc = (float)c * 0.75f;
            
            inner_sum += (int)fc + c;
            
            /* Periodic scheduling barrier */
            if (j % 7 == 0) {
                asm volatile("" : : : "memory");
            }
        }
        
        total += inner_sum;
        
        /* Branch with different computation paths */
        if (i % 2 == 0) {
            total = total * 3 - 17;
        } else {
            total = (total + 256) / 2;
        }
    }
    
    return total;
}

/* Function with pointer chasing and complex control flow */
int __attribute__((noinline, optimize("O2")))
test_pointer_chasing(int* data, int length) {
    int result = 0;
    int* ptr = data;
    int count = length;
    
    while (count-- > 0) {
        /* Pointer dereference */
        int val = *ptr;
        
        /* Complex bit manipulation */
        int rotated = (val << 4) | (val >> 28);
        
        /* Mixed 32/64 bit operations */
        long long big_val = (long long)val * rotated;
        int hi_part = (int)(big_val >> 32);
        int lo_part = (int)(big_val & 0xFFFFFFFF);
        
        /* Conditional based on multiple factors */
        result += (hi_part > lo_part) ? hi_part - lo_part : lo_part - hi_part;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Update pointer with bounds check */
        if (ptr < data + length - 1) {
            ptr++;
        } else {
            ptr = data;
        }
        
        /* Additional computation that depends on result */
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    const int ARRAY_SIZE = 1024;
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int i, result1, result2, result3;
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Starting selective scheduler test...\n");
    
    /* Call test functions with different characteristics */
    result1 = test_mixed_operations(data, ARRAY_SIZE / 2);
    printf("Test 1 result: %d\n", result1);
    
    result2 = test_nested_loops(ARRAY_SIZE / 16);
    printf("Test 2 result: %d\n", result2);
    
    result3 = test_pointer_chasing(data, ARRAY_SIZE);
    printf("Test 3 result: %d\n", result3);
    
    /* Final computation using all results */
    int final_result = result1 + result2 - result3;
    printf("Final result: %d\n", final_result);
    
    /* Verify with a simple checksum */
    int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= data[i];
    }
    printf("Data checksum: %08x\n", checksum);
    
    free(data);
    
    return (final_result > 0) ? 0 : 1;
}
