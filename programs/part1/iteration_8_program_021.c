#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Simple deterministic pseudo-random generator */
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
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < N; i++) {
        a8[i] = (int8_t)(rand_int() % 256 - 128);
        b8[i] = (int8_t)(rand_int() % 256 - 128);
        a16[i] = (int16_t)(rand_int() % 65536 - 32768);
        b16[i] = (int16_t)(rand_int() % 65536 - 32768);
        a32[i] = (int32_t)rand_int();
        b32[i] = (int32_t)rand_int();
        a64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        b64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        af[i] = rand_float() * 1000.0f - 500.0f;
        bf[i] = rand_float() * 1000.0f - 500.0f;
        ad[i] = rand_double() * 1000.0 - 500.0;
        bd[i] = rand_double() * 1000.0 - 500.0;
    }
}

/* GT_EXPR variants with different data types */
__attribute__((optimize("O3", "tree-vectorize")))
static void gt_comparison_int8(int8_t *restrict a, int8_t *restrict b, 
                              int8_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void gt_comparison_float(float *restrict a, float *restrict b,
                               float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + 1.0f : b[i] - 1.0f;
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void ge_comparison_int16(int16_t *restrict a, int16_t *restrict b,
                               int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 2 : b[i] / 2;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void ge_comparison_double(double *restrict a, double *restrict b,
                                double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 1.5 : b[i] * 0.5;
    }
}

/* LT_EXPR variants with swapped operands based on mode */
__attribute__((optimize("O3", "tree-vectorize")))
static void lt_comparison_int32(int32_t *restrict a, int32_t *restrict b,
                               int32_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] < b[i]) ? a[i] + 100 : b[i] - 100;
        }
    } else {
        /* Swapped order: b[i] < a[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] < a[i]) ? b[i] + 200 : a[i] - 200;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void lt_comparison_float_nested(float *restrict a, float *restrict b,
                                      float *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            out[i] = a[i] * 2.0f;
        } else if (b[i] < a[i]) {
            out[i] = b[i] * 3.0f;
        } else {
            out[i] = (a[i] + b[i]) / 2.0f;
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void le_comparison_int64(int64_t *restrict a, int64_t *restrict b,
                               int64_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] <= b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] | 0xFF : b[i] & ~0xFF;
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] | 0xAA : a[i] & ~0xAA;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void le_comparison_mixed(int32_t *restrict a, int32_t *restrict b,
                               float *restrict c, float *restrict d,
                               float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] <= b[i]) ? c[i] : d[i];
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void md_gt_comparison(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void md_le_comparison(float a[M][M], float b[M][M], float out[M][M], int mode) {
    if (mode) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] * 2.0f : b[i][j] / 2.0f;
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] * 3.0f : a[i][j] / 3.0f;
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void strided_ge_comparison(double *restrict a, double *restrict b,
                                 double *restrict out, int stride) {
    for (int i = 0; i < N; i += stride) {
        out[i] = (a[i] >= b[i]) ? a[i] : -b[i];
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int8_t *a8 = (int8_t*)aligned_alloc(64, N * sizeof(int8_t));
    int8_t *b8 = (int8_t*)aligned_alloc(64, N * sizeof(int8_t));
    int8_t *out8 = (int8_t*)aligned_alloc(64, N * sizeof(int8_t));
    
    int16_t *a16 = (int16_t*)aligned_alloc(64, N * sizeof(int16_t));
    int16_t *b16 = (int16_t*)aligned_alloc(64, N * sizeof(int16_t));
    int16_t *out16 = (int16_t*)aligned_alloc(64, N * sizeof(int16_t));
    
    int32_t *a32 = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    int32_t *b32 = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    int32_t *out32 = (int32_t*)aligned_alloc(64, N * sizeof(int32_t));
    
    int64_t *a64 = (int64_t*)aligned_alloc(64, N * sizeof(int64_t));
    int64_t *b64 = (int64_t*)aligned_alloc(64, N * sizeof(int64_t));
    int64_t *out64 = (int64_t*)aligned_alloc(64, N * sizeof(int64_t));
    
    float *af = (float*)aligned_alloc(64, N * sizeof(float));
    float *bf = (float*)aligned_alloc(64, N * sizeof(float));
    float *outf = (float*)aligned_alloc(64, N * sizeof(float));
    float *cf = (float*)aligned_alloc(64, N * sizeof(float));
    float *df = (float*)aligned_alloc(64, N * sizeof(float));
    
    double *ad = (double*)aligned_alloc(64, N * sizeof(double));
    double *bd = (double*)aligned_alloc(64, N * sizeof(double));
    double *outd = (double*)aligned_alloc(64, N * sizeof(double));
    
    int32_t md_a[M][M];
    int32_t md_b[M][M];
    int32_t md_out[M][M];
    
    float md_af[M][M];
    float md_bf[M][M];
    float md_outf[M][M];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    for (int i = 0; i < N; i++) {
        cf[i] = rand_float() * 200.0f - 100.0f;
        df[i] = rand_float() * 200.0f - 100.0f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = rand_int() % 1000;
            md_b[i][j] = rand_int() % 1000;
            md_af[i][j] = rand_float() * 500.0f;
            md_bf[i][j] = rand_float() * 500.0f;
        }
    }
    
    long long checksum = 0;
    
    /* Execute all comparison variants with different modes */
    for (int mode = 0; mode < 2; mode++) {
        gt_comparison_int8(a8, b8, out8);
        gt_comparison_float(af, bf, outf);
        ge_comparison_int16(a16, b16, out16);
        ge_comparison_double(ad, bd, outd);
        lt_comparison_int32(a32, b32, out32, mode);
        lt_comparison_float_nested(af, bf, outf);
        le_comparison_int64(a64, b64, out64, mode);
        le_comparison_mixed(a32, b32, af, bf, outf);
        md_gt_comparison(md_a, md_b, md_out);
        md_le_comparison(md_af, md_bf, md_outf, mode);
        strided_ge_comparison(ad, bd, outd, 4);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + (int)outf[i];
        checksum += (long long)(outd[i] * 1000.0);
        if (i < N/4) {
            checksum += out64[i];
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += md_out[i][j] + (int)(md_outf[i][j] * 100.0f);
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(out32);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(outf); free(cf); free(df);
    free(ad); free(bd); free(outd);
    
    return 0;
}
