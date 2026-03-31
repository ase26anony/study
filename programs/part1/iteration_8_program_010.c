#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Simple deterministic RNG for reproducible results */
static uint32_t seed = 123456789;
static uint32_t rand_det() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < N; i++) {
        a8[i] = (int8_t)(rand_det() % 256 - 128);
        b8[i] = (int8_t)(rand_det() % 256 - 128);
        a16[i] = (int16_t)(rand_det() % 65536 - 32768);
        b16[i] = (int16_t)(rand_det() % 65536 - 32768);
        a32[i] = (int32_t)rand_det();
        b32[i] = (int32_t)rand_det();
        a64[i] = ((int64_t)rand_det() << 32) | rand_det();
        b64[i] = ((int64_t)rand_det() << 32) | rand_det();
        af[i] = (float)(rand_det() % 1000) / 10.0f;
        bf[i] = (float)(rand_det() % 1000) / 10.0f;
        ad[i] = (double)(rand_det() % 1000) / 10.0;
        bd[i] = (double)(rand_det() % 1000) / 10.0;
    }
}

/* GT_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + 1.0f : b[i] - 1.0f;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] * 2.0 : b[i] / 2.0;
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] : (int16_t)(b[i] * 3);
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            out[i] = a[i] + b[i];
        } else {
            out[i] = a[i] - b[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] : -b[i];
    }
}

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int8_swapped(int8_t *restrict a, int8_t *restrict b,
                                  int8_t *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            /* Direct: a[i] < b[i] */
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        } else {
            /* Swapped: b[i] < a[i] - should trigger std::swap logic */
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] < b[i]) ? a[i] << 1 : b[i] >> 1;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] < b[i]) ? a[i] * 1.5 : b[i] * 0.5;
    }
}

/* LE_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int16_swapped(int16_t *restrict a, int16_t *restrict b,
                                   int16_t *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            /* Direct: a[i] <= b[i] */
            out[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        } else {
            /* Swapped: b[i] <= a[i] - should trigger std::swap logic */
            out[i] = (b[i] <= a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] <= b[i]) ? a[i] * b[i] : a[i] / b[i];
    }
}

/* Multi-dimensional array comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d(float a[M][M], float b[M][M], float out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] : -b[i][j];
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out, int stride) {
    for (int i = 0; i < N; i += stride) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* Complex nested conditional with multiple comparison types */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = a[i] + c[i];
        } else if (a[i] >= c[i]) {
            out[i] = b[i] - c[i];
        } else if (b[i] < c[i]) {
            out[i] = a[i] * b[i];
        } else if (b[i] <= a[i]) {
            out[i] = c[i] / 2;
        } else {
            out[i] = 0;
        }
    }
}

int main() {
    /* Allocate and initialize arrays */
    int8_t *a8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *b8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *out8 = __builtin_alloca(N * sizeof(int8_t));
    
    int16_t *a16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *b16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *out16 = __builtin_alloca(N * sizeof(int16_t));
    
    int32_t *a32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *b32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *c32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *out32 = __builtin_alloca(N * sizeof(int32_t));
    
    int64_t *a64 = __builtin_alloca(N * sizeof(int64_t));
    int64_t *b64 = __builtin_alloca(N * sizeof(int64_t));
    int64_t *out64 = __builtin_alloca(N * sizeof(int64_t));
    
    float *af = __builtin_alloca(N * sizeof(float));
    float *bf = __builtin_alloca(N * sizeof(float));
    float *outf = __builtin_alloca(N * sizeof(float));
    
    double *ad = __builtin_alloca(N * sizeof(double));
    double *bd = __builtin_alloca(N * sizeof(double));
    double *outd = __builtin_alloca(N * sizeof(double));
    
    /* Multi-dimensional arrays */
    int32_t arr2d_a[M][M];
    int32_t arr2d_b[M][M];
    int32_t arr2d_out[M][M];
    
    float farr2d_a[M][M];
    float farr2d_b[M][M];
    float farr2d_out[M][M];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d_a[i][j] = rand_det() % 1000;
            arr2d_b[i][j] = rand_det() % 1000;
            farr2d_a[i][j] = (float)(rand_det() % 1000) / 10.0f;
            farr2d_b[i][j] = (float)(rand_det() % 1000) / 10.0f;
        }
    }
    
    /* Initialize c32 array */
    for (int i = 0; i < N; i++) {
        c32[i] = (int32_t)rand_det();
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        vector_gt_double(ad, bd, outd);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_int32(a32, b32, out32);
        vector_ge_float(af, bf, outf);
        
        /* LT_EXPR tests with swapped operands */
        vector_lt_int8_swapped(a8, b8, out8, mode);
        vector_lt_int64(a64, b64, out64);
        vector_lt_double(ad, bd, outd);
        
        /* LE_EXPR tests with swapped operands */
        vector_le_int16_swapped(a16, b16, out16, mode);
        vector_le_int32(a32, b32, out32);
        vector_le_float(af, bf, outf);
        
        /* Multi-dimensional tests */
        vector_gt_2d(arr2d_a, arr2d_b, arr2d_out);
        vector_le_2d(farr2d_a, farr2d_b, farr2d_out);
        
        /* Strided access test */
        vector_ge_strided(a32, b32, out32, 4);
        
        /* Mixed comparisons test */
        vector_mixed_comparisons(a32, b32, c32, out32);
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += out8[i] + out16[i] + out32[i] + (int64_t)outf[i];
            if (i % 4 == 0) {
                checksum += out64[i] + (int64_t)outd[i];
            }
        }
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                checksum += arr2d_out[i][j] + (int64_t)farr2d_out[i][j];
            }
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
