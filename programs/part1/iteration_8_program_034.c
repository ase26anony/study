#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Deterministic pseudo-random number generator */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
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
        out[i] = a[i] >= b[i] ? a[i] * 2 : b[i] / 2;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] * b[i] : a[i] / (b[i] + 1.0);
    }
}

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = a[i] < b[i] ? a[i] - b[i] : b[i] - a[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < N; i++) {
            out[i] = b[i] < a[i] ? b[i] + a[i] : a[i] + b[i];
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
            out[i] = b[i] < a[i] ? b[i] : a[i];
        }
    }
}

/* LE_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = a[i] <= b[i] ? a[i] | b[i] : a[i] & b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] - should trigger std::swap logic */
        for (int i = 0; i < N; i++) {
            out[i] = b[i] <= a[i] ? b[i] ^ a[i] : a[i] ^ b[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        out[i] = (mode ? a[i] <= b[i] : b[i] <= a[i]) ? 
                 a[i] * 2.0 : b[i] * 3.0;
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gt(int32_t a[][M], int32_t b[][M], 
                        int32_t out[][M]) {
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] > b[i][j] ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_le(float a[][M], float b[][M],
                        float out[][M], int mode) {
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            if (mode) {
                out[i][j] = a[i][j] <= b[i][j] ? a[i][j] : b[i][j];
            } else {
                out[i][j] = b[i][j] <= a[i][j] ? b[i][j] : a[i][j];
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_ge(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out) {
    const int stride = 4;
    for (int i = 0; i < N; i += stride) {
        out[i] = a[i] >= b[i] ? a[i] : b[i];
    }
}

/* Complex conditional with multiple comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i] * 2;
        } else if (a[i] < b[i]) {
            out[i] = c[i] / 2;
        } else {
            out[i] = c[i];
        }
    }
}

int main(void) {
    /* Declare arrays */
    int8_t a8[N], b8[N], out8[N];
    int16_t a16[N], b16[N], out16[N];
    int32_t a32[N], b32[N], out32[N], c32[N];
    int64_t a64[N], b64[N], out64[N];
    float af[N], bf[N], outf[N];
    double ad[N], bd[N], outd[N];
    
    /* Multi-dimensional arrays */
    int32_t md_a[N/M][M], md_b[N/M][M], md_out[N/M][M];
    float md_af[N/M][M], md_bf[N/M][M], md_outf[N/M][M];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = lcg_rand() % 1000;
            md_b[i][j] = lcg_rand() % 1000;
            md_af[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            md_bf[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    /* Initialize c32 array */
    for (int i = 0; i < N; i++) {
        c32[i] = lcg_rand() % 1000;
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        
        /* LT_EXPR tests with mode toggling */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        
        /* LE_EXPR tests with mode toggling */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        
        /* Multi-dimensional tests */
        vector_md_gt(md_a, md_b, md_out);
        vector_md_le(md_af, md_bf, md_outf, mode);
        
        /* Strided access test */
        vector_strided_ge(a32, b32, out32);
        
        /* Mixed comparisons test */
        vector_mixed_comparisons(a32, b32, c32, out32);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + (int64_t)outf[i];
        if (i < N/2) checksum += out64[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += md_out[i][j] + (int64_t)md_outf[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
