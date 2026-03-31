#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define LCG_MULT 1103515245
#define LCG_INCR 12345
#define LCG_MOD 0x7fffffff

/* Simple deterministic RNG for reproducible results */
static uint32_t lcg_seed = 123456789;

static inline uint32_t lcg_rand(void) {
    lcg_seed = (LCG_MULT * lcg_seed + LCG_INCR) & LCG_MOD;
    return lcg_seed;
}

/* Initialize arrays with deterministic pseudo-random values */
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
        out[i] = (a[i] > b[i]) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        if (a[i] >= b[i]) {
            out[i] = a[i] * 2;
        } else {
            out[i] = b[i] / 2;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * b[i] : a[i] / (b[i] + 1.0);
    }
}

/* LT_EXPR variants with swapped operands */
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
static void vector_lt_float_swapped(float *restrict a, float *restrict b,
                                   float *restrict out, int swap) {
    for (int i = 0; i < SIZE; i++) {
        /* Conditional swap of operands */
        float left = swap ? b[i] : a[i];
        float right = swap ? a[i] : b[i];
        out[i] = (left < right) ? left * right : left + right;
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] - should trigger std::swap logic */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double_swapped(double *restrict a, double *restrict b,
                                    double *restrict out, int swap) {
    for (int i = 0; i < SIZE; i++) {
        /* Complex expression with potentially swapped operands */
        double x = swap ? b[i] : a[i];
        double y = swap ? a[i] : b[i];
        out[i] = (x <= y) ? x * y : x / (y + 1.0);
    }
}

/* Multi-dimensional array access */
#define DIM 32
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[DIM][DIM], int32_t b[DIM][DIM],
                        int32_t out[DIM][DIM]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d(float a[DIM][DIM], float b[DIM][DIM],
                        float out[DIM][DIM], int swap) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            float left = swap ? b[i][j] : a[i][j];
            float right = swap ? a[i][j] : b[i][j];
            out[i][j] = (left <= right) ? left : right;
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out, int stride) {
    for (int i = 0; i < SIZE; i += stride) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_strided(double *restrict a, double *restrict b,
                             double *restrict out, int stride, int swap) {
    for (int i = 0; i < SIZE; i += stride) {
        double left = swap ? b[i] : a[i];
        double right = swap ? a[i] : b[i];
        out[i] = (left < right) ? left : right;
    }
}

int main(void) {
    /* Allocate arrays */
    int8_t *a8 = malloc(SIZE * sizeof(int8_t));
    int8_t *b8 = malloc(SIZE * sizeof(int8_t));
    int8_t *out8 = malloc(SIZE * sizeof(int8_t));
    
    int16_t *a16 = malloc(SIZE * sizeof(int16_t));
    int16_t *b16 = malloc(SIZE * sizeof(int16_t));
    int16_t *out16 = malloc(SIZE * sizeof(int16_t));
    
    int32_t *a32 = malloc(SIZE * sizeof(int32_t));
    int32_t *b32 = malloc(SIZE * sizeof(int32_t));
    int32_t *out32 = malloc(SIZE * sizeof(int32_t));
    
    int64_t *a64 = malloc(SIZE * sizeof(int64_t));
    int64_t *b64 = malloc(SIZE * sizeof(int64_t));
    int64_t *out64 = malloc(SIZE * sizeof(int64_t));
    
    float *af = malloc(SIZE * sizeof(float));
    float *bf = malloc(SIZE * sizeof(float));
    float *outf = malloc(SIZE * sizeof(float));
    
    double *ad = malloc(SIZE * sizeof(double));
    double *bd = malloc(SIZE * sizeof(double));
    double *outd = malloc(SIZE * sizeof(double));
    
    /* Multi-dimensional arrays */
    int32_t a2d[DIM][DIM], b2d[DIM][DIM], out2d[DIM][DIM];
    float af2d[DIM][DIM], bf2d[DIM][DIM], outf2d[DIM][DIM];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            a2d[i][j] = lcg_rand() % 1000;
            b2d[i][j] = lcg_rand() % 1000;
            af2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            bf2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    /* Toggle mode flag to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        
        /* LT_EXPR tests with swapped operands */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float_swapped(af, bf, outf, mode);
        
        /* LE_EXPR tests with swapped operands */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double_swapped(ad, bd, outd, mode);
        
        /* 2D array tests */
        vector_gt_2d(a2d, b2d, out2d);
        vector_le_2d(af2d, bf2d, outf2d, mode);
        
        /* Strided access tests */
        vector_ge_strided(a32, b32, out32, 2);
        vector_lt_strided(ad, bd, outd, 3, mode);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            checksum += out2d[i][j] + (int64_t)outf2d[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(out32);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(outf);
    free(ad); free(bd); free(outd);
    
    return 0;
}
