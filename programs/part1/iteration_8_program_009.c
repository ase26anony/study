#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define CHUNK 128

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
    for (int i = 0; i < SIZE; i++) {
        a8[i] = (int8_t)(lcg_rand() & 0xFF);
        b8[i] = (int8_t)(lcg_rand() & 0xFF);
        a16[i] = (int16_t)(lcg_rand() & 0xFFFF);
        b16[i] = (int16_t)(lcg_rand() & 0xFFFF);
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

/* GT_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict c, float *restrict d,
                           float *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] > b[i]) ? c[i] : d[i];
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* LT_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped operands - should trigger std::swap logic */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] < b[i]) ? a[i] * 2.0f : b[i] * 2.0f;
        }
    } else {
        /* Swapped operands */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] < a[i]) ? b[i] * 2.0f : a[i] * 2.0f;
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped operands */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] / 2.0 : b[i] / 2.0;
        }
    } else {
        /* Swapped operands */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] / 2.0 : a[i] / 2.0;
        }
    }
}

/* Multi-dimensional array access with comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[][CHUNK], int32_t b[][CHUNK],
                        int32_t out[][CHUNK]) {
    for (int i = 0; i < SIZE/CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d(float a[][CHUNK], float b[][CHUNK],
                        float out[][CHUNK], int mode) {
    if (mode) {
        for (int i = 0; i < SIZE/CHUNK; i++) {
            for (int j = 0; j < CHUNK; j++) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] : b[i][j];
            }
        }
    } else {
        /* Swapped operands in 2D */
        for (int i = 0; i < SIZE/CHUNK; i++) {
            for (int j = 0; j < CHUNK; j++) {
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] : a[i][j];
            }
        }
    }
}

/* Complex conditional with mixed comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        if (a[i] > b[i]) {
            out[i] = a[i] + c[i];
        } else if (a[i] < b[i]) {
            out[i] = b[i] - c[i];
        } else {
            out[i] = c[i];
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(double *restrict a, double *restrict b,
                             double *restrict out) {
    for (int i = 0; i < SIZE; i += 4) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
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
    float *cf = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    float *df = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    float *outf = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 32);
    
    double *ad = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    double *bd = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    double *outd = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 32);
    
    /* 2D arrays */
    int32_t (*a2d)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(int32_t)), 32);
    int32_t (*b2d)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(int32_t)), 32);
    int32_t (*out2d)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(int32_t)), 32);
    
    float (*af2d)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(float)), 32);
    float (*bf2d)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(float)), 32);
    float (*outf2d)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(float)), 32);
    
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize additional arrays */
    for (int i = 0; i < SIZE; i++) {
        c32[i] = (int32_t)lcg_rand();
        cf[i] = (float)(lcg_rand() % 1000) / 10.0f;
        df[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Initialize 2D arrays */
    for (int i = 0; i < SIZE/CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            a2d[i][j] = (int32_t)lcg_rand();
            b2d[i][j] = (int32_t)lcg_rand();
            af2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            bf2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    long long checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, cf, df, outf);
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        vector_gt_2d(a2d, b2d, out2d);
        vector_le_2d(af2d, bf2d, outf2d, mode);
        vector_mixed_comparisons(a32, b32, c32, out32);
        vector_ge_strided(ad, bd, outd);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += out8[i] + out16[i] + out32[i] + (long long)out64[i];
        checksum += (long long)outf[i] + (long long)outd[i];
    }
    
    for (int i = 0; i < SIZE/CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            checksum += out2d[i][j] + (long long)outf2d[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(c32); free(out32);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(cf); free(df); free(outf);
    free(ad); free(bd); free(outd);
    free(a2d); free(b2d); free(out2d);
    free(af2d); free(bf2d); free(outf2d);
    
    return 0;
}
