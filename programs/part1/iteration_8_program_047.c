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
            /* Direct comparison */
            out[i] = (a[i] < b[i]) ? a[i] * 2.0f : b[i];
        } else {
            /* Swapped in conditional */
            float x = (i % 2) ? a[i] : b[i];
            float y = (i % 2) ? b[i] : a[i];
            out[i] = (x < y) ? x : y;
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
            out[i] = (a[i] <= b[i]) ? a[i] + 100 : b[i] - 100;
        }
    } else {
        /* b[i] <= a[i] - should trigger std::swap logic */
        for (int i = 0; i < SIZE; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + 100 : a[i] - 100;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    for (int i = 0; i < SIZE; i++) {
        /* Complex expression with potential operand swapping */
        double x = (mode == 0) ? a[i] : b[i];
        double y = (mode == 0) ? b[i] : a[i];
        out[i] = (x <= y) ? x * 0.75 : y * 1.25;
    }
}

/* Multi-dimensional array access with comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gt(int32_t a[][16], int32_t b[][16], 
                        int32_t out[][16]) {
    for (int i = 0; i < SIZE/16; i++) {
        for (int j = 0; j < 16; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_le(float *restrict a, float *restrict b,
                             float *restrict out) {
    for (int i = 0; i < SIZE; i += 2) {
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
        if (i + 1 < SIZE) {
            out[i + 1] = (b[i + 1] <= a[i + 1]) ? b[i + 1] : a[i + 1];
        }
    }
}

/* Combined comparison types in one loop */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict out, int selector) {
    for (int i = 0; i < SIZE; i++) {
        switch (selector) {
            case 0:
                out[i] = (a[i] > b[i]) ? 1 : 0;  /* GT_EXPR */
                break;
            case 1:
                out[i] = (a[i] >= b[i]) ? 2 : 0; /* GE_EXPR */
                break;
            case 2:
                out[i] = (a[i] < b[i]) ? 3 : 0;  /* LT_EXPR */
                break;
            case 3:
                out[i] = (a[i] <= b[i]) ? 4 : 0; /* LE_EXPR */
                break;
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays */
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
    int32_t (*amd)[16] = malloc((SIZE/16) * 16 * sizeof(int32_t));
    int32_t (*bmd)[16] = malloc((SIZE/16) * 16 * sizeof(int32_t));
    int32_t (*outmd)[16] = malloc((SIZE/16) * 16 * sizeof(int32_t));
    
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < SIZE/16; i++) {
        for (int j = 0; j < 16; j++) {
            amd[i][j] = lcg_rand() % 1000;
            bmd[i][j] = lcg_rand() % 1000;
        }
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode < 2; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        
        /* LT_EXPR tests with mode-dependent swapping */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        
        /* LE_EXPR tests with mode-dependent swapping */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        
        /* Multi-dimensional and strided tests */
        vector_md_gt(amd, bmd, outmd);
        vector_strided_le(af, bf, outf);
        
        /* Mixed comparisons */
        for (int sel = 0; sel < 4; sel++) {
            vector_mixed_comparisons(a32, b32, out32, sel);
        }
        
        /* Compute checksum to prevent dead code elimination */
        for (int i = 0; i < SIZE; i++) {
            checksum += out8[i] + out16[i] + out32[i] + out64[i] + 
                       (int64_t)outf[i] + (int64_t)outd[i];
        }
        
        for (int i = 0; i < SIZE/16; i++) {
            for (int j = 0; j < 16; j++) {
                checksum += outmd[i][j];
            }
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
    free(amd); free(bmd); free(outmd);
    
    return 0;
}
