#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define CHUNK 128

/* Simple deterministic pseudo-random generator */
static uint32_t seed = 123456789;
static uint32_t lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < SIZE; i++) {
        a8[i] = (int8_t)(lcg_rand() % 256 - 128);
        b8[i] = (int8_t)(lcg_rand() % 256 - 128);
        a16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        b16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        a32[i] = (int32_t)lcg_rand();
        b32[i] = (int32_t)lcg_rand();
        a64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        b64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        af[i] = (float)(lcg_rand() % 1000) / 10.0f - 50.0f;
        bf[i] = (float)(lcg_rand() % 1000) / 10.0f - 50.0f;
        ad[i] = (double)(lcg_rand() % 10000) / 100.0 - 50.0;
        bd[i] = (double)(lcg_rand() % 10000) / 100.0 - 50.0;
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
        if (a[i] >= b[i]) {
            out[i] = a[i] * 2.0;
        } else {
            out[i] = b[i] / 2.0;
        }
    }
}

/* LT_EXPR variants with swapped operands based on mode */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int n, int mode) {
    if (mode) {
        /* Original: a[i] < b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped: b[i] < a[i] - should trigger std::swap */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int n, int mode) {
    for (int i = 0; i < n; i++) {
        /* Complex expression with swapped operands */
        float cmp = mode ? (a[i] < b[i]) : (b[i] < a[i]);
        out[i] = cmp ? a[i] * b[i] : a[i] + b[i];
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int n, int mode) {
    if (mode) {
        /* Original: a[i] <= b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + 1 : b[i] - 1;
        }
    } else {
        /* Swapped: b[i] <= a[i] - should trigger std::swap */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + 1 : a[i] - 1;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int n, int mode) {
    for (int i = 0; i < n; i++) {
        double val1 = mode ? a[i] : b[i];
        double val2 = mode ? b[i] : a[i];
        out[i] = (val1 <= val2) ? val1 * val2 : val1 - val2;
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gt(int32_t a[][CHUNK], int32_t b[][CHUNK],
                        int32_t out[][CHUNK], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_le(float *restrict a, float *restrict b,
                             float *restrict out, int n, int stride) {
    for (int i = 0; i < n; i += stride) {
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
}

/* Complex nested conditional with multiple comparison types */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out,
                                    int n, int mode) {
    for (int i = 0; i < n; i++) {
        if (mode) {
            if (a[i] > b[i]) {
                out[i] = c[i] + 1;
            } else if (a[i] >= c[i]) {
                out[i] = b[i] - 1;
            } else if (b[i] < c[i]) {
                out[i] = a[i] * 2;
            } else if (c[i] <= a[i]) {
                out[i] = b[i] / 2;
            } else {
                out[i] = 0;
            }
        } else {
            /* Swapped operand versions */
            if (b[i] > a[i]) {
                out[i] = c[i] + 2;
            } else if (c[i] >= a[i]) {
                out[i] = b[i] - 2;
            } else if (c[i] < b[i]) {
                out[i] = a[i] * 3;
            } else if (a[i] <= c[i]) {
                out[i] = b[i] / 3;
            } else {
                out[i] = 1;
            }
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
    
    /* Multi-dimensional arrays */
    int32_t (*amd)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(int32_t)), 32);
    int32_t (*bmd)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(int32_t)), 32);
    int32_t (*outmd)[CHUNK] = __builtin_assume_aligned(malloc(8 * CHUNK * sizeof(int32_t)), 32);
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < CHUNK; j++) {
            amd[i][j] = lcg_rand() % 1000;
            bmd[i][j] = lcg_rand() % 1000;
        }
    }
    
    /* Initialize c32 for mixed comparisons */
    for (int i = 0; i < SIZE; i++) {
        c32[i] = (int32_t)lcg_rand();
    }
    
    int64_t checksum = 0;
    
    /* Test with different modes to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8, SIZE);
        vector_gt_float(af, bf, outf, SIZE);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16, SIZE);
        vector_ge_double(ad, bd, outd, SIZE);
        
        /* LT_EXPR tests with mode-based swapping */
        vector_lt_int32(a32, b32, out32, SIZE, mode);
        vector_lt_float(af, bf, outf, SIZE, mode);
        
        /* LE_EXPR tests with mode-based swapping */
        vector_le_int64(a64, b64, out64, SIZE, mode);
        vector_le_double(ad, bd, outd, SIZE, mode);
        
        /* Multi-dimensional test */
        vector_md_gt(amd, bmd, outmd, 8, CHUNK);
        
        /* Strided access test */
        vector_strided_le(af, bf, outf, SIZE, 4);
        
        /* Mixed comparisons test */
        vector_mixed_comparisons(a32, b32, c32, out32, SIZE, mode);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < CHUNK; j++) {
            checksum += outmd[i][j];
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
    free(amd); free(bmd); free(outmd);
    
    return 0;
}
