#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define CHUNK 128

/* Simple deterministic RNG for reproducible results */
static uint32_t seed = 123456789;
static uint32_t rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

static float rand_float(void) {
    return (float)rand_int() / (float)UINT32_MAX;
}

static double rand_double(void) {
    return (double)rand_int() / (double)UINT32_MAX;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < SIZE; i++) {
        a8[i] = (int8_t)(rand_int() % 256 - 128);
        b8[i] = (int8_t)(rand_int() % 256 - 128);
        a16[i] = (int16_t)(rand_int() % 65536 - 32768);
        b16[i] = (int16_t)(rand_int() % 65536 - 32768);
        a32[i] = (int32_t)rand_int();
        b32[i] = (int32_t)rand_int();
        a64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        b64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        af[i] = rand_float() * 1000.0f - 500.0f;
        bf[i] = rand_float() * 1000.0f - 500.0f;
        ad[i] = rand_double() * 1000.0 - 500.0;
        bd[i] = rand_double() * 1000.0 - 500.0;
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
            out[i] = (a[i] < b[i]) ? a[i] + 100 : b[i] - 100;
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] < a[i]) ? b[i] + 200 : a[i] - 200;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int mode, int n) {
    for (int i = 0; i < n; i++) {
        /* Complex expression with swapped operands based on mode */
        float cmp_val = (mode) ? 
                       ((a[i] < b[i]) ? a[i] * 2.0f : b[i] / 2.0f) :
                       ((b[i] < a[i]) ? b[i] * 3.0f : a[i] / 3.0f);
        out[i] = cmp_val;
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode, int n) {
    if (mode) {
        /* a[i] <= b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] | 0xFF : b[i] & ~0xFF;
        }
    } else {
        /* b[i] <= a[i] - should trigger std::swap logic */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] | 0xAA : a[i] & ~0xAA;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode, int n) {
    for (int i = 0; i < n; i++) {
        /* Nested conditional with swapped comparison */
        double val1 = (mode) ? a[i] : b[i];
        double val2 = (mode) ? b[i] : a[i];
        out[i] = (val1 <= val2) ? val1 + val2 : val1 - val2;
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
                             float *restrict out, int stride, int n) {
    for (int i = 0; i < n; i += stride) {
        for (int j = 0; j < stride && (i + j) < n; j++) {
            out[i + j] = (a[i + j] <= b[i + j]) ? 
                        a[i + j] * b[i + j] : a[i + j] / b[i + j];
        }
    }
}

/* Complex expression mixing comparison types */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out,
                                    int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT, GE, LT, LE in one expression */
        int32_t tmp = (a[i] > b[i]) ? a[i] : b[i];
        tmp = (tmp >= c[i]) ? tmp + c[i] : tmp - c[i];
        tmp = (a[i] < b[i]) ? tmp * 2 : tmp / 2;
        out[i] = (tmp <= (a[i] + b[i] + c[i])) ? tmp : 0;
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
            amd[i][j] = rand_int() % 1000;
            bmd[i][j] = rand_int() % 1000;
        }
    }
    
    /* Initialize c32 for mixed comparisons */
    for (int i = 0; i < SIZE; i++) {
        c32[i] = rand_int() % 1000;
    }
    
    int64_t total_checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT comparisons */
        vector_gt_int8(a8, b8, out8, SIZE);
        vector_gt_float(af, bf, outf, SIZE);
        
        /* GE comparisons */
        vector_ge_int16(a16, b16, out16, SIZE);
        vector_ge_double(ad, bd, outd, SIZE);
        
        /* LT comparisons with mode toggling */
        vector_lt_int32(a32, b32, out32, mode, SIZE);
        vector_lt_float(af, bf, outf, mode, SIZE);
        
        /* LE comparisons with mode toggling */
        vector_le_int64(a64, b64, out64, mode, SIZE);
        vector_le_double(ad, bd, outd, mode, SIZE);
        
        /* Multi-dimensional */
        vector_md_gt(amd, bmd, outmd, 8, CHUNK);
        
        /* Strided access */
        vector_strided_le(af, bf, outf, 4, SIZE);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, out32, SIZE);
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < SIZE; i++) {
            total_checksum += out8[i] + out16[i] + out32[i] + out64[i];
            total_checksum += (int64_t)outf[i] + (int64_t)outd[i];
        }
        
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < CHUNK; j++) {
                total_checksum += outmd[i][j];
            }
        }
    }
    
    printf("Total checksum: %ld\n", total_checksum);
    
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
