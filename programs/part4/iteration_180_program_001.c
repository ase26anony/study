#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

// Aligned allocation for better vectorization
static void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

// GT_EXPR (>)
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using > operator
        // This should trigger GT_EXPR case
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        // Additional operation to prevent optimization
        d[i] = (a[i] > 0) ? c[i] + 1 : c[i] - 1;
    }
}

// GE_EXPR (>=)
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using >= operator
        // This should trigger GE_EXPR case
        c[i] = (a[i] >= b[i]) ? a[i] * 1.5f : b[i] * 0.5f;
        
        // Masked store pattern
        if (a[i] >= 0.0f) {
            d[i] = c[i] * 2.0f;
        } else {
            d[i] = c[i] * 0.5f;
        }
    }
}

// LT_EXPR (<)
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using < operator
        // This should trigger LT_EXPR case
        c[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Reduction-like pattern
        d[i] = (a[i] < 100) ? c[i] * 3 : c[i];
    }
}

// LE_EXPR (<=)
void test_le_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using <= operator
        // This should trigger LE_EXPR case
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        // Complex conditional with multiple uses
        if (a[i] <= 0.0f && b[i] <= 0.0f) {
            d[i] = c[i] * -1.0f;
        } else if (a[i] <= b[i]) {
            d[i] = c[i] * 2.0f;
        } else {
            d[i] = c[i];
        }
    }
}

// Reduction with GT_EXPR
int reduction_gt_expr(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        // Conditional reduction using > operator
        sum += (a[i] > b[i]) ? a[i] : b[i];
    }
    return sum;
}

// Reduction with GE_EXPR
float reduction_ge_expr(float* restrict a, float* restrict b) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        // Conditional reduction using >= operator
        sum += (a[i] >= b[i]) ? a[i] : b[i];
    }
    return sum;
}

// Reduction with LT_EXPR
int reduction_lt_expr(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        // Conditional reduction using < operator
        sum += (a[i] < b[i]) ? a[i] * 2 : b[i] * 3;
    }
    return sum;
}

// Reduction with LE_EXPR
float reduction_le_expr(float* restrict a, float* restrict b) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        // Conditional reduction using <= operator
        sum += (a[i] <= b[i]) ? a[i] * 1.5f : b[i] * 2.5f;
    }
    return sum;
}

// Blend operation with all comparison types
void blend_all_comparisons(int* restrict a, int* restrict b, int* restrict c, 
                           int* restrict d, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        // Use all four comparison operators in a single loop
        // to maximize coverage chances
        int temp = 0;
        
        if (a[i] > b[i]) {
            temp += a[i];
        }
        
        if (a[i] >= b[i] + 10) {
            temp += b[i];
        }
        
        if (a[i] < b[i] * 2) {
            temp += a[i] * b[i];
        }
        
        if (a[i] <= b[i] + 5) {
            temp -= b[i];
        }
        
        out[i] = temp + c[i] + d[i];
    }
}

int main() {
    // Allocate aligned memory for better vectorization
    int* a_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* d_int = aligned_alloc(ALIGN, N * sizeof(int));
    int* out_int = aligned_alloc(ALIGN, N * sizeof(int));
    
    float* a_float = aligned_alloc(ALIGN, N * sizeof(float));
    float* b_float = aligned_alloc(ALIGN, N * sizeof(float));
    float* c_float = aligned_alloc(ALIGN, N * sizeof(float));
    float* d_float = aligned_alloc(ALIGN, N * sizeof(float));
    
    if (!a_int || !b_int || !c_int || !d_int || !out_int ||
        !a_float || !b_float || !c_float || !d_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with varying data to create mix of true/false comparisons
    for (int i = 0; i < N; ++i) {
        // Integer arrays: create pattern that will produce both true and false
        a_int[i] = i;
        b_int[i] = N/2 - i % 100;
        c_int[i] = i * 2;
        d_int[i] = i % 50;
        
        // Float arrays: similar pattern
        a_float[i] = (float)i * 0.5f;
        b_float[i] = (float)(N/4) - (i % 75) * 0.3f;
        c_float[i] = (float)i * 0.25f;
        d_float[i] = (float)(i % 60) * 0.1f;
    }
    
    // Call all test functions to exercise different comparison operators
    test_gt_expr(a_int, b_int, c_int, d_int);
    test_ge_expr(a_float, b_float, c_float, d_float);
    test_lt_expr(a_int, b_int, c_int, d_int);
    test_le_expr(a_float, b_float, c_float, d_float);
    
    // Call reduction functions
    int sum_gt = reduction_gt_expr(a_int, b_int);
    float sum_ge = reduction_ge_expr(a_float, b_float);
    int sum_lt = reduction_lt_expr(a_int, b_int);
    float sum_le = reduction_le_expr(a_float, b_float);
    
    // Call blend function that uses all comparison types
    blend_all_comparisons(a_int, b_int, c_int, d_int, out_int);
    
    // Compute checksums to ensure computations aren't optimized away
    int int_checksum = 0;
    float float_checksum = 0.0f;
    
    for (int i = 0; i < N; ++i) {
        int_checksum += c_int[i] + d_int[i] + out_int[i];
        float_checksum += c_float[i] + d_float[i];
    }
    
    // Add reduction results to checksums
    int_checksum += sum_gt + sum_lt;
    float_checksum += sum_ge + sum_le;
    
    printf("Integer checksum: %d\n", int_checksum);
    printf("Float checksum: %f\n", float_checksum);
    
    // Free allocated memory
    free(a_int);
    free(b_int);
    free(c_int);
    free(d_int);
    free(out_int);
    free(a_float);
    free(b_float);
    free(c_float);
    free(d_float);
    
    return 0;
}
