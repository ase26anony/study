/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Function with complex loop body to create ILP opportunities */
__attribute__((noinline, optimize("O2")))
int test_loop_ilp(int* arr, int n) {
    int sum_int = 0;
    float sum_float = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer operations */
        int a = arr[i];
        int b = i * 2;
        int c = (a > b) ? a : b;  /* Conditional move pattern */
        
        /* Memory access pattern */
        arr[i] = c + i;
        
        /* Integer computation with builtin */
        int popcnt = __builtin_popcount(a);
        
        /* Floating point operations */
        float x = (float)a;
        float y = (float)b;
        float z = x * y;
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Accumulate results */
        sum_int += c + popcnt;
        sum_float += z;
        
        /* Another barrier */
        barrier = i;
        
        /* Call non-inline functions */
        sum_int += helper_mul(a, i);
        sum_float += helper_fmul(x, y);
        
        /* Complex conditional with side effects */
        if (i % 3 == 0) {
            sum_int += arr[i] * 2;
            asm volatile ("" : : : "memory");
        } else if (i % 3 == 1) {
            sum_float *= 1.01f;
        } else {
            /* Use builtin for complex RTL */
            sum_int += __builtin_ctz(i | 1);
        }
    }
    
    /* Mix results to prevent dead code elimination */
    return sum_int + (int)sum_float + barrier;
}

/* Function with nested loops for outer loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* mat, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        float row_float = 0.0f;
        
        for (int j = 0; j < cols; j++) {
            /* Data-dependent computation */
            int idx = i * cols + j;
            int val = mat[idx];
            
            /* Mixed operations */
            int scaled = val * (i + j);
            float fscaled = (float)scaled * 0.5f;
            
            /* Conditional with both paths used */
            if ((i + j) % 2) {
                row_sum += scaled;
                /* Generate mem RTL */
                mat[idx] = scaled % 256;
            } else {
                row_float += fscaled;
                /* Another memory access pattern */
                mat[idx] = (int)fscaled;
            }
            
            /* Use ternary operator for cond_exec RTL */
            int temp = (val > 100) ? val * 2 : val / 2;
            row_sum += temp;
            
            /* Scheduling barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Reduce results */
        total += row_sum + (int)row_float;
        
        /* Outer loop barrier */
        if (i % 4 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return total;
}

/* Function with pointer chasing to create complex dependencies */
__attribute__((noinline, target("arch=core2")))
int test_pointer_chasing(int* data, int size) {
    int result = 0;
    int* ptr = data;
    
    for (int i = 0; i < size; i++) {
        /* Pointer arithmetic and dereference */
        int val = *ptr;
        
        /* Complex integer computation */
        int rotated = (val << 3) | (val >> 29);  /* Rotate left by 3 */
        int masked = rotated & 0x7FFFFFFF;
        
        /* Floating point conversion and computation */
        double dval = (double)masked;
        dval = dval * 1.234567;
        
        /* Store result */
        *ptr = (int)dval;
        
        /* Update pointer with wrap-around */
        ptr = data + ((ptr - data + 1) % size);
        
        /* Accumulate with mixed types */
        result += masked + (int)dval;
        
        /* Conditional branch with both hot paths */
        if (val % 7 < 3) {
            result += __builtin_popcount(val);
            asm volatile ("" : : : "memory");
        } else {
            result -= __builtin_ctz(val | 1);
        }
        
        /* Another scheduling region */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Function using 64-bit operations for different machine modes */
__attribute__((noinline))
int64_t test_64bit_ops(int64_t* arr, int n) {
    int64_t sum = 0;
    volatile int64_t barrier = 0;
    
    for (int i = 0; i < n; i++) {
        /* 64-bit operations */
        int64_t a = arr[i];
        int64_t b = (int64_t)i * 1000000LL;
        
        /* 64-bit conditional move */
        int64_t c = (a > b) ? a : b;
        
        /* Mixed 32/64 bit operations */
        int32_t low = (int32_t)(c & 0xFFFFFFFF);
        int32_t high = (int32_t)(c >> 32);
        
        /* Complex 64-bit computation */
        int64_t product = (int64_t)low * (int64_t)high;
        int64_t shifted = product << (i % 16);
        
        /* Memory store */
        arr[i] = shifted;
        
        /* Accumulate */
        sum += c + shifted;
        barrier = i;
        
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Use builtin for 64-bit popcount */
        sum += __builtin_popcountll(c);
    }
    
    return sum + barrier;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    const int SIZE = 256;
    const int ROWS = 16;
    const int COLS = 16;
    
    /* Allocate and initialize test data */
    int* data1 = (int*)malloc(SIZE * sizeof(int));
    int* data2 = (int*)malloc(ROWS * COLS * sizeof(int));
    int64_t* data3 = (int64_t*)malloc(SIZE * sizeof(int64_t));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (i * 37 + 123) % 1000;
        data3[i] = (int64_t)data1[i] * 1000000LL;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        data2[i] = (i * 73 + 456) % 500;
    }
    
    /* Call test functions with different patterns */
    int result1 = test_loop_ilp(data1, SIZE);
    int result2 = test_nested_loops(data2, ROWS, COLS);
    int result3 = test_pointer_chasing(data1, SIZE);
    int64_t result4 = test_64bit_ops(data3, SIZE);
    
    /* Combine results to ensure all code is live */
    int final_result = result1 + result2 + result3 + (int)result4;
    
    printf("Test results:\n");
    printf("  Loop ILP: %d\n", result1);
    printf("  Nested loops: %d\n", result2);
    printf("  Pointer chasing: %d\n", result3);
    printf("  64-bit ops: %lld\n", (long long)result4);
    printf("  Final combined: %d\n", final_result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return (final_result > 0) ? 0 : 1;
}
