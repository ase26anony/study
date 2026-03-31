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

/* Complex loop with mixed operations - Test 1 */
__attribute__((noinline, optimize("O3")))
int test_selective_sched_loop(int* arr, int n) {
    volatile int barrier; /* Prevent optimization */
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with addressing computation */
        int val = arr[i];
        
        /* Mixed integer operations */
        int prod = helper_mul(val, i);
        int shifted = (prod << 3) | (prod >> 5);
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : shifted;
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(cond_val);
        
        /* Floating point operations */
        float fval = (float)val;
        float fprod = helper_fmul(fval, fval * 0.5f);
        
        /* Branch with different computation paths */
        if (i % 4 == 0) {
            sum += popcnt * 2;
            fsum += fprod;
        } else if (i % 4 == 1) {
            sum += cond_val ^ 0x55AA;
            fsum -= fprod;
        } else if (i % 4 == 2) {
            sum += __builtin_clz(cond_val);
            fsum *= 1.01f;
        } else {
            sum += (cond_val * i) >> 2;
            fsum = fsum / 1.5f;
        }
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Another memory write */
        arr[i] = cond_val + (int)fsum;
    }
    
    barrier = sum;
    return sum + (int)fsum;
}

/* Test with nested loops - Test 2 */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int* mat, int rows, int cols) {
    int total = 0;
    
    for (int r = 0; r < rows; r++) {
        int row_sum = 0;
        for (int c = 0; c < cols; c++) {
            /* 2D array access */
            int idx = r * cols + c;
            int val = mat[idx];
            
            /* Complex computation with division (creates different RTL) */
            int scaled = (val * 7) / 3;
            
            /* Bit manipulation */
            int masked = scaled & 0xFF00FF;
            int swapped = ((masked >> 16) & 0xFFFF) | ((masked & 0xFFFF) << 16);
            
            /* Conditional based on multiple variables */
            int result = (r > c) ? swapped : (swapped ^ 0x12345678);
            
            row_sum += result;
            
            /* Memory write with conditional */
            mat[idx] = (result > 0) ? result : -result;
        }
        
        /* Outer loop computation */
        total += row_sum * r;
        
        /* Scheduling barrier in outer loop */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

/* Test with pointer chasing - Test 3 */
__attribute__((noinline, optimize("O3")))
int test_pointer_chasing(int* data, int size, int stride) {
    int sum = 0;
    int* ptr = data;
    
    for (int i = 0; i < size; i++) {
        /* Pointer arithmetic and dereference */
        int val = *ptr;
        
        /* Complex addressing mode potential */
        int next_idx = (val + i) % size;
        
        /* Mix of operations */
        int transformed = ((val * 3) + 7) ^ 0xDEADBEEF;
        int rotated = (transformed << 4) | (transformed >> 28);
        
        /* Use builtin for parity */
        int parity = __builtin_parity(rotated);
        
        sum += rotated * (1 - 2 * parity); /* Multiply by ±1 */
        
        /* Update pointer with stride */
        ptr = data + ((i * stride) % size);
        
        /* Barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return sum;
}

/* Test with floating point array - Test 4 */
__attribute__((noinline, optimize("O3")))
float test_fp_array(float* arr, int n) {
    float sum = 0.0f;
    float prod = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Load and FP operation */
        float val = arr[i];
        
        /* Multiple FP operations in dependency chain */
        float squared = val * val;
        float cubed = squared * val;
        float inv = 1.0f / (val + 1.0f);
        
        /* Conditional FP operation */
        float selected = (val > 0.5f) ? squared : cubed;
        
        /* FP multiply-add pattern */
        sum = sum + selected * inv;
        prod = prod * (selected + 1.0f);
        
        /* Store result */
        arr[i] = selected;
        
        /* Integer computation mixed in */
        int int_val = (int)(val * 1000.0f);
        sum += (float)(int_val % 100);
        
        /* Barrier */
        if (i % 16 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    return sum / prod;
}

/* Main driver */
int main() {
    const int SIZE = 256;
    const int MAT_ROWS = 16;
    const int MAT_COLS = 16;
    
    /* Allocate and initialize test data */
    int* data1 = (int*)malloc(SIZE * sizeof(int));
    int* data2 = (int*)malloc(MAT_ROWS * MAT_COLS * sizeof(int));
    int* data3 = (int*)malloc(SIZE * sizeof(int));
    float* data4 = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (i * 37 + 123) % 1000;
        data3[i] = (i * 51 + 456) % 1000;
        data4[i] = (float)((i * 73 + 789) % 1000) / 1000.0f;
    }
    
    for (int i = 0; i < MAT_ROWS * MAT_COLS; i++) {
        data2[i] = (i * 29 + 321) % 1000;
    }
    
    /* Run all tests */
    int result1 = test_selective_sched_loop(data1, SIZE);
    int result2 = test_nested_loops(data2, MAT_ROWS, MAT_COLS);
    int result3 = test_pointer_chasing(data3, SIZE, 7);
    float result4 = test_fp_array(data4, SIZE);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + (int)result4;
    
    printf("Test Results:\n");
    printf("  Test 1: %d\n", result1);
    printf("  Test 2: %d\n", result2);
    printf("  Test 3: %d\n", result3);
    printf("  Test 4: %f\n", result4);
    printf("  Final: %d\n", final_result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    
    return (final_result != 0) ? 0 : 1;
}
