#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static void* aligned_alloc(size_t align, size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, align, size) != 0) return NULL;
    return ptr;
}

/* GT_EXPR (>) test */
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using > operator */
        c[i] = (a[i] > b[i]) ? a[i] * 2 : b[i] / 2;
        
        /* Masked store pattern */
        if (a[i] > b[i]) {
            d[i] = a[i] + b[i];
        } else {
            d[i] = a[i] - b[i];
        }
    }
}

/* GE_EXPR (>=) test */
void test_ge_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        /* Conditional increment using >= operator */
        sum += (a[i] >= b[i]) ? a[i] : 0;
        
        /* Blend pattern */
        c[i] = (a[i] >= b[i]) ? a[i] : b[i];
        
        /* Another masked operation */
        d[i] = (a[i] >= b[i]) ? a[i] * b[i] : a[i] + b[i];
    }
    
    /* Use sum to prevent dead code elimination */
    c[0] += sum % 100;
}

/* LT_EXPR (<) test */
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Conditional assignment using < operator */
        c[i] = (a[i] < b[i]) ? a[i] * 3 : b[i] * 2;
        
        /* Complex masked expression */
        if (a[i] < b[i]) {
            d[i] = (a[i] << 2) | (b[i] & 0xFF);
        } else {
            d[i] = (b[i] << 2) | (a[i] & 0xFF);
        }
    }
}

/* LE_EXPR (<=) test */
void test_le_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    int prod = 1;
    for (int i = 0; i < N; ++i) {
        /* Conditional operation using <= operator */
        c[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        
        /* Masked reduction */
        if (a[i] <= b[i]) {
            prod *= (c[i] + 1) & 0x3F; /* Mod to prevent overflow */
        }
        
        /* Another conditional pattern */
        d[i] = (a[i] <= b[i]) ? (a[i] | b[i]) : (a[i] & b[i]);
    }
    
    /* Use prod to prevent dead code elimination */
    d[0] += prod & 0xFF;
}

/* Floating point versions to test different data types */
void test_float_gt_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Floating point comparison with > */
        c[i] = (a[i] > b[i]) ? a[i] * 1.5f : b[i] * 0.5f;
    }
}

void test_float_le_expr(float* restrict a, float* restrict b, float* restrict c) {
    for (int i = 0; i < N; ++i) {
        /* Floating point comparison with <= */
        if (a[i] <= b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Mixed type comparisons */
void test_mixed_comparisons(int* restrict a, int* restrict b, 
                           short* restrict c, char* restrict d) {
    for (int i = 0; i < N; ++i) {
        /* Multiple comparisons in one loop */
        c[i] = (a[i] > b[i]) ? (short)(a[i] >> 1) : (short)b[i];
        d[i] = (a[i] <= b[i]) ? (char)(a[i] & 0xFF) : (char)(b[i] & 0xFF);
    }
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* d_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    float* a_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* b_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    float* c_float = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    short* c_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    char* d_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    if (!a_int || !b_int || !c_int || !d_int || 
        !a_float || !b_float || !c_float ||
        !c_short || !d_char) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying patterns to ensure mix of true/false comparisons */
    for (int i = 0; i < N; ++i) {
        a_int[i] = i;
        b_int[i] = N/2 - i % 100;  /* Creates varying comparison results */
        c_int[i] = 0;
        d_int[i] = 0;
        
        a_float[i] = (float)i * 1.5f;
        b_float[i] = (float)(N/2 - i % 50) * 0.8f;
        c_float[i] = 0.0f;
        
        c_short[i] = 0;
        d_char[i] = 0;
    }
    
    /* Execute all test functions */
    test_gt_expr(a_int, b_int, c_int, d_int);
    test_ge_expr(a_int, b_int, c_int, d_int);
    test_lt_expr(a_int, b_int, c_int, d_int);
    test_le_expr(a_int, b_int, c_int, d_int);
    
    test_float_gt_expr(a_float, b_float, c_float);
    test_float_le_expr(a_float, b_float, c_float);
    
    test_mixed_comparisons(a_int, b_int, c_short, d_char);
    
    /* Compute checksums to ensure computations aren't optimized away */
    int int_checksum = 0;
    float float_checksum = 0.0f;
    short short_checksum = 0;
    char char_checksum = 0;
    
    for (int i = 0; i < N; ++i) {
        int_checksum += c_int[i] + d_int[i];
        float_checksum += c_float[i];
        short_checksum += c_short[i];
        char_checksum += d_char[i];
    }
    
    printf("Checksums:\n");
    printf("  Integer: %d\n", int_checksum);
    printf("  Float: %.2f\n", float_checksum);
    printf("  Short: %d\n", (int)short_checksum);
    printf("  Char: %d\n", (int)char_checksum);
    
    /* Verify results are non-zero (ensures computations happened) */
    if (int_checksum == 0 && float_checksum == 0.0f && 
        short_checksum == 0 && char_checksum == 0) {
        printf("WARNING: All checksums are zero - computations may have been optimized away\n");
    }
    
    /* Cleanup */
    free(a_int); free(b_int); free(c_int); free(d_int);
    free(a_float); free(b_float); free(c_float);
    free(c_short); free(d_char);
    
    return 0;
}
