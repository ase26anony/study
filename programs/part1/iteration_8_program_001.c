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

/* GT_EXPR variants with different data types */
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
        out[i] = a[i] >= b[i] ? a[i] : -b[i];
    }
}

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = a[i] < b[i] ? a[i] : b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
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
            out[i] = a[i] < b[i] ? a[i] * b[i] : a[i] / (b[i] + 1.0f);
        } else {
            out[i] = b[i] < a[i] ? b[i] * a[i] : b[i] / (a[i] + 1.0f);
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = a[i] <= b[i] ? a[i] | b[i] : a[i] & b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < N; i++) {
            out[i] = b[i] <= a[i] ? b[i] | a[i] : b[i] & a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        out[i] = (mode ? a[i] <= b[i] : b[i] <= a[i]) ? 
                 a[i] * 0.5 : b[i] * 2.0;
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] > b[i][j] ? a[i][j] : b[i][j];
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(float *restrict a, float *restrict b,
                             float *restrict out) {
    const int stride = 4;
    for (int i = 0; i < N; i += stride) {
        out[i] = a[i] >= b[i] ? a[i] : b[i];
        if (i + 1 < N) out[i + 1] = a[i + 1] >= b[i + 1] ? a[i + 1] : b[i + 1];
        if (i + 2 < N) out[i + 2] = a[i + 2] >= b[i + 2] ? a[i + 2] : b[i + 2];
        if (i + 3 < N) out[i + 3] = a[i + 3] >= b[i + 3] ? a[i + 3] : b[i + 3];
    }
}

/* Complex conditional with mixed comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict d,
                                    int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i] + d[i];
        } else if (a[i] >= b[i] - 10) {
            out[i] = c[i] - d[i];
        } else if (b[i] < a[i]) {  /* Swapped LT */
            out[i] = c[i] * d[i];
        } else if (b[i] <= a[i] + 10) {  /* Swapped LE */
            out[i] = c[i] / (d[i] ? d[i] : 1);
        } else {
            out[i] = 0;
        }
    }
}

int main(void) {
    /* Allocate arrays */
    int8_t *a8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *b8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *out8 = __builtin_alloca(N * sizeof(int8_t));
    
    int16_t *a16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *b16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *out16 = __builtin_alloca(N * sizeof(int16_t));
    
    int32_t *a32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *b32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *out32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *c32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *d32 = __builtin_alloca(N * sizeof(int32_t));
    
    int64_t *a64 = __builtin_alloca(N * sizeof(int64_t));
    int64_t *b64 = __builtin_alloca(N * sizeof(int64_t));
    int64_t *out64 = __builtin_alloca(N * sizeof(int64_t));
    
    float *af = __builtin_alloca(N * sizeof(float));
    float *bf = __builtin_alloca(N * sizeof(float));
    float *outf = __builtin_alloca(N * sizeof(float));
    
    double *ad = __builtin_alloca(N * sizeof(double));
    double *bd = __builtin_alloca(N * sizeof(double));
    double *outd = __builtin_alloca(N * sizeof(double));
    
    int32_t arr2d_a[M][M], arr2d_b[M][M], arr2d_out[M][M];
    
    /* Initialize arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize additional arrays */
    for (int i = 0; i < N; i++) {
        c32[i] = (int32_t)lcg_rand();
        d32[i] = (int32_t)lcg_rand();
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d_a[i][j] = (int32_t)lcg_rand();
            arr2d_b[i][j] = (int32_t)lcg_rand();
        }
    }
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        
        /* LT_EXPR tests with mode switching */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        
        /* LE_EXPR tests with mode switching */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        
        /* Multi-dimensional test */
        vector_gt_2d(arr2d_a, arr2d_b, arr2d_out);
        
        /* Strided access test */
        vector_ge_strided(af, bf, outf);
        
        /* Mixed comparisons test */
        vector_mixed_comparisons(a32, b32, c32, d32, out32);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + (int64_t)outf[i];
        if (i % 4 == 0) {
            checksum += out64[i] + (int64_t)outd[i];
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr2d_out[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
