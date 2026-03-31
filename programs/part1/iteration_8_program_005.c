#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32

/* Simple deterministic PRNG for reproducible results */
static uint32_t seed = 123456789;
static inline uint32_t lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int8_t *a8, int16_t *a16, int32_t *a32, int64_t *a64,
                       float *af, double *ad, int n) {
    for (int i = 0; i < n; i++) {
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
                          int8_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict c, float *restrict d,
                           float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? c[i] : d[i];
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* LT_EXPR variants - with swapped operands in different branches */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode, int n) {
    if (mode) {
        /* Original order: a[i] < b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float_if(float *restrict a, float *restrict b,
                              float *restrict out, int mode, int n) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            if (a[i] < b[i]) {
                out[i] = a[i];
            } else {
                out[i] = b[i];
            }
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (b[i] < a[i]) {
                out[i] = b[i];
            } else {
                out[i] = a[i];
            }
        }
    }
}

/* LE_EXPR variants - with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode, int n) {
    if (mode) {
        /* Original order: a[i] <= b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double_if(double *restrict a, double *restrict b,
                               double *restrict out, int mode, int n) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            if (a[i] <= b[i]) {
                out[i] = a[i];
            } else {
                out[i] = b[i];
            }
        }
    } else {
        for (int i = 0; i < n; i++) {
            if (b[i] <= a[i]) {
                out[i] = b[i];
            } else {
                out[i] = a[i];
            }
        }
    }
}

/* Multi-dimensional array access with comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[M][N], int32_t b[M][N], 
                        int32_t out[M][N], int mode) {
    if (mode) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
            }
        }
    } else {
        /* Different access pattern */
        for (int j = 0; j < N; j++) {
            for (int i = 0; i < M; i++) {
                out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
            }
        }
    }
}

/* Complex expression with multiple comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict d,
                                    int32_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT, GE, LT, LE in one loop */
        int32_t temp1 = (a[i] > b[i]) ? c[i] : d[i];
        int32_t temp2 = (a[i] >= b[i]) ? temp1 : c[i];
        int32_t temp3 = (a[i] < b[i]) ? temp2 : d[i];
        out[i] = (a[i] <= b[i]) ? temp3 : temp1;
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int8_t a8[N], b8[N], out8[N];
    int16_t a16[N], b16[N], out16[N];
    int32_t a32[N], b32[N], c32[N], d32[N], out32[N];
    int64_t a64[N], b64[N], out64[N];
    float af[N], bf[N], cf[N], df[N], outf[N];
    double ad[N], bd[N], cd[N], dd[N], outd[N];
    
    /* Multi-dimensional arrays */
    int32_t arr_a[M][N], arr_b[M][N], arr_out[M][N];
    
    init_arrays(a8, a16, a32, a64, af, ad, N);
    init_arrays(b8, b16, b32, b64, bf, bd, N);
    init_arrays(c32, d32, c32, b64, cf, cd, N); /* Reuse for c/d arrays */
    
    /* Initialize 2D arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr_a[i][j] = (int32_t)lcg_rand();
            arr_b[i][j] = (int32_t)lcg_rand();
        }
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8, N);
        vector_gt_float(af, bf, cf, df, outf, N);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16, N);
        vector_ge_double(ad, bd, outd, N);
        
        /* LT_EXPR tests with swapped operands */
        vector_lt_int32(a32, b32, out32, mode, N);
        vector_lt_float_if(af, bf, outf, mode, N);
        
        /* LE_EXPR tests with swapped operands */
        vector_le_int64(a64, b64, out64, mode, N);
        vector_le_double_if(ad, bd, outd, mode, N);
        
        /* Multi-dimensional test */
        vector_gt_2d(arr_a, arr_b, arr_out, mode);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, d32, out32, N);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr_out[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
