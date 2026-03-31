#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32

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
    for (int i = 0; i < N; i++) {
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

/* GT_EXPR variants with different data types */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];  // GT_EXPR with ternary
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  // GT_EXPR in if condition
            out[i] = a[i] * 2.0f;
        } else {
            out[i] = b[i] * 0.5f;
        }
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] + b[i] : a[i] - b[i];  // GE_EXPR
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        // GE_EXPR with swapped operands in different contexts
        out[i] = (b[i] >= a[i]) ? a[i] : b[i];
    }
}

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            // Direct LT_EXPR
            out[i] = (a[i] < b[i]) ? -a[i] : b[i];
        } else {
            // Swapped operands: should trigger std::swap(cond_expr0, cond_expr1)
            out[i] = (b[i] < a[i]) ? -b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float_nested(float *restrict a, float *restrict b,
                                  float *restrict c, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        // Complex expression with LT_EXPR
        out[i] = (a[i] < b[i]) ? 
                 ((c[i] < a[i]) ? c[i] : a[i]) :
                 ((b[i] < c[i]) ? b[i] : c[i]);
    }
}

/* LE_EXPR variants with operand swapping */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        if (mode) {
            // Direct LE_EXPR
            out[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
        } else {
            // Swapped operands
            out[i] = (b[i] <= a[i]) ? b[i] | a[i] : b[i] & a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double_strided(double *restrict a, double *restrict b,
                                    double *restrict out, int n) {
    // Strided access pattern
    for (int i = 0; i < n; i += 2) {
        out[i] = (a[i] <= b[i]) ? a[i] * b[i] : a[i] / b[i];
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d_swapped(float a[M][M], float b[M][M], float out[M][M], int mode) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (mode) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] : b[i][j];
            } else {
                // Swapped operands in 2D
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] : a[i][j];
            }
        }
    }
}

/* Mixed comparisons in single loop */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        // Mix GT, GE, LT, LE in same loop
        if (a[i] > b[i]) {
            out[i] = c[i] + 1;
        } else if (a[i] >= c[i]) {
            out[i] = b[i] - 1;
        } else if (b[i] < c[i]) {
            out[i] = a[i] * 2;
        } else if (a[i] <= b[i]) {
            out[i] = c[i] / 2;
        } else {
            out[i] = 0;
        }
    }
}

int main(void) {
    /* Allocate arrays */
    int8_t *a8 = malloc(N * sizeof(int8_t));
    int8_t *b8 = malloc(N * sizeof(int8_t));
    int8_t *out8 = malloc(N * sizeof(int8_t));
    
    int16_t *a16 = malloc(N * sizeof(int16_t));
    int16_t *b16 = malloc(N * sizeof(int16_t));
    int16_t *out16 = malloc(N * sizeof(int16_t));
    
    int32_t *a32 = malloc(N * sizeof(int32_t));
    int32_t *b32 = malloc(N * sizeof(int32_t));
    int32_t *c32 = malloc(N * sizeof(int32_t));
    int32_t *out32 = malloc(N * sizeof(int32_t));
    
    int64_t *a64 = malloc(N * sizeof(int64_t));
    int64_t *b64 = malloc(N * sizeof(int64_t));
    int64_t *out64 = malloc(N * sizeof(int64_t));
    
    float *af = malloc(N * sizeof(float));
    float *bf = malloc(N * sizeof(float));
    float *cf = malloc(N * sizeof(float));
    float *outf = malloc(N * sizeof(float));
    
    double *ad = malloc(N * sizeof(double));
    double *bd = malloc(N * sizeof(double));
    double *outd = malloc(N * sizeof(double));
    
    /* 2D arrays */
    int32_t arr_a[M][M], arr_b[M][M], arr_out[M][M];
    float farr_a[M][M], farr_b[M][M], farr_out[M][M];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr_a[i][j] = lcg_rand() % 1000;
            arr_b[i][j] = lcg_rand() % 1000;
            farr_a[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            farr_b[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    /* Initialize c arrays */
    for (int i = 0; i < N; i++) {
        c32[i] = lcg_rand();
        cf[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8, N);
        vector_gt_float(af, bf, outf, N);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16, N);
        vector_ge_double(ad, bd, outd, N);
        
        /* LT_EXPR tests with mode switching */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float_nested(af, bf, cf, outf, N);
        
        /* LE_EXPR tests with mode switching */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double_strided(ad, bd, outd, N);
        
        /* 2D array tests */
        vector_gt_2d(arr_a, arr_b, arr_out);
        vector_le_2d_swapped(farr_a, farr_b, farr_out, mode);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, out32, N);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr_out[i][j] + (int64_t)farr_out[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(c32); free(out32);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(cf); free(outf);
    free(ad); free(bd); free(outd);
    
    return 0;
}
