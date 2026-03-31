#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define SUB_SIZE 32

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
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] < a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] < b[i]) ? a[i] * 2.0f : b[i] / 2.0f;
        }
    } else {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] < a[i]) ? b[i] * 2.0f : a[i] / 2.0f;
        }
    }
}

/* LE_EXPR variants with swapped operands based on mode */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] | 0xFF : b[i] & ~0xFF;
        }
    } else {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] | 0xFF : a[i] & ~0xFF;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + 10.0 : b[i] - 10.0;
        }
    } else {
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + 10.0 : a[i] - 10.0;
        }
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[][SUB_SIZE], int32_t b[][SUB_SIZE],
                        int32_t out[][SUB_SIZE]) {
    for (int i = 0; i < SIZE/SUB_SIZE; i++) {
        for (int j = 0; j < SUB_SIZE; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d(float a[][SUB_SIZE], float b[][SUB_SIZE],
                        float out[][SUB_SIZE], int mode) {
    if (mode) {
        for (int i = 0; i < SIZE/SUB_SIZE; i++) {
            for (int j = 0; j < SUB_SIZE; j++) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] : b[i][j];
            }
        }
    } else {
        for (int i = 0; i < SIZE/SUB_SIZE; i++) {
            for (int j = 0; j < SUB_SIZE; j++) {
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] : a[i][j];
            }
        }
    }
}

/* Complex nested conditional with multiple comparison types */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        if (a[i] > b[i]) {
            out[i] = a[i] + c[i];
        } else if (a[i] >= b[i]) {
            out[i] = a[i] - c[i];
        } else if (a[i] < b[i]) {
            out[i] = b[i] + c[i];
        } else if (a[i] <= b[i]) {
            out[i] = b[i] - c[i];
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
    int32_t (*a2d)[SUB_SIZE] = __builtin_assume_aligned(
        malloc((SIZE/SUB_SIZE) * SUB_SIZE * sizeof(int32_t)), 32);
    int32_t (*b2d)[SUB_SIZE] = __builtin_assume_aligned(
        malloc((SIZE/SUB_SIZE) * SUB_SIZE * sizeof(int32_t)), 32);
    int32_t (*out2d)[SUB_SIZE] = __builtin_assume_aligned(
        malloc((SIZE/SUB_SIZE) * SUB_SIZE * sizeof(int32_t)), 32);
    
    float (*af2d)[SUB_SIZE] = __builtin_assume_aligned(
        malloc((SIZE/SUB_SIZE) * SUB_SIZE * sizeof(float)), 32);
    float (*bf2d)[SUB_SIZE] = __builtin_assume_aligned(
        malloc((SIZE/SUB_SIZE) * SUB_SIZE * sizeof(float)), 32);
    float (*outf2d)[SUB_SIZE] = __builtin_assume_aligned(
        malloc((SIZE/SUB_SIZE) * SUB_SIZE * sizeof(float)), 32);
    
    if (!a8 || !b8 || !out8 || !a16 || !b16 || !out16 || !a32 || !b32 || !c32 || 
        !out32 || !a64 || !b64 || !out64 || !af || !bf || !outf || !ad || !bd || 
        !outd || !a2d || !b2d || !out2d || !af2d || !bf2d || !outf2d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < SIZE/SUB_SIZE; i++) {
        for (int j = 0; j < SUB_SIZE; j++) {
            a2d[i][j] = (int32_t)lcg_rand();
            b2d[i][j] = (int32_t)lcg_rand();
            af2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            bf2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    /* Initialize c32 array */
    for (int i = 0; i < SIZE; i++) {
        c32[i] = (int32_t)lcg_rand();
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        vector_gt_2d(a2d, b2d, out2d);
        vector_le_2d(af2d, bf2d, outf2d, mode);
        vector_mixed_comparisons(a32, b32, c32, out32);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i] + 
                   (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < SIZE/SUB_SIZE; i++) {
        for (int j = 0; j < SUB_SIZE; j++) {
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
