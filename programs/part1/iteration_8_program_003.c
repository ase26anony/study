#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Simple deterministic pseudo-random generator */
static uint32_t seed = 123456789;
static uint32_t lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *fa, float *fb, double *da, double *db) {
    for (int i = 0; i < N; i++) {
        a8[i] = (int8_t)(lcg_rand() % 256 - 128);
        b8[i] = (int8_t)(lcg_rand() % 256 - 128);
        a16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        b16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        a32[i] = (int32_t)lcg_rand();
        b32[i] = (int32_t)lcg_rand();
        a64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        b64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        fa[i] = (float)(lcg_rand() % 1000) / 10.0f;
        fb[i] = (float)(lcg_rand() % 1000) / 10.0f;
        da[i] = (double)(lcg_rand() % 1000) / 10.0;
        db[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
}

/* GT_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + 1.0f : b[i] - 1.0f;
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 2 : b[i] / 2;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 1.5 : b[i] * 0.5;
    }
}

/* LT_EXPR variants - with swapped operands in different modes */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] < a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float_multi(float a[][M], float b[][M], 
                                 float out[][M], int mode) {
    /* Multi-dimensional array access */
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            if (mode) {
                out[i][j] = (a[i][j] < b[i][j]) ? a[i][j] : b[i][j];
            } else {
                out[i][j] = (b[i][j] < a[i][j]) ? b[i][j] : a[i][j];
            }
        }
    }
}

/* LE_EXPR variants - with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] <= b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] - should trigger std::swap logic */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] | a[i] : b[i] & a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double_strided(double *restrict a, double *restrict b,
                                    double *restrict out, int stride, int mode) {
    /* Strided access pattern */
    for (int i = 0; i < N; i += stride) {
        if (mode) {
            out[i] = (a[i] <= b[i]) ? a[i] * 2.0 : b[i] / 2.0;
        } else {
            out[i] = (b[i] <= a[i]) ? b[i] * 2.0 : a[i] / 2.0;
        }
    }
}

/* Complex conditional with mixed comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict d,
                                    int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        /* Mix of GT, GE, LT, LE comparisons */
        if (a[i] > b[i]) {
            out[i] = c[i] + d[i];
        } else if (a[i] >= c[i]) {
            out[i] = b[i] - d[i];
        } else if (d[i] < b[i]) {  /* Swapped operands */
            out[i] = a[i] * c[i];
        } else if (c[i] <= a[i]) { /* Swapped operands */
            out[i] = b[i] / (d[i] + 1);
        } else {
            out[i] = 0;
        }
    }
}

/* Nested loops with comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_nested_comparisons(int16_t a[][8], int16_t b[][8],
                                     int16_t out[][8]) {
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? 
                       ((a[i][j] < 0) ? -a[i][j] : a[i][j]) :
                       ((b[i][j] <= 0) ? b[i][j] * 2 : b[i][j] / 2);
        }
    }
}

int main(void) {
    /* Declare arrays */
    int8_t a8[N], b8[N], out8[N];
    int16_t a16[N], b16[N], out16[N];
    int32_t a32[N], b32[N], c32[N], d32[N], out32[N];
    int64_t a64[N], b64[N], out64[N];
    float fa[N], fb[N], out_f[N];
    double da[N], db[N], out_d[N];
    
    /* Multi-dimensional arrays */
    float fa_md[N/M][M], fb_md[N/M][M], out_f_md[N/M][M];
    int16_t a16_md[N/8][8], b16_md[N/8][8], out16_md[N/8][8];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, fa, fb, da, db);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            fa_md[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            fb_md[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            a16_md[i][j] = (int16_t)(lcg_rand() % 65536 - 32768);
            b16_md[i][j] = (int16_t)(lcg_rand() % 65536 - 32768);
        }
    }
    
    /* Initialize additional arrays */
    for (int i = 0; i < N; i++) {
        c32[i] = (int32_t)lcg_rand();
        d32[i] = (int32_t)lcg_rand();
    }
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode < 2; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(fa, fb, out_f);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(da, db, out_d);
        
        /* LT_EXPR tests with mode toggling */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float_multi(fa_md, fb_md, out_f_md, mode);
        
        /* LE_EXPR tests with mode toggling */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double_strided(da, db, out_d, 2, mode);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, d32, out32);
        
        /* Nested comparisons */
        vector_nested_comparisons(a16_md, b16_md, out16_md);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)out_f[i] + (int64_t)out_d[i];
    }
    
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (int64_t)out_f_md[i][j];
        }
    }
    
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            checksum += out16_md[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
