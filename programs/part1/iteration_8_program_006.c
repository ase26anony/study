#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define LCG_MULT 1103515245
#define LCG_INCR 12345
#define LCG_MOD 0x7fffffff

/* Simple deterministic RNG for reproducible results */
static uint32_t lcg_state = 123456789;

static uint32_t lcg_rand(void) {
    lcg_state = (LCG_MULT * lcg_state + LCG_INCR) & LCG_MOD;
    return lcg_state;
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
        a32[i] = (int32_t)lcg_rand() - 0x3fffffff;
        b32[i] = (int32_t)lcg_rand() - 0x3fffffff;
        a64[i] = (int64_t)lcg_rand() * 1000;
        b64[i] = (int64_t)lcg_rand() * 1000;
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
        /* Swapped order: b[i] < a[i] */
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
            if (a[i] < b[i]) {
                out[i] = a[i] * 2.0f;
            } else {
                out[i] = b[i] / 2.0f;
            }
        }
    } else {
        for (int i = 0; i < SIZE; i++) {
            if (b[i] < a[i]) {
                out[i] = b[i] * 3.0f;
            } else {
                out[i] = a[i] / 3.0f;
            }
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* a[i] <= b[i] */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* b[i] <= a[i] */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (mode ? (a[i] <= b[i]) : (b[i] <= a[i])) 
                 ? a[i] * b[i] : a[i] / b[i];
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[][16], int32_t b[][16], 
                        int32_t out[][16]) {
    for (int i = 0; i < SIZE/16; i++) {
        for (int j = 0; j < 16; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_strided(float *restrict a, float *restrict b,
                             float *restrict out) {
    for (int i = 0; i < SIZE; i += 4) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
        out[i+1] = (a[i+1] >= b[i+1]) ? a[i+1] * 2.0f : b[i+1];
        out[i+2] = (a[i+2] >= b[i+2]) ? a[i+2] : b[i+2] * 3.0f;
        out[i+3] = (a[i+3] >= b[i+3]) ? a[i+3] * 4.0f : b[i+3] * 0.5f;
    }
}

/* Complex ternary with multiple comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict d,
                                    int32_t *restrict out) {
    for (int i = 0; i < SIZE; i++) {
        out[i] = (a[i] > b[i]) ? c[i] : 
                ((a[i] < b[i]) ? d[i] : 
                ((a[i] >= c[i]) ? b[i] : a[i]));
    }
}

int main() {
    /* Allocate and initialize arrays */
    int8_t a8[SIZE], b8[SIZE], out8[SIZE];
    int16_t a16[SIZE], b16[SIZE], out16[SIZE];
    int32_t a32[SIZE], b32[SIZE], c32[SIZE], d32[SIZE], out32[SIZE];
    int64_t a64[SIZE], b64[SIZE], out64[SIZE];
    float af[SIZE], bf[SIZE], outf[SIZE];
    double ad[SIZE], bd[SIZE], outd[SIZE];
    
    /* Multi-dimensional arrays */
    int32_t a2d[SIZE/16][16], b2d[SIZE/16][16], out2d[SIZE/16][16];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize additional arrays */
    for (int i = 0; i < SIZE; i++) {
        c32[i] = (int32_t)lcg_rand() - 0x3fffffff;
        d32[i] = (int32_t)lcg_rand() - 0x3fffffff;
    }
    
    for (int i = 0; i < SIZE/16; i++) {
        for (int j = 0; j < 16; j++) {
            a2d[i][j] = (int32_t)lcg_rand() - 0x3fffffff;
            b2d[i][j] = (int32_t)lcg_rand() - 0x3fffffff;
        }
    }
    
    int64_t total_checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* Execute all comparison variants */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        vector_gt_2d(a2d, b2d, out2d);
        vector_ge_strided(af, bf, outf);
        vector_mixed_comparisons(a32, b32, c32, d32, out32);
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < SIZE; i++) {
            total_checksum += out8[i] + out16[i] + out32[i] + 
                            (int64_t)outf[i] + (int64_t)outd[i];
            if (i < SIZE/16) {
                for (int j = 0; j < 16; j++) {
                    total_checksum += out2d[i][j];
                }
            }
        }
    }
    
    printf("Total checksum: %ld\n", total_checksum);
    return 0;
}
