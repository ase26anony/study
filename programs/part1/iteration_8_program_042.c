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

/* GT_EXPR variants with different data types */
__attribute__((optimize("O3", "tree-vectorize")))
static void gt_comparison_int8(int8_t *restrict a, int8_t *restrict b, 
                              int8_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void gt_comparison_float(float *restrict a, float *restrict b,
                               float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + 1.0f : b[i] - 1.0f;
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void ge_comparison_int16(int16_t *restrict a, int16_t *restrict b,
                               int16_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 2 : b[i] / 2;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void ge_comparison_double(double *restrict a, double *restrict b,
                                double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 1.5 : b[i] * 0.5;
    }
}

/* LT_EXPR variants - will trigger std::swap for swapped operands */
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
static void lt_comparison_float_nested(float *restrict a, float *restrict b,
                                      float *restrict c, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            out[i] = c[i] * 2.0f;
        } else if (b[i] < a[i]) {  /* Swapped operands */
            out[i] = c[i] * 0.5f;
        } else {
            out[i] = c[i];
        }
    }
}

/* LE_EXPR variants with swapped operands */
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
static void le_comparison_double_multi(double *restrict a, double *restrict b,
                                      double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons with swapped operands */
        int cond1 = (a[i] <= b[i]);
        int cond2 = (b[i] <= a[i]);  /* Swapped */
        out[i] = cond1 ? (cond2 ? 0.0 : 1.0) : 2.0;
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void md_comparison_gt(int32_t a[][M], int32_t b[][M], 
                            int32_t out[][M], int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void md_comparison_le_swapped(int32_t a[][M], int32_t b[][M],
                                    int32_t out[][M], int rows, int mode) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < M; j++) {
            if (mode) {
                out[i][j] = (a[i][j] <= b[i][j]) ? 1 : 0;
            } else {
                /* Swapped operands in multi-dimensional access */
                out[i][j] = (b[i][j] <= a[i][j]) ? 1 : 0;
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void strided_comparison_lt(float *restrict a, float *restrict b,
                                 float *restrict out, int stride, int n) {
    for (int i = 0; i < n; i += stride) {
        out[i] = (a[i] < b[i]) ? a[i] : b[i];
    }
}

int main(void) {
    /* Declare arrays */
    int8_t a8[N], b8[N], out8[N];
    int16_t a16[N], b16[N], out16[N];
    int32_t a32[N], b32[N], out32[N];
    int64_t a64[N], b64[N], out64[N];
    float af[N], bf[N], outf[N], cf[N];
    double ad[N], bd[N], outd[N];
    
    /* Multi-dimensional arrays */
    int32_t md_a[N/32][M], md_b[N/32][M], md_out[N/32][M];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/32; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = (int32_t)lcg_rand();
            md_b[i][j] = (int32_t)lcg_rand();
        }
    }
    
    /* Initialize auxiliary array */
    for (int i = 0; i < N; i++) {
        cf[i] = (float)(lcg_rand() % 100) / 10.0f;
    }
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        gt_comparison_int8(a8, b8, out8, N);
        gt_comparison_float(af, bf, outf, N);
        
        /* GE_EXPR tests */
        ge_comparison_int16(a16, b16, out16, N);
        ge_comparison_double(ad, bd, outd, N);
        
        /* LT_EXPR tests with swapped operands */
        lt_comparison_int32(a32, b32, out32, mode);
        lt_comparison_float_nested(af, bf, cf, outf, N);
        
        /* LE_EXPR tests with swapped operands */
        le_comparison_int64(a64, b64, out64, mode);
        le_comparison_double_multi(ad, bd, outd, N);
        
        /* Multi-dimensional comparisons */
        md_comparison_gt(md_a, md_b, md_out, N/32);
        md_comparison_le_swapped(md_a, md_b, md_out, N/32, mode);
        
        /* Strided access */
        strided_comparison_lt(af, bf, outf, 4, N);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < N/32; i++) {
        for (int j = 0; j < M; j++) {
            checksum += md_out[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
