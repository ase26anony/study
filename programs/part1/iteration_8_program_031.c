#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Deterministic pseudo-random number generator */
static uint32_t lcg = 123456789;
static inline uint32_t rand_lcg(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < N; i++) {
        a8[i] = (int8_t)(rand_lcg() % 256 - 128);
        b8[i] = (int8_t)(rand_lcg() % 256 - 128);
        a16[i] = (int16_t)(rand_lcg() % 65536 - 32768);
        b16[i] = (int16_t)(rand_lcg() % 65536 - 32768);
        a32[i] = (int32_t)rand_lcg();
        b32[i] = (int32_t)rand_lcg();
        a64[i] = ((int64_t)rand_lcg() << 32) | rand_lcg();
        b64[i] = ((int64_t)rand_lcg() << 32) | rand_lcg();
        af[i] = (float)(rand_lcg() % 1000) / 10.0f;
        bf[i] = (float)(rand_lcg() % 1000) / 10.0f;
        ad[i] = (double)(rand_lcg() % 1000) / 10.0;
        bd[i] = (double)(rand_lcg() % 1000) / 10.0;
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

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            out[i] = a[i] * 2;
        } else {
            out[i] = b[i] / 2;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] * b[i] : a[i] / (b[i] + 1.0);
    }
}

/* LT_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int mode, int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] < b[i] ? a[i] + i : b[i] - i;
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = b[i] < a[i] ? b[i] + i : a[i] - i;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] < b[i] ? a[i] * 2.0f : b[i] * 0.5f;
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int mode, int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] <= b[i] ? a[i] << 2 : b[i] >> 2;
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = b[i] <= a[i] ? b[i] << 2 : a[i] >> 2;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] <= b[i] ? a[i] + 1.0 : b[i] - 1.0;
    }
}

/* Multi-dimensional array comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gt(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] > b[i][j] ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_le(int mode, float a[M][M], float b[M][M], float out[M][M]) {
    if (mode) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = a[i][j] <= b[i][j] ? a[i][j] * 2.0f : b[i][j];
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = b[i][j] <= a[i][j] ? b[i][j] * 2.0f : a[i][j];
            }
        }
    }
}

/* Complex conditional with multiple comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i] + 1;
        } else if (a[i] >= c[i]) {
            out[i] = b[i] - 1;
        } else if (b[i] < c[i]) {
            out[i] = a[i] * 2;
        } else if (b[i] <= a[i]) {
            out[i] = c[i] / 2;
        } else {
            out[i] = 0;
        }
    }
}

int main(void) {
    /* Allocate arrays */
    int8_t *a8 = malloc(N * sizeof(int8_t));
    int8_t *b8 = malloc(N * sizeof(int8_t));
    int8_t *out8 = malloc(N * sizeof(int8_t));
    
    int16_t *a16 = malloc(N * sizeof(int16_t));
    int16_t *b16 = malloc(N * sizeof(int16_t));
    int16_t *out16 = malloc(N * sizeof(int16_t));
    
    int32_t *a32 = malloc(N * sizeof(int32_t));
    int32_t *b32 = malloc(N * sizeof(int32_t));
    int32_t *c32 = malloc(N * sizeof(int32_t));
    int32_t *out32 = malloc(N * sizeof(int32_t));
    
    int64_t *a64 = malloc(N * sizeof(int64_t));
    int64_t *b64 = malloc(N * sizeof(int64_t));
    int64_t *out64 = malloc(N * sizeof(int64_t));
    
    float *af = malloc(N * sizeof(float));
    float *bf = malloc(N * sizeof(float));
    float *outf = malloc(N * sizeof(float));
    
    double *ad = malloc(N * sizeof(double));
    double *bd = malloc(N * sizeof(double));
    double *outd = malloc(N * sizeof(double));
    
    int32_t md_a[M][M], md_b[M][M], md_out[M][M];
    float md_af[M][M], md_bf[M][M], md_outf[M][M];
    
    /* Initialize arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    for (int i = 0; i < N; i++) {
        c32[i] = (int32_t)rand_lcg();
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = (int32_t)rand_lcg();
            md_b[i][j] = (int32_t)rand_lcg();
            md_af[i][j] = (float)(rand_lcg() % 1000) / 10.0f;
            md_bf[i][j] = (float)(rand_lcg() % 1000) / 10.0f;
        }
    }
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode < 2; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        
        /* LT_EXPR tests with swapped operands */
        vector_lt_int32(mode, a32, b32, out32);
        vector_lt_float(af, bf, outf);
        
        /* LE_EXPR tests with swapped operands */
        vector_le_int64(mode, a64, b64, out64);
        vector_le_double(ad, bd, outd);
        
        /* Multi-dimensional tests */
        vector_md_gt(md_a, md_b, md_out);
        vector_md_le(mode, md_af, md_bf, md_outf);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, out32);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += md_out[i][j] + (int64_t)md_outf[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(c32); free(out32);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(outf);
    free(ad); free(bd); free(outd);
    
    return 0;
}
