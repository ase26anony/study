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
        a64[i] = ((int64_t)lcg_rand() << 32) | lcg_rand();
        b64[i] = ((int64_t)lcg_rand() << 32) | lcg_rand();
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

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] * 2.0 : b[i] / 2.0;
    }
}

/* LT_EXPR variants - with swapped operands in conditional */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] < b[i] ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped operands: b[i] < a[i] instead of a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = b[i] < a[i] ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            out[i] = a[i] < b[i] ? a[i] : b[i];
        } else {
            /* Swapped comparison */
            out[i] = b[i] < a[i] ? b[i] : a[i];
        }
    }
}

/* LE_EXPR variants - with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] <= b[i] ? a[i] | b[i] : a[i] & b[i];
        }
    } else {
        /* Swapped operands */
        for (int i = 0; i < N; i++) {
            out[i] = b[i] <= a[i] ? b[i] | a[i] : b[i] & a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        out[i] = (mode ? a[i] <= b[i] : b[i] <= a[i]) ? a[i] : b[i];
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_multi_dim(int32_t a[M][M], int32_t b[M][M],
                               int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] > b[i][j] ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_multi_dim(float a[M][M], float b[M][M],
                               float out[M][M], int mode) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (mode) {
                out[i][j] = a[i][j] <= b[i][j] ? a[i][j] * 2.0f : b[i][j] * 2.0f;
            } else {
                /* Swapped comparison with stride access pattern */
                out[i][j] = b[i][j] <= a[i][j] ? b[i][j] * 3.0f : a[i][j] * 3.0f;
            }
        }
    }
}

/* Complex conditional with mixed comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict d,
                                    int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i];
        } else if (a[i] <= b[i]) {
            out[i] = d[i];
        } else {
            out[i] = a[i] + b[i];
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_lt(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out) {
    for (int i = 0; i < N/2; i++) {
        int idx = i * 2;
        out[idx] = a[idx] < b[idx] ? a[idx] : b[idx];
        out[idx + 1] = a[idx + 1] < b[idx + 1] ? a[idx + 1] : b[idx + 1];
    }
}

int main() {
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
    int32_t *d32 = malloc(N * sizeof(int32_t));
    int32_t *out32 = malloc(N * sizeof(int32_t));
    int32_t *out32_2 = malloc(N * sizeof(int32_t));
    
    int64_t *a64 = malloc(N * sizeof(int64_t));
    int64_t *b64 = malloc(N * sizeof(int64_t));
    int64_t *out64 = malloc(N * sizeof(int64_t));
    
    float *af = malloc(N * sizeof(float));
    float *bf = malloc(N * sizeof(float));
    float *outf = malloc(N * sizeof(float));
    float *outf2 = malloc(N * sizeof(float));
    
    double *ad = malloc(N * sizeof(double));
    double *bd = malloc(N * sizeof(double));
    double *outd = malloc(N * sizeof(double));
    
    /* Multi-dimensional arrays */
    int32_t md_a[M][M], md_b[M][M], md_out[M][M];
    float md_af[M][M], md_bf[M][M], md_outf[M][M];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = lcg_rand() % 1000;
            md_b[i][j] = lcg_rand() % 1000;
            md_af[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            md_bf[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    /* Initialize c32 and d32 for mixed comparisons */
    for (int i = 0; i < N; i++) {
        c32[i] = lcg_rand() % 1000;
        d32[i] = lcg_rand() % 1000;
    }
    
    long long checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        
        /* LT_EXPR tests with swapped operands */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf2, mode);
        
        /* LE_EXPR tests with swapped operands */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        
        /* Multi-dimensional tests */
        vector_gt_multi_dim(md_a, md_b, md_out);
        vector_le_multi_dim(md_af, md_bf, md_outf, mode);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, d32, out32_2);
        
        /* Strided access */
        vector_strided_lt(a32, b32, out32);
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += out8[i] + out16[i] + out32[i] + out32_2[i] + 
                       (int64_t)out64[i] + (int)outf[i] + (int)outf2[i] + (int)outd[i];
        }
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                checksum += md_out[i][j] + (int)md_outf[i][j];
            }
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Free allocated memory */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(c32); free(d32); free(out32); free(out32_2);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(outf); free(outf2);
    free(ad); free(bd); free(outd);
    
    return 0;
}
