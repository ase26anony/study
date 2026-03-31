#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define CHUNK 128

/* Simple deterministic pseudo-random generator */
static uint32_t seed = 123456789;
static uint32_t lcg() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < SIZE; i++) {
        a8[i] = (int8_t)(lcg() % 256 - 128);
        b8[i] = (int8_t)(lcg() % 256 - 128);
        a16[i] = (int16_t)(lcg() % 65536 - 32768);
        b16[i] = (int16_t)(lcg() % 65536 - 32768);
        a32[i] = (int32_t)lcg();
        b32[i] = (int32_t)lcg();
        a64[i] = ((int64_t)lcg() << 32) | lcg();
        b64[i] = ((int64_t)lcg() << 32) | lcg();
        af[i] = (float)(lcg() % 1000) / 10.0f;
        bf[i] = (float)(lcg() % 1000) / 10.0f;
        ad[i] = (double)(lcg() % 1000) / 10.0;
        bd[i] = (double)(lcg() % 1000) / 10.0;
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

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode, int n) {
    if (mode) {
        /* Normal order: a[i] < b[i] */
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
static void vector_lt_float_nested(float *restrict a, float *restrict b,
                                  float *restrict out, int mode, int n) {
    for (int i = 0; i < n; i++) {
        if (mode) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        } else {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode, int n) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double_nested(double *restrict a, double *restrict b,
                                   double *restrict out, int mode, int n) {
    for (int i = 0; i < n; i++) {
        if (mode) {
            out[i] = (a[i] <= b[i]) ? a[i] * 2.0 : b[i] / 2.0;
        } else {
            out[i] = (b[i] <= a[i]) ? b[i] * 2.0 : a[i] / 2.0;
        }
    }
}

/* Multi-dimensional array access */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[][CHUNK], int32_t b[][CHUNK],
                        int32_t out[][CHUNK], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d(float a[][CHUNK], float b[][CHUNK],
                        float out[][CHUNK], int mode, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (mode) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] : b[i][j];
            } else {
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] : a[i][j];
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out, int stride, int n) {
    for (int i = 0; i < n; i += stride) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* Complex expression with multiple comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out,
                                    int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT and LT in same expression */
        if (a[i] > b[i]) {
            out[i] = (c[i] < a[i]) ? c[i] : a[i];
        } else {
            out[i] = (b[i] <= c[i]) ? b[i] : c[i];
        }
    }
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int8_t *a8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 32);
    int8_t *b8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 32);
    int8_t *out8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 32);
    
    int16_t *a16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 32);
    int16_t *b16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 32);
    int16_t *out16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 32);
    
    int32_t *a32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 32);
    int32_t *b32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 32);
    int32_t *c32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 32);
    int32_t *out32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 32);
    
    int64_t *a64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 32);
    int64_t *b64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 32);
    int64_t *out64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 32);
    
    float *af = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    float *bf = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    float *outf = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    
    double *ad = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    double *bd = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    double *outd = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    
    /* 2D arrays */
    int32_t (*a2d)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(int32_t)), 32);
    int32_t (*b2d)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(int32_t)), 32);
    int32_t (*out2d)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(int32_t)), 32);
    
    float (*af2d)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(float)), 32);
    float (*bf2d)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(float)), 32);
    float (*outf2d)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(float)), 32);
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < CHUNK; j++) {
            a2d[i][j] = lcg() % 1000;
            b2d[i][j] = lcg() % 1000;
            af2d[i][j] = (float)(lcg() % 1000) / 10.0f;
            bf2d[i][j] = (float)(lcg() % 1000) / 10.0f;
        }
    }
    
    /* Initialize c32 for mixed comparisons */
    for (int i = 0; i < SIZE; i++) {
        c32[i] = lcg() % 1000;
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8, SIZE);
        vector_gt_float(af, bf, outf, SIZE);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16, SIZE);
        vector_ge_double(ad, bd, outd, SIZE);
        
        /* LT_EXPR tests with mode switching */
        vector_lt_int32(a32, b32, out32, mode, SIZE);
        vector_lt_float_nested(af, bf, outf, mode, SIZE);
        
        /* LE_EXPR tests with mode switching */
        vector_le_int64(a64, b64, out64, mode, SIZE);
        vector_le_double_nested(ad, bd, outd, mode, SIZE);
        
        /* 2D array tests */
        vector_gt_2d(a2d, b2d, out2d, 8, CHUNK);
        vector_le_2d(af2d, bf2d, outf2d, mode, 8, CHUNK);
        
        /* Strided access */
        vector_ge_strided(a32, b32, out32, 4, SIZE);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, out32, SIZE);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += out8[i] + out16[i] + out32[i] + (int64_t)outf[i];
        checksum += out64[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < CHUNK; j++) {
            checksum += out2d[i][j] + (int64_t)outf2d[i][j];
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
    free(a2d); free(b2d); free(out2d);
    free(af2d); free(bf2d); free(outf2d);
    
    return 0;
}
