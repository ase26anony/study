#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Simple deterministic pseudo-random generator */
static uint32_t lcg_seed = 123456789;
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
        a64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        b64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        af[i] = (float)(lcg_rand() % 1000) / 10.0f;
        bf[i] = (float)(lcg_rand() % 1000) / 10.0f;
        ad[i] = (double)(lcg_rand() % 1000) / 10.0;
        bd[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
}

/* GT_EXPR variants with __attribute__((optimize)) */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + b[i] : a[i] - b[i];
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
        out[i] = (a[i] >= b[i]) ? a[i] * 2.0 : b[i] * 0.5;
    }
}

/* LT_EXPR variants - with swapped operands in different branches */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int n, int mode) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int n, int mode) {
    for (int i = 0; i < n; i++) {
        if (mode) {
            out[i] = (a[i] < b[i]) ? a[i] * b[i] : a[i] / (b[i] + 1.0f);
        } else {
            out[i] = (b[i] < a[i]) ? b[i] * a[i] : b[i] / (a[i] + 1.0f);
        }
    }
}

/* LE_EXPR variants - with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int n, int mode) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int n, int mode) {
    for (int i = 0; i < n; i++) {
        out[i] = (mode ? (a[i] <= b[i]) : (b[i] <= a[i])) 
                 ? a[i] * b[i] : a[i] + b[i];
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[M][M], int32_t b[M][M], 
                        int32_t out[M][M], int mode) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (mode) {
                out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
            } else {
                out[i][j] = (b[i][j] > a[i][j]) ? b[i][j] : a[i][j];
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(float *restrict a, float *restrict b,
                             float *restrict out, int n, int stride) {
    for (int i = 0; i < n; i += stride) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* Complex conditional with multiple comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out,
                                    int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i] * 2;
        } else if (a[i] >= b[i] - 10) {
            out[i] = c[i] + 5;
        } else if (a[i] < b[i]) {
            out[i] = c[i] - 3;
        } else if (a[i] <= b[i] + 10) {
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
    int32_t arr_a[M][M], arr_b[M][M], arr_out[M][M];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr_a[i][j] = (int32_t)lcg_rand() % 1000;
            arr_b[i][j] = (int32_t)lcg_rand() % 1000;
        }
    }
    
    /* Initialize c32 array */
    for (int i = 0; i < N; i++) {
        c32[i] = (int32_t)lcg_rand() % 1000;
    }
    
    int64_t total_checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8, N);
        vector_gt_float(af, bf, outf, N);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16, N);
        vector_ge_double(ad, bd, outd, N);
        
        /* LT_EXPR tests with swapped operands */
        vector_lt_int32(a32, b32, out32, N, mode);
        vector_lt_float(af, bf, outf, N, mode);
        
        /* LE_EXPR tests with swapped operands */
        vector_le_int64(a64, b64, out64, N, mode);
        vector_le_double(ad, bd, outd, N, mode);
        
        /* Multi-dimensional test */
        vector_gt_2d(arr_a, arr_b, arr_out, mode);
        
        /* Strided access test */
        vector_ge_strided(af, bf, outf, N, 2);
        
        /* Mixed comparisons test */
        vector_mixed_comparisons(a32, b32, c32, out32, N);
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += out8[i] + out16[i] + out32[i] + out64[i];
            total_checksum += (int64_t)outf[i] + (int64_t)outd[i];
        }
        
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                total_checksum += arr_out[i][j];
            }
        }
    }
    
    printf("Total checksum: %ld\n", total_checksum);
    return 0;
}
