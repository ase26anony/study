#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Simple deterministic RNG for reproducible results */
static uint32_t seed = 123456789;
static uint32_t rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

static float rand_float(void) {
    return (float)rand_int() / (float)UINT32_MAX;
}

static double rand_double(void) {
    return (double)rand_int() / (double)UINT32_MAX;
}

/* Initialize arrays with deterministic values */
static void init_arrays(void) {
    for (int i = 0; i < N; i++) {
        a_int8[i] = (int8_t)(rand_int() % 256 - 128);
        b_int8[i] = (int8_t)(rand_int() % 256 - 128);
        a_int16[i] = (int16_t)(rand_int() % 65536 - 32768);
        b_int16[i] = (int16_t)(rand_int() % 65536 - 32768);
        a_int32[i] = (int32_t)rand_int();
        b_int32[i] = (int32_t)rand_int();
        a_int64[i] = (int64_t)rand_int() * rand_int();
        b_int64[i] = (int64_t)rand_int() * rand_int();
        a_float[i] = rand_float() * 1000.0f - 500.0f;
        b_float[i] = rand_float() * 1000.0f - 500.0f;
        a_double[i] = rand_double() * 1000.0 - 500.0;
        b_double[i] = rand_double() * 1000.0 - 500.0;
        
        c_int8[i] = (int8_t)(rand_int() % 256 - 128);
        d_int8[i] = (int8_t)(rand_int() % 256 - 128);
        c_float[i] = rand_float() * 1000.0f - 500.0f;
        d_float[i] = rand_float() * 1000.0f - 500.0f;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr2d_a[i][j] = rand_float() * 100.0f - 50.0f;
            arr2d_b[i][j] = rand_float() * 100.0f - 50.0f;
        }
    }
}

/* Global arrays for testing */
static int8_t a_int8[N], b_int8[N], c_int8[N], d_int8[N], out_int8[N];
static int16_t a_int16[N], b_int16[N], out_int16[N];
static int32_t a_int32[N], b_int32[N], out_int32[N];
static int64_t a_int64[N], b_int64[N], out_int64[N];
static float a_float[N], b_float[N], c_float[N], d_float[N], out_float[N];
static double a_double[N], b_double[N], out_double[N];
static float arr2d_a[N][M], arr2d_b[N][M], arr2d_out[N][M];

/* GT_EXPR variants with __attribute__((optimize)) */
__attribute__((optimize("O3", "tree-vectorize")))
static void test_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                         int8_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void test_gt_float(float *restrict a, float *restrict b,
                          float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void test_gt_double(double *restrict a, double *restrict b,
                           double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            out[i] = a[i] * 2.0;
        } else {
            out[i] = b[i] * 0.5;
        }
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void test_ge_int16(int16_t *restrict a, int16_t *restrict b,
                          int16_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void test_ge_int32(int32_t *restrict a, int32_t *restrict b,
                          int32_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void test_ge_float_conditional(float *restrict a, float *restrict b,
                                      float *restrict c, float *restrict d,
                                      float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? c[i] : d[i];
    }
}

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void test_lt_int64(int64_t *restrict a, int64_t *restrict b,
                          int64_t *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            /* This should trigger std::swap(cond_expr0, cond_expr1) */
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        } else {
            /* Swapped operands */
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void test_lt_float_nested(float *restrict a, float *restrict b,
                                 float *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            if (a[i] < b[i]) {
                out[i] = a[i] * b[i];
            } else {
                out[i] = a[i] / (b[i] + 1.0f);
            }
        } else {
            /* Swapped comparison */
            if (b[i] < a[i]) {
                out[i] = b[i] * a[i];
            } else {
                out[i] = b[i] / (a[i] + 1.0f);
            }
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void test_le_int32(int32_t *restrict a, int32_t *restrict b,
                          int32_t *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            out[i] = (a[i] <= b[i]) ? a[i] + i : b[i] - i;
        } else {
            /* Swapped operands to trigger std::swap */
            out[i] = (b[i] <= a[i]) ? b[i] + i : a[i] - i;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void test_le_double(double *restrict a, double *restrict b,
                           double *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        double result;
        if (mode) {
            result = (a[i] <= b[i]) ? a[i] * 3.0 : b[i] / 3.0;
        } else {
            /* Swapped comparison */
            result = (b[i] <= a[i]) ? b[i] * 3.0 : a[i] / 3.0;
        }
        out[i] = result;
    }
}

/* Multi-dimensional array test with GT_EXPR */
__attribute__((optimize("O3", "tree-vectorize")))
static void test_2d_gt_float(float a[][M], float b[][M], float out[][M], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

/* Mixed comparisons in single loop */
__attribute__((optimize("O3", "tree-vectorize")))
static void test_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                   int32_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            out[i] = a[i] * 2;
        } else if (a[i] >= b[i]) {
            out[i] = a[i] + b[i];
        } else if (a[i] < b[i]) {
            out[i] = b[i] - a[i];
        } else if (a[i] <= b[i]) {
            out[i] = a[i] * b[i];
        } else {
            out[i] = 0;
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void test_strided_lt_float(float *restrict a, float *restrict b,
                                  float *restrict out, int n) {
    for (int i = 0; i < n; i += 4) {
        out[i] = (a[i] < b[i]) ? a[i] : b[i];
        out[i+1] = (a[i+1] < b[i+1]) ? a[i+1] : b[i+1];
        out[i+2] = (a[i+2] < b[i+2]) ? a[i+2] : b[i+2];
        out[i+3] = (a[i+3] < b[i+3]) ? a[i+3] : b[i+3];
    }
}

int main(void) {
    init_arrays();
    
    int64_t checksum = 0;
    
    /* Test GT_EXPR variants */
    test_gt_int8(a_int8, b_int8, out_int8, N);
    test_gt_float(a_float, b_float, out_float, N);
    test_gt_double(a_double, b_double, out_double, N);
    
    /* Test GE_EXPR variants */
    test_ge_int16(a_int16, b_int16, out_int16, N);
    test_ge_int32(a_int32, b_int32, out_int32, N);
    test_ge_float_conditional(a_float, b_float, c_float, d_float, out_float, N);
    
    /* Test LT_EXPR and LE_EXPR with mode switching to trigger swapped operands */
    for (int mode = 0; mode <= 1; mode++) {
        test_lt_int64(a_int64, b_int64, out_int64, mode);
        test_lt_float_nested(a_float, b_float, out_float, mode);
        test_le_int32(a_int32, b_int32, out_int32, mode);
        test_le_double(a_double, b_double, out_double, mode);
    }
    
    /* Test multi-dimensional arrays */
    test_2d_gt_float(arr2d_a, arr2d_b, arr2d_out, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a_int32, b_int32, out_int32, N);
    
    /* Test strided access */
    test_strided_lt_float(a_float, b_float, out_float, N);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out_int8[i] + out_int16[i] + out_int32[i] + out_int64[i];
        checksum += (int64_t)out_float[i] + (int64_t)out_double[i];
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (int64_t)arr2d_out[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
