#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Deterministic pseudo-random generator */
static uint32_t seed = 123456789;
static inline uint32_t lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int16_t *a16, int32_t *a32, int64_t *a64,
                        float *af, double *ad) {
    for (int i = 0; i < N; i++) {
        a8[i] = (int8_t)(lcg_rand() % 256 - 128);
        a16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        a32[i] = (int32_t)lcg_rand();
        a64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        af[i] = (float)(lcg_rand() % 1000) / 10.0f;
        ad[i] = (double)(lcg_rand() % 1000) / 10.0;
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
                           float *restrict c, float *restrict d,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] > b[i] ? c[i] : d[i];
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] + b[i] : a[i] - b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            out[i] = a[i] * 2.0;
        } else {
            out[i] = b[i] * 0.5;
        }
    }
}

/* LT_EXPR variants - with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
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
static void vector_lt_float_nested(float *restrict a, float *restrict b,
                                  float *restrict c, float *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            out[i] = c[i] * 2.0f;
        } else if (b[i] < a[i]) {
            out[i] = c[i] * 0.5f;
        } else {
            out[i] = c[i];
        }
    }
}

/* LE_EXPR variants - with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] <= b[i] ? a[i] + 1 : b[i] - 1;
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = b[i] <= a[i] ? b[i] + 1 : a[i] - 1;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double_ternary(double *restrict a, double *restrict b,
                                    double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] <= b[i] ? a[i] * b[i] : a[i] / b[i];
    }
}

/* Multi-dimensional array comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] > b[i][j] ? 1 : 0;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d_strided(float a[M][M], float b[M][M], float out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j += 2) {
            out[i][j] = a[i][j] <= b[i][j] ? a[i][j] : b[i][j];
        }
    }
}

/* Complex mixed-type comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, float *restrict b,
                                    double *restrict c, int8_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > (int32_t)b[i]) ? 
                 (c[i] <= (double)a[i] ? 1 : 2) : 
                 (b[i] < (float)a[i] ? 3 : 4);
    }
}

int main(void) {
    /* Allocate arrays */
    int8_t *a8 = (int8_t*)malloc(N * sizeof(int8_t));
    int8_t *b8 = (int8_t*)malloc(N * sizeof(int8_t));
    int8_t *out8 = (int8_t*)malloc(N * sizeof(int8_t));
    
    int16_t *a16 = (int16_t*)malloc(N * sizeof(int16_t));
    int16_t *b16 = (int16_t*)malloc(N * sizeof(int16_t));
    int16_t *out16 = (int16_t*)malloc(N * sizeof(int16_t));
    
    int32_t *a32 = (int32_t*)malloc(N * sizeof(int32_t));
    int32_t *b32 = (int32_t*)malloc(N * sizeof(int32_t));
    int32_t *out32 = (int32_t*)malloc(N * sizeof(int32_t));
    
    int64_t *a64 = (int64_t*)malloc(N * sizeof(int64_t));
    int64_t *b64 = (int64_t*)malloc(N * sizeof(int64_t));
    int64_t *out64 = (int64_t*)malloc(N * sizeof(int64_t));
    
    float *af = (float*)malloc(N * sizeof(float));
    float *bf = (float*)malloc(N * sizeof(float));
    float *cf = (float*)malloc(N * sizeof(float));
    float *df = (float*)malloc(N * sizeof(float));
    float *outf = (float*)malloc(N * sizeof(float));
    
    double *ad = (double*)malloc(N * sizeof(double));
    double *bd = (double*)malloc(N * sizeof(double));
    double *outd = (double*)malloc(N * sizeof(double));
    
    int32_t arr2d_a[M][M], arr2d_b[M][M], arr2d_out[M][M];
    float farr2d_a[M][M], farr2d_b[M][M], farr2d_out[M][M];
    
    /* Initialize arrays */
    init_arrays(a8, a16, a32, a64, af, ad);
    init_arrays(b8, b16, b32, b64, bf, bd);
    init_arrays(NULL, NULL, NULL, NULL, cf, bd); /* Reuse for cf */
    init_arrays(NULL, NULL, NULL, NULL, df, bd); /* Reuse for df */
    
    /* Initialize 2D arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d_a[i][j] = lcg_rand() % 1000;
            arr2d_b[i][j] = lcg_rand() % 1000;
            farr2d_a[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            farr2d_b[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    int64_t checksum = 0;
    
    /* Test GT_EXPR variants */
    vector_gt_int8(a8, b8, out8);
    vector_gt_float(af, bf, cf, df, outf);
    
    /* Test GE_EXPR variants */
    vector_ge_int16(a16, b16, out16);
    vector_ge_double(ad, bd, outd);
    
    /* Test LT_EXPR variants with swapped operands */
    for (int mode = 0; mode < 2; mode++) {
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float_nested(af, bf, cf, outf);
    }
    
    /* Test LE_EXPR variants with swapped operands */
    for (int mode = 0; mode < 2; mode++) {
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double_ternary(ad, bd, outd);
    }
    
    /* Test multi-dimensional comparisons */
    vector_gt_2d(arr2d_a, arr2d_b, arr2d_out);
    vector_le_2d_strided(farr2d_a, farr2d_b, farr2d_out);
    
    /* Test mixed-type comparisons */
    vector_mixed_comparisons(a32, af, ad, out8);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr2d_out[i][j] + (int64_t)farr2d_out[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(out32);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(cf); free(df); free(outf);
    free(ad); free(bd); free(outd);
    
    return 0;
}
