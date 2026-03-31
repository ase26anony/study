#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Simple deterministic pseudo-random generator */
static uint32_t seed = 123456789;
static inline uint32_t lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < N; i++) {
        a8[i] = (int8_t)(lcg_rand() % 256 - 128);
        b8[i] = (int8_t)(lcg_rand() % 256 - 128);
        a16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        b16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        a32[i] = (int32_t)lcg_rand();
        b32[i] = (int32_t)lcg_rand();
        a64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        b64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        af[i] = (float)(lcg_rand() % 1000) / 10.0f;
        bf[i] = (float)(lcg_rand() % 1000) / 10.0f;
        ad[i] = (double)(lcg_rand() % 1000) / 10.0;
        bd[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
}

/* GT_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] > b[i] ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] > b[i] ? a[i] + b[i] : a[i] - b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = a[i] * 2.0;
        } else {
            out[i] = b[i] / 2.0;
        }
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] * b[i] : a[i] / (b[i] + 1.0f);
    }
}

/* LT_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int8_swapped(int8_t *restrict a, int8_t *restrict b,
                                  int8_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] < b[i] ? a[i] : b[i];
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = b[i] < a[i] ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] < b[i] ? a[i] + 1 : b[i] - 1;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            out[i] = a[i] * 3.0;
        } else {
            out[i] = b[i] * 0.5;
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int16_swapped(int16_t *restrict a, int16_t *restrict b,
                                   int16_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] <= b[i] ? a[i] : b[i];
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = b[i] <= a[i] ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] <= b[i]) ? (a[i] | b[i]) : (a[i] & b[i]);
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] <= b[i] ? a[i] + 2.0f * b[i] : a[i] - 2.0f * b[i];
    }
}

/* Multi-dimensional array comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] > b[i][j] ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d(float a[M][M], float b[M][M], float out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] <= b[i][j] ? a[i][j] + b[i][j] : a[i][j] - b[i][j];
        }
    }
}

/* Strided access patterns */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out, int stride) {
    for (int i = 0; i < N; i += stride) {
        out[i] = a[i] >= b[i] ? a[i] * 2 : b[i] * 3;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_strided(double *restrict a, double *restrict b,
                             double *restrict out, int stride) {
    for (int i = 0; i < N; i += stride) {
        out[i] = a[i] < b[i] ? a[i] + b[i] : a[i] - b[i];
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int8_t *a8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *b8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *out8 = __builtin_alloca(N * sizeof(int8_t));
    
    int16_t *a16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *b16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *out16 = __builtin_alloca(N * sizeof(int16_t));
    
    int32_t *a32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *b32 = __builtin_alloca(N * sizeof(int32_t));
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
    
    /* 2D arrays */
    int32_t a2d[M][M], b2d[M][M], out2d[M][M];
    float af2d[M][M], bf2d[M][M], outf2d[M][M];
    
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            a2d[i][j] = (int32_t)lcg_rand();
            b2d[i][j] = (int32_t)lcg_rand();
            af2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            bf2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    int64_t checksum = 0;
    
    /* Execute all comparison variants with different modes */
    for (int mode = 0; mode < 2; mode++) {
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        vector_gt_double(ad, bd, outd);
        
        vector_ge_int16(a16, b16, out16);
        vector_ge_int32(a32, b32, out32);
        vector_ge_float(af, bf, outf);
        
        vector_lt_int8_swapped(a8, b8, out8, mode);
        vector_lt_int64(a64, b64, out64);
        vector_lt_double(ad, bd, outd);
        
        vector_le_int16_swapped(a16, b16, out16, mode);
        vector_le_int32(a32, b32, out32);
        vector_le_float(af, bf, outf);
        
        vector_gt_2d(a2d, b2d, out2d);
        vector_le_2d(af2d, bf2d, outf2d);
        
        vector_ge_strided(a32, b32, out32, 2);
        vector_lt_strided(ad, bd, outd, 4);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += out2d[i][j] + (int64_t)outf2d[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
