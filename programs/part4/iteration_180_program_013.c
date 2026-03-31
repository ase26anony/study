#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGN 32

/* Aligned allocation for better vectorization */
static void* aligned_alloc(size_t alignment, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

/* Test GT_EXPR (>) transformation */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using > operator */
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        /* Additional masked operation to encourage bitop transformation */
        if (a[i] > b[i]) {
            d[i] = a[i] - b[i];
        } else {
            d[i] = b[i] - a[i];
        }
    }
}

/* Test GE_EXPR (>=) transformation */
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    float threshold = 0.5f;
    for (int i = 0; i < N; ++i) {
        /* Conditional increment pattern */
        if (a[i] >= threshold) {
            c[i] = b[i] * 2.0f;
        } else {
            c[i] = b[i] * 0.5f;
        }
        
        /* Blend operation using >= */
        d[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* Test LT_EXPR (<) transformation */
void test_lt_expr(int16_t* restrict a, int16_t* restrict b, int16_t* restrict c, int16_t* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional store with < operator */
        if (a[i] < b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
        
        /* Reduction-like pattern */
        d[i] = (a[i] < 0) ? -a[i] : a[i];
    }
}

/* Test LE_EXPR (<=) transformation */
void test_le_expr(double* restrict a, double* restrict b, double* restrict c, double* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Multiple uses of <= operator */
        double temp = (a[i] <= b[i]) ? a[i] : b[i];
        c[i] = temp * 3.0;
        
        /* Conditional with <= in arithmetic */
        if (a[i] <= 0.0) {
            d[i] = -a[i] * b[i];
        } else {
            d[i] = a[i] * b[i];
        }
    }
}

/* Mixed test with all comparison types in one loop */
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict out) {
    for (int i = 0; i < N; ++i) {
        int val = 0;
        
        /* Chain of comparisons to potentially trigger different transformations */
        if (a[i] > b[i]) {
            val += 10;
        }
        if (a[i] >= b[i] + 5) {
            val += 20;
        }
        if (a[i] < b[i] - 3) {
            val += 30;
        }
        if (a[i] <= b[i] * 2) {
            val += 40;
        }
        
        out[i] = val;
    }
}

/* Initialize arrays with varying patterns to ensure mix of true/false conditions */
void init_arrays() {
    /* Arrays will be allocated in main */
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* out_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* a_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* b_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* c_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* d_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    int16_t* a_int16 = (int16_t*)aligned_alloc(ALIGN, N * sizeof(int16_t));
    int16_t* b_int16 = (int16_t*)aligned_alloc(ALIGN, N * sizeof(int16_t));
    int16_t* c_int16 = (int16_t*)aligned_alloc(ALIGN, N * sizeof(int16_t));
    int16_t* d_int16 = (int16_t*)aligned_alloc(ALIGN, N * sizeof(int16_t));
    
    double* a_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* b_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* c_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    double* d_double = (double*)aligned_alloc(ALIGN, N * sizeof(double));
    
    if (!a_int || !b_int || !c_int || !d_int || !out_int ||
        !a_float || !b_float || !c_float || !d_float ||
        !a_int16 || !b_int16 || !c_int16 || !d_int16 ||
        !a_double || !b_double || !c_double || !d_double) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying patterns to ensure all comparison paths are taken */
    for (int i = 0; i < N; ++i) {
        /* Integer arrays: create mix of >, <, = conditions */
        a_int[i] = i;
        b_int[i] = N/2;
        
        /* Float arrays: values crossing threshold */
        a_float[i] = (i % 10) / 10.0f;
        b_float[i] = 0.5f;
        
        /* int16 arrays: signed values for < 0 comparisons */
        a_int16[i] = (int16_t)(i - N/2);
        b_int16[i] = (int16_t)(i % 100);
        
        /* Double arrays: positive and negative values */
        a_double[i] = (i % 20) - 10.0;
        b_double[i] = (i % 15) - 7.0;
    }
    
    /* Execute all tests to trigger different comparison transformations */
    test_gt_expr(a_int, b_int, c_int, d_int);
    test_ge_expr(a_float, b_float, c_float, d_float);
    test_lt_expr(a_int16, b_int16, c_int16, d_int16);
    test_le_expr(a_double, b_double, c_double, d_double);
    test_mixed_comparisons(a_int, b_int, out_int);
    
    /* Compute checksums to ensure computations aren't optimized away */
    uint64_t checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c_int[i];
        checksum += (uint64_t)(c_float[i] * 1000);
        checksum += c_int16[i];
        checksum += (uint64_t)(c_double[i] * 1000);
        checksum += out_int[i];
    }
    
    printf("Checksum: %lu\n", checksum);
    
    /* Free allocated memory */
    free(a_int); free(b_int); free(c_int); free(d_int); free(out_int);
    free(a_float); free(b_float); free(c_float); free(d_float);
    free(a_int16); free(b_int16); free(c_int16); free(d_int16);
    free(a_double); free(b_double); free(c_double); free(d_double);
    
    return 0;
}
