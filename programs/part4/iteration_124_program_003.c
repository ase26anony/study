/* Test for selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>

/* Helper functions to prevent optimization */
static volatile int sink;

/* Non-inline function to generate call RTL */
__attribute__((noinline, optimize("O2")))
int helper_compute(int x, int y) {
    return x * y + (x ^ y);
}

/* Complex loop with mixed operations */
__attribute__((noinline, optimize("O2")))
int test_selective_scheduling_1(int* array, int n) {
    int sum = 0;
    int prod = 1;
    float fsum = 0.0f;
    
    /* Mixed integer and FP operations */
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing mode */
        int val = array[i];
        
        /* Data-dependent computation */
        sum += val * i;
        prod *= (val & 0xFF) + 1;
        
        /* Floating point operation */
        fsum += (float)val * 0.5f;
        
        /* Conditional move/ternary */
        int cond_val = (val > 100) ? val : 100;
        sum += cond_val;
        
        /* Builtin function */
        sum += __builtin_popcount(val);
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Another memory write */
        array[i] = val + i;
    }
    
    /* Use results to prevent dead code elimination */
    sink = sum + prod + (int)fsum;
    return sum;
}

/* Test with nested loops and branches */
__attribute__((noinline, optimize("O2")))
int test_selective_scheduling_2(int* data, int size) {
    int total = 0;
    
    for (int i = 0; i < size; i++) {
        /* Create multiple basic blocks */
        if (i % 2 == 0) {
            /* Even path */
            int x = data[i];
            int y = data[size - i - 1];
            
            /* Complex expression */
            total += (x * y) >> (i & 3);
            total ^= helper_compute(x, y);
            
            /* More FP ops */
            float fx = (float)x;
            float fy = (float)y;
            total += (int)(fx * fy);
        } else {
            /* Odd path - different operations */
            int val = data[i];
            
            /* Bit manipulation */
            total += __builtin_ctz(val | 1);
            total += __builtin_clz(val);
            
            /* Division creates complex RTL */
            total += val / (i + 1);
            
            /* Modulo operation */
            total += val % 7;
        }
        
        /* Unrolled inner loop */
        for (int j = 0; j < 3; j++) {
            total += data[(i + j) % size] * j;
        }
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Test with 64-bit operations */
__attribute__((noinline, optimize("O2")))
long long test_selective_scheduling_3(long long* arr, int n) {
    long long result = 0;
    long long accum = 1;
    
    for (int i = 0; i < n; i++) {
        /* Mix of 32-bit and 64-bit operations */
        int idx = i & (n - 1);
        long long val = arr[idx];
        
        /* 64-bit multiplication */
        result += val * i;
        
        /* 64-bit shift */
        accum <<= (val & 7);
        
        /* Conditional with 64-bit values */
        long long max_val = (val > result) ? val : result;
        result = max_val;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
        
        /* Write back */
        arr[idx] = val + accum;
    }
    
    return result + accum;
}

/* Main test driver */
int main() {
    const int SIZE = 256;
    int* array1 = (int*)malloc(SIZE * sizeof(int));
    long long* array2 = (long long*)malloc(SIZE * sizeof(long long));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 13 + 7) & 0xFFF;
        array2[i] = (long long)i * i * 3;
    }
    
    /* Run all tests */
    int result1 = test_selective_scheduling_1(array1, SIZE);
    int result2 = test_selective_scheduling_2(array1, SIZE);
    long long result3 = test_selective_scheduling_3(array2, SIZE);
    
    /* Use results to prevent optimization */
    printf("Test Results:\n");
    printf("  Test 1: %d\n", result1);
    printf("  Test 2: %d\n", result2);
    printf("  Test 3: %lld\n", result3);
    
    /* Final computation using all results */
    int final_result = result1 + result2 + (int)result3;
    printf("Final checksum: %d\n", final_result);
    
    free(array1);
    free(array2);
    
    return (final_result > 0) ? 0 : 1;
}
