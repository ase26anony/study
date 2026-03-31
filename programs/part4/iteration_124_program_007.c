/* Test program to trigger selective scheduler debug dumping in GCC */
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

/* Complex function with mixed operations to generate diverse RTL */
__attribute__((noinline, optimize("O3")))
int test_function_1(int* arr, int n, int seed) {
    volatile int barrier;  /* Prevent optimization */
    int sum = seed;
    float fsum = seed * 0.5f;
    
    /* Mixed integer and FP operations */
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing mode */
        int val = arr[i];
        
        /* Data-dependent computation with conditional */
        if (val > 0) {
            /* Integer multiplication */
            sum += val * i;
            
            /* Built-in function for complex RTL */
            sum += __builtin_popcount(val);
            
            /* Ternary operator for conditional move pattern */
            int tmp = (i % 3 == 0) ? val : (val >> 1);
            sum += tmp;
            
            /* Function call */
            sum += helper_mul(val, i);
        } else {
            /* Different computation path */
            sum -= (-val) * i;
            
            /* Floating point in integer loop */
            fsum += (float)val * 0.1f;
            fsum = helper_fmul(fsum, 1.01f);
        }
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Complex addressing with multiple uses */
        if (i > 0) {
            arr[i] = (arr[i-1] + val) * 2;
        }
        
        /* Another barrier to create scheduling regions */
        barrier = i;
    }
    
    /* Final mixing */
    return sum + (int)fsum;
}

/* Function with nested loops for outer loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_function_2(int* matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* 2D array access */
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Complex computation with multiple dependencies */
            int prod = val * (i + j);
            row_sum += prod;
            
            /* Conditional with arithmetic */
            if ((i ^ j) & 1) {
                row_sum -= (val >> 2);
            } else {
                row_sum += (val << 1);
            }
            
            /* Use different data types */
            float fval = (float)val;
            if (fval > 100.0f) {
                row_sum += 5;
            }
        }
        
        /* Reduce with barrier */
        asm volatile("" : : : "memory");
        total += row_sum;
        
        /* Modify matrix for next iteration */
        if (i < rows - 1) {
            matrix[(i + 1) * cols] += row_sum % 256;
        }
    }
    
    return total;
}

/* Function with pointer chasing and unpredictable branches */
__attribute__((noinline, optimize("O3")))
int test_function_3(int* data, int size) {
    int result = 0;
    int* ptr = data;
    int count = 0;
    
    while (count < size) {
        /* Unpredictable memory access pattern */
        int val = *ptr;
        
        /* Complex chain of operations */
        int transformed = ((val * 1103515245) + 12345) & 0x7fffffff;
        
        /* Conditional based on computation */
        if (transformed % 7 < 3) {
            result += transformed;
            ptr += 2;  /* Skip pattern */
        } else {
            result -= transformed >> 4;
            ptr += 1;
        }
        
        /* Barrier to prevent fusion */
        asm volatile("" : : : "memory");
        
        /* Bounds check */
        if (ptr >= data + size) {
            ptr = data;
        }
        
        count++;
        
        /* Mix in some floating point */
        float ftemp = (float)result * 0.01f;
        result += (int)ftemp;
    }
    
    return result;
}

/* Function using SIMD-like operations manually */
__attribute__((noinline, optimize("O3"), target("arch=haswell")))
int test_function_4(short* vec_a, short* vec_b, int len) {
    int sum = 0;
    
    for (int i = 0; i < len; i += 4) {
        /* Manual SIMD-like operations */
        int32_t a0 = vec_a[i];
        int32_t a1 = vec_a[i + 1];
        int32_t a2 = vec_a[i + 2];
        int32_t a3 = vec_a[i + 3];
        
        int32_t b0 = vec_b[i];
        int32_t b1 = vec_b[i + 1];
        int32_t b2 = vec_b[i + 2];
        int32_t b3 = vec_b[i + 3];
        
        /* Multiple independent multiplies */
        int32_t p0 = a0 * b0;
        int32_t p1 = a1 * b1;
        int32_t p2 = a2 * b2;
        int32_t p3 = a3 * b3;
        
        /* Horizontal reduction with barriers */
        asm volatile("" : : : "memory");
        sum += p0 + p1;
        asm volatile("" : : : "memory");
        sum += p2 + p3;
        
        /* Cross-element dependencies */
        vec_a[i] = (short)(sum & 0xFFFF);
        vec_b[i] = (short)((sum >> 16) & 0xFFFF);
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    /* Initialize test data */
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 64;
    const int MATRIX_COLS = 16;
    const int VEC_LEN = 256;
    
    int* array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* matrix = (int*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    int* array3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    short* vec_a = (short*)malloc(VEC_LEN * sizeof(short));
    short* vec_b = (short*)malloc(VEC_LEN * sizeof(short));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7FFF;
        array3[i] = (i * 1664525 + 1013904223) & 0x7FFF;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    for (int i = 0; i < VEC_LEN; i++) {
        vec_a[i] = (short)((i * 1103515245 + 12345) & 0x7FFF);
        vec_b[i] = (short)((i * 1664525 + 1013904223) & 0x7FFF);
    }
    
    /* Call test functions with volatile to prevent dead code elimination */
    volatile int result1 = test_function_1(array1, ARRAY_SIZE, 42);
    volatile int result2 = test_function_2(matrix, MATRIX_ROWS, MATRIX_COLS);
    volatile int result3 = test_function_3(array3, ARRAY_SIZE);
    volatile int result4 = test_function_4(vec_a, vec_b, VEC_LEN);
    
    /* Combine results */
    int total = result1 + result2 + result3 + result4;
    
    printf("Test results: %d, %d, %d, %d\n", result1, result2, result3, result4);
    printf("Total: %d\n", total);
    
    /* Cleanup */
    free(array1);
    free(matrix);
    free(array3);
    free(vec_a);
    free(vec_b);
    
    return total != 0 ? 0 : 1;
}
