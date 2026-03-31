#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define CHUNK 128

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
    for (int i = 0; i < SIZE; i++) {
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
                           float *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + 1.0f : b[i] - 1.0f;
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 2 : b[i] / 2;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 1.5 : b[i] * 0.5;
    }
}

/* LT_EXPR variants with swapped operands based on mode */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] < b[i] */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int mode) {
    for (int i = 0; i < SIZE; i++) {
        if (mode) {
            out[i] = (a[i] < b[i]) ? a[i] * 2.0f : b[i];
        } else {
            out[i] = (b[i] < a[i]) ? b[i] * 2.0f : a[i];
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + 1 : b[i] - 1;
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + 1 : a[i] - 1;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    for (int i = 0; i < SIZE; i += 2) {
        if (mode) {
            out[i] = (a[i] <= b[i]) ? a[i] * 3.0 : b[i];
            out[i+1] = (a[i+1] <= b[i+1]) ? a[i+1] * 3.0 : b[i+1];
        } else {
            out[i] = (b[i] <= a[i]) ? b[i] * 3.0 : a[i];
            out[i+1] = (b[i+1] <= a[i+1]) ? b[i+1] * 3.0 : a[i+1];
        }
    }
}

/* Multi-dimensional array access with GT/GE comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gtge(int32_t a[][CHUNK], int32_t b[][CHUNK],
                          int32_t out[][CHUNK]) {
    for (int i = 0; i < SIZE/CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            /* Mix GT and GE in same loop */
            if (a[i][j] > b[i][j]) {
                out[i][j] = a[i][j];
            } else if (a[i][j] >= b[i][j]) {
                out[i][j] = b[i][j];
            } else {
                out[i][j] = 0;
            }
        }
    }
}

/* Complex nested conditionals with LT/LE */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_nested_ltle(int64_t *restrict a, int64_t *restrict b,
                              int64_t *restrict c, int64_t *restrict d,
                              int64_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        if (a[i] < b[i]) {
            out[i] = (c[i] <= d[i]) ? c[i] : d[i];
        } else {
            out[i] = (d[i] <= c[i]) ? d[i] : c[i];
        }
    }
}

/* Strided access pattern with comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_comp(float *restrict a, float *restrict b,
                               float *restrict out) {
    for (int i = 0; i < SIZE/4; i++) {
        int idx = i * 4;
        out[idx] = (a[idx] > b[idx]) ? a[idx] : b[idx];
        out[idx+1] = (a[idx+1] >= b[idx+1]) ? a[idx+1] : b[idx+1];
        out[idx+2] = (a[idx+2] < b[idx+2]) ? a[idx+2] : b[idx+2];
        out[idx+3] = (a[idx+3] <= b[idx+3]) ? a[idx+3] : b[idx+3];
    }
}

int main(void) {
    /* Allocate aligned memory for better vectorization */
    int8_t *a8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 16);
    int8_t *b8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 16);
    int8_t *out8 = __builtin_assume_aligned(malloc(SIZE * sizeof(int8_t)), 16);
    
    int16_t *a16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 16);
    int16_t *b16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 16);
    int16_t *out16 = __builtin_assume_aligned(malloc(SIZE * sizeof(int16_t)), 16);
    
    int32_t *a32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 16);
    int32_t *b32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 16);
    int32_t *out32 = __builtin_assume_aligned(malloc(SIZE * sizeof(int32_t)), 16);
    
    int64_t *a64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 16);
    int64_t *b64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 16);
    int64_t *c64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 16);
    int64_t *d64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 16);
    int64_t *out64 = __builtin_assume_aligned(malloc(SIZE * sizeof(int64_t)), 16);
    
    float *af = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 16);
    float *bf = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 16);
    float *outf = __builtin_assume_aligned(malloc(SIZE * sizeof(float)), 16);
    
    double *ad = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 16);
    double *bd = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 16);
    double *outd = __builtin_assume_aligned(malloc(SIZE * sizeof(double)), 16);
    
    /* Multi-dimensional arrays */
    int32_t (*amd)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(int32_t)), 16);
    int32_t (*bmd)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(int32_t)), 16);
    int32_t (*outmd)[CHUNK] = __builtin_assume_aligned(malloc((SIZE/CHUNK) * CHUNK * sizeof(int32_t)), 16);
    
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < SIZE/CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            amd[i][j] = (int32_t)lcg_rand();
            bmd[i][j] = (int32_t)lcg_rand();
        }
    }
    
    /* Initialize c64 and d64 for nested tests */
    for (int i = 0; i < SIZE; i++) {
        c64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
        d64[i] = (int64_t)lcg_rand() | ((int64_t)lcg_rand() << 32);
    }
    
    long long checksum = 0;
    
    /* Test GT_EXPR variants */
    vector_gt_int8(a8, b8, out8);
    vector_gt_float(af, bf, outf);
    
    /* Test GE_EXPR variants */
    vector_ge_int16(a16, b16, out16);
    vector_ge_double(ad, bd, outd);
    
    /* Test LT_EXPR and LE_EXPR with mode toggling for swapped operands */
    for (int mode = 0; mode <= 1; mode++) {
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
    }
    
    /* Test multi-dimensional comparisons */
    vector_md_gtge(amd, bmd, outmd);
    
    /* Test nested conditionals */
    vector_nested_ltle(a64, b64, c64, d64, out64);
    
    /* Test strided access */
    vector_strided_comp(af, bf, outf);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < SIZE/CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            checksum += outmd[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(out32);
    free(a64); free(b64); free(c64); free(d64); free(out64);
    free(af); free(bf); free(outf);
    free(ad); free(bd); free(outd);
    free(amd); free(bmd); free(outmd);
    
    return 0;
}
