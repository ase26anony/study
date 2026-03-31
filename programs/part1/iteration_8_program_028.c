#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

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
    for (int i = 0; i < N; i++) {
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

/* GT_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] + 1.0f : b[i] - 1.0f;
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 2 : b[i] / 2;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] >= b[i]) ? a[i] * 1.5 : b[i] * 0.5;
    }
}

/* LT_EXPR variants - with swapped operands in different modes */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] < a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            if (a[i] < b[i]) {
                out[i] = a[i] * b[i];
            } else {
                out[i] = a[i] / b[i];
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            if (b[i] < a[i]) {
                out[i] = b[i] * a[i];
            } else {
                out[i] = b[i] / a[i];
            }
        }
    }
}

/* LE_EXPR variants - with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] <= b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] - should trigger std::swap */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] | a[i] : b[i] & a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + 2.0 : b[i] - 2.0;
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + 2.0 : a[i] - 2.0;
        }
    }
}

/* Multi-dimensional array access with comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gt(int32_t a[M][M], int32_t b[M][M], int32_t out[M][M]) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

/* Complex nested conditionals with multiple comparison types */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict d,
                                    int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i] + d[i];
        } else if (a[i] >= b[i]) {
            out[i] = c[i] - d[i];
        } else if (a[i] < b[i]) {
            out[i] = c[i] * d[i];
        } else if (a[i] <= b[i]) {
            out[i] = c[i] / (d[i] ? d[i] : 1);
        } else {
            out[i] = 0;
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_ge(float *restrict a, float *restrict b,
                             float *restrict out) {
    const int stride = 4;
    for (int i = 0; i < N; i += stride) {
        for (int j = 0; j < stride && (i + j) < N; j++) {
            out[i + j] = (a[i + j] >= b[i + j]) ? a[i + j] : b[i + j];
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int8_t *a8 = malloc(N * sizeof(int8_t));
    int8_t *b8 = malloc(N * sizeof(int8_t));
    int8_t *out8 = malloc(N * sizeof(int8_t));
    
    int16_t *a16 = malloc(N * sizeof(int16_t));
    int16_t *b16 = malloc(N * sizeof(int16_t));
    int16_t *out16 = malloc(N * sizeof(int16_t));
    
    int32_t *a32 = malloc(N * sizeof(int32_t));
    int32_t *b32 = malloc(N * sizeof(int32_t));
    int32_t *c32 = malloc(N * sizeof(int32_t));
    int32_t *d32 = malloc(N * sizeof(int32_t));
    int32_t *out32 = malloc(N * sizeof(int32_t));
    int32_t *out32_mixed = malloc(N * sizeof(int32_t));
    
    int64_t *a64 = malloc(N * sizeof(int64_t));
    int64_t *b64 = malloc(N * sizeof(int64_t));
    int64_t *out64 = malloc(N * sizeof(int64_t));
    
    float *af = malloc(N * sizeof(float));
    float *bf = malloc(N * sizeof(float));
    float *outf = malloc(N * sizeof(float));
    float *outf_strided = malloc(N * sizeof(float));
    
    double *ad = malloc(N * sizeof(double));
    double *bd = malloc(N * sizeof(double));
    double *outd = malloc(N * sizeof(double));
    
    int32_t md_a[M][M], md_b[M][M], md_out[M][M];
    
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize additional arrays */
    for (int i = 0; i < N; i++) {
        c32[i] = (int32_t)rand_int();
        d32[i] = (int32_t)rand_int();
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = (int32_t)rand_int();
            md_b[i][j] = (int32_t)rand_int();
        }
    }
    
    /* Execute all comparison variants */
    vector_gt_int8(a8, b8, out8);
    vector_gt_float(af, bf, outf);
    
    vector_ge_int16(a16, b16, out16);
    vector_ge_double(ad, bd, outd);
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
    }
    
    vector_md_gt(md_a, md_b, md_out);
    vector_mixed_comparisons(a32, b32, c32, d32, out32_mixed);
    vector_strided_ge(af, bf, outf_strided);
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i] + 
                   (int64_t)outf[i] + (int64_t)outd[i] +
                   out32_mixed[i] + (int64_t)outf_strided[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += md_out[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(c32); free(d32); free(out32); free(out32_mixed);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(outf); free(outf_strided);
    free(ad); free(bd); free(outd);
    
    return 0;
}
