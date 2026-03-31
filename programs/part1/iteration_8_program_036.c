#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Deterministic pseudo-random generator */
static uint32_t seed = 123456789;
static uint32_t lcg_rand(void) {
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
        out[i] = (a[i] > b[i]) ? a[i] + b[i] : a[i] - b[i];
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
        out[i] = (a[i] >= b[i]) ? a[i] * b[i] : a[i] / (b[i] + 1.0);
    }
}

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void lt_comparison_int32(int32_t *restrict a, int32_t *restrict b,
                               int32_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void lt_comparison_mixed(int32_t *restrict a, int32_t *restrict b,
                               float *restrict c, float *restrict d,
                               float *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] < b[i]) ? c[i] : d[i];
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] < a[i]) ? d[i] : c[i];
        }
    }
}

/* LE_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void le_comparison_int64(int64_t *restrict a, int64_t *restrict b,
                               int64_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] - should trigger std::swap logic */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void le_comparison_float(float *restrict a, float *restrict b,
                               float *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            if (a[i] <= b[i]) {
                out[i] = a[i] * 2.0f;
            } else {
                out[i] = b[i] * 0.5f;
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            if (b[i] <= a[i]) {
                out[i] = b[i] * 2.0f;
            } else {
                out[i] = a[i] * 0.5f;
            }
        }
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void md_gt_comparison(int32_t a[M][M], int32_t b[M][M], 
                            int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void md_le_comparison(float a[M][M], float b[M][M],
                            float out[M][M], int mode) {
    if (mode) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] + b[i][j] : a[i][j] - b[i][j];
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] + a[i][j] : b[i][j] - a[i][j];
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void strided_lt_comparison(int32_t *restrict a, int32_t *restrict b,
                                 int32_t *restrict out) {
    for (int i = 0; i < N; i += 4) {
        out[i] = (a[i] < b[i]) ? a[i] : b[i];
        out[i+1] = (a[i+1] < b[i+1]) ? a[i+1] : b[i+1];
        out[i+2] = (a[i+2] < b[i+2]) ? a[i+2] : b[i+2];
        out[i+3] = (a[i+3] < b[i+3]) ? a[i+3] : b[i+3];
    }
}

int main(void) {
    /* Declare all arrays */
    int8_t a8[N], b8[N], out8[N];
    int16_t a16[N], b16[N], out16[N];
    int32_t a32[N], b32[N], out32[N], out32_2[N];
    int64_t a64[N], b64[N], out64[N];
    float af[N], bf[N], outf[N], outf2[N], cf[N], df[N];
    double ad[N], bd[N], outd[N];
    
    /* Multi-dimensional arrays */
    int32_t md_a[M][M], md_b[M][M], md_out[M][M];
    float md_af[M][M], md_bf[M][M], md_outf[M][M];
    
    /* Initialize arrays */
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
    
    /* Initialize float arrays for mixed comparison */
    for (int i = 0; i < N; i++) {
        cf[i] = (float)(lcg_rand() % 1000) / 10.0f;
        df[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    long long checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT comparisons */
        gt_comparison_int8(a8, b8, out8);
        gt_comparison_float(af, bf, outf);
        
        /* GE comparisons */
        ge_comparison_int16(a16, b16, out16);
        ge_comparison_double(ad, bd, outd);
        
        /* LT comparisons with mode toggling */
        lt_comparison_int32(a32, b32, out32, mode);
        lt_comparison_mixed(a32, b32, af, bf, outf2, mode);
        
        /* LE comparisons with mode toggling */
        le_comparison_int64(a64, b64, out64, mode);
        le_comparison_float(af, bf, outf, mode);
        
        /* Multi-dimensional comparisons */
        md_gt_comparison(md_a, md_b, md_out);
        md_le_comparison(md_af, md_bf, md_outf, mode);
        
        /* Strided comparison */
        strided_lt_comparison(a32, b32, out32_2);
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += out8[i] + out16[i] + out32[i] + out32_2[i] + 
                       (long long)out64[i] + (long long)outf[i] + 
                       (long long)outf2[i] + (long long)outd[i];
        }
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                checksum += md_out[i][j] + (long long)md_outf[i][j];
            }
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}
