#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define LCG_MULT 1103515245
#define LCG_INCR 12345
#define LCG_MOD 0x7fffffff

/* Deterministic pseudo-random number generator */
static uint32_t lcg_state = 1;

static uint32_t lcg_rand(void) {
    lcg_state = (LCG_MULT * lcg_state + LCG_INCR) & LCG_MOD;
    return lcg_state;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *fa, float *fb, double *da, double *db) {
    for (int i = 0; i < SIZE; i++) {
        uint32_t r = lcg_rand();
        a8[i] = (int8_t)(r & 0xFF);
        b8[i] = (int8_t)((r >> 8) & 0xFF);
        a16[i] = (int16_t)(r & 0xFFFF);
        b16[i] = (int16_t)((r >> 16) & 0xFFFF);
        a32[i] = (int32_t)r;
        b32[i] = (int32_t)(r ^ 0xAAAAAAAA);
        a64[i] = (int64_t)r * 1000;
        b64[i] = (int64_t)(r ^ 0xAAAAAAAA) * 1000;
        fa[i] = (float)r / 1000.0f;
        fb[i] = (float)(r ^ 0xAAAAAAAA) / 1000.0f;
        da[i] = (double)r / 10000.0;
        db[i] = (double)(r ^ 0xAAAAAAAA) / 10000.0;
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
                           float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + 1.0f : b[i] - 1.0f;
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 2 : b[i] / 2;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 1.5 : b[i] * 0.5;
    }
}

/* LT_EXPR variants with potential operand swapping */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int n, int mode) {
    if (mode) {
        /* Original order: a[i] < b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] < b[i]) ? a[i] + 100 : b[i] - 100;
        }
    } else {
        /* Swapped order in condition: b[i] < a[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] < a[i]) ? a[i] + 200 : b[i] - 200;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float_nested(float *restrict a, float *restrict b,
                                  float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            out[i] = a[i] * 2.0f;
        } else if (b[i] < a[i]) {  /* Swapped operands */
            out[i] = b[i] * 3.0f;
        } else {
            out[i] = (a[i] + b[i]) / 2.0f;
        }
    }
}

/* LE_EXPR variants with operand swapping */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int n, int mode) {
    if (mode) {
        /* Original order: a[i] <= b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] << 1 : b[i] >> 1;
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? a[i] << 2 : b[i] >> 2;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double_ternary(double *restrict a, double *restrict b,
                                    double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] <= b[i]) ? 
                 ((b[i] <= a[i]) ? 0.0 : a[i]) :  /* Nested with swapped */
                 b[i];
    }
}

/* Multi-dimensional array access */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[][16], int32_t b[][16],
                        int32_t out[][16], int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 16; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
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

/* Mixed comparisons in same loop */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            out[i] = 1;
        } else if (a[i] >= b[i]) {
            out[i] = 2;
        } else if (a[i] < b[i]) {
            out[i] = 3;
        } else if (a[i] <= b[i]) {
            out[i] = 4;
        } else {
            out[i] = 0;
        }
    }
}

int main(void) {
    /* Allocate aligned memory for better vectorization */
    int8_t *a8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 32);
    int8_t *b8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 32);
    int8_t *out8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 32);
    
    int16_t *a16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 32);
    int16_t *b16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 32);
    int16_t *out16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 32);
    
    int32_t *a32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 32);
    int32_t *b32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 32);
    int32_t *out32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 32);
    
    int64_t *a64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 32);
    int64_t *b64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 32);
    int64_t *out64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 32);
    
    float *fa = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    float *fb = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    float *fout = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    
    double *da = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    double *db = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    double *dout = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    
    /* 2D arrays */
    int32_t (*a2d)[16] = __builtin_assume_aligned(malloc(SIZE/16 * 16 * sizeof(int32_t)), 32);
    int32_t (*b2d)[16] = __builtin_assume_aligned(malloc(SIZE/16 * 16 * sizeof(int32_t)), 32);
    int32_t (*out2d)[16] = __builtin_assume_aligned(malloc(SIZE/16 * 16 * sizeof(int32_t)), 32);
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, fa, fb, da, db);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < SIZE/16; i++) {
        for (int j = 0; j < 16; j++) {
            uint32_t r = lcg_rand();
            a2d[i][j] = (int32_t)r;
            b2d[i][j] = (int32_t)(r ^ 0x55555555);
        }
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode < 2; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8, SIZE);
        vector_gt_float(fa, fb, fout, SIZE);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16, SIZE);
        vector_ge_double(da, db, dout, SIZE);
        
        /* LT_EXPR tests with mode-dependent operand order */
        vector_lt_int32(a32, b32, out32, SIZE, mode);
        vector_lt_float_nested(fa, fb, fout, SIZE);
        
        /* LE_EXPR tests with mode-dependent operand order */
        vector_le_int64(a64, b64, out64, SIZE, mode);
        vector_le_double_ternary(da, db, dout, SIZE);
        
        /* Multi-dimensional test */
        vector_gt_2d(a2d, b2d, out2d, SIZE/16);
        
        /* Strided access test */
        vector_ge_strided(fa, fb, fout, SIZE, 4);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, out32, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i] + 
                   (int64_t)fout[i] + (int64_t)dout[i];
    }
    
    for (int i = 0; i < SIZE/16; i++) {
        for (int j = 0; j < 16; j++) {
            checksum += out2d[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(out32);
    free(a64); free(b64); free(out64);
    free(fa); free(fb); free(fout);
    free(da); free(db); free(dout);
    free(a2d); free(b2d); free(out2d);
    
    return 0;
}
