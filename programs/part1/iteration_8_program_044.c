#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Deterministic pseudo-random generator */
static uint32_t seed = 123456789;
static uint32_t lcg() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int8_t *a8, int8_t *b8, int16_t *a16, int16_t *b16,
                       int32_t *a32, int32_t *b32, int64_t *a64, int64_t *b64,
                       float *af, float *bf, double *ad, double *bd) {
    for (int i = 0; i < N; i++) {
        a8[i] = (int8_t)(lcg() % 256 - 128);
        b8[i] = (int8_t)(lcg() % 256 - 128);
        a16[i] = (int16_t)(lcg() % 65536 - 32768);
        b16[i] = (int16_t)(lcg() % 65536 - 32768);
        a32[i] = (int32_t)lcg();
        b32[i] = (int32_t)lcg();
        a64[i] = ((int64_t)lcg() << 32) | lcg();
        b64[i] = ((int64_t)lcg() << 32) | lcg();
        af[i] = (float)(lcg() % 1000) / 10.0f;
        bf[i] = (float)(lcg() % 1000) / 10.0f;
        ad[i] = (double)(lcg() % 1000) / 10.0;
        bd[i] = (double)(lcg() % 1000) / 10.0;
    }
}

/* GT_EXPR variants with different data types */
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

/* LT_EXPR variants with swapped operands based on mode */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int n, int mode) {
    if (mode) {
        /* Normal order: a[i] < b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] < b[i]) ? a[i] : b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] < a[i]) ? b[i] : a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int n, int mode) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] < b[i]) ? a[i] * 2.0f : b[i] / 2.0f;
        }
    } else {
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] < a[i]) ? b[i] * 2.0f : a[i] / 2.0f;
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int n, int mode) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int n, int mode) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] * b[i] : a[i] / b[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] * a[i] : b[i] / a[i];
        }
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gt(int32_t a[][M], int32_t b[][M], 
                        int32_t out[][M], int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_le(float a[][M], float b[][M],
                        float out[][M], int rows, int mode) {
    if (mode) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] : b[i][j];
            }
        }
    } else {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < M; j++) {
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] : a[i][j];
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_ge(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out, int n, int stride) {
    for (int i = 0; i < n; i += stride) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

/* Complex conditional with mixed comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict out,
                                    int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i] * 2;
        } else if (a[i] >= b[i] - 10) {
            out[i] = c[i] + 5;
        } else if (b[i] < a[i]) {  /* Swapped operands */
            out[i] = c[i] - 3;
        } else if (b[i] <= a[i] + 10) {  /* Swapped operands */
            out[i] = c[i] / 2;
        } else {
            out[i] = c[i];
        }
    }
}

int main() {
    /* Allocate arrays */
    int8_t *a8 = malloc(N * sizeof(int8_t));
    int8_t *b8 = malloc(N * sizeof(int8_t));
    int8_t *out8 = malloc(N * sizeof(int8_t));
    
    int16_t *a16 = malloc(N * sizeof(int16_t));
    int16_t *b16 = malloc(N * sizeof(int16_t));
    int16_t *out16 = malloc(N * sizeof(int16_t));
    
    int32_t *a32 = malloc(N * sizeof(int32_t));
    int32_t *b32 = malloc(N * sizeof(int32_t));
    int32_t *out32 = malloc(N * sizeof(int32_t));
    int32_t *c32 = malloc(N * sizeof(int32_t));
    int32_t *out_mixed = malloc(N * sizeof(int32_t));
    
    int64_t *a64 = malloc(N * sizeof(int64_t));
    int64_t *b64 = malloc(N * sizeof(int64_t));
    int64_t *out64 = malloc(N * sizeof(int64_t));
    
    float *af = malloc(N * sizeof(float));
    float *bf = malloc(N * sizeof(float));
    float *outf = malloc(N * sizeof(float));
    
    double *ad = malloc(N * sizeof(double));
    double *bd = malloc(N * sizeof(double));
    double *outd = malloc(N * sizeof(double));
    
    /* Multi-dimensional arrays */
    int rows = N / M;
    int32_t (*md_a)[M] = malloc(rows * M * sizeof(int32_t));
    int32_t (*md_b)[M] = malloc(rows * M * sizeof(int32_t));
    int32_t (*md_out)[M] = malloc(rows * M * sizeof(int32_t));
    
    float (*md_af)[M] = malloc(rows * M * sizeof(float));
    float (*md_bf)[M] = malloc(rows * M * sizeof(float));
    float (*md_outf)[M] = malloc(rows * M * sizeof(float));
    
    /* Initialize arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = lcg() % 1000;
            md_b[i][j] = lcg() % 1000;
            md_af[i][j] = (float)(lcg() % 1000) / 10.0f;
            md_bf[i][j] = (float)(lcg() % 1000) / 10.0f;
        }
    }
    
    /* Initialize c32 for mixed comparisons */
    for (int i = 0; i < N; i++) {
        c32[i] = lcg() % 1000;
    }
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8, N);
        vector_gt_float(af, bf, outf, N);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16, N);
        vector_ge_double(ad, bd, outd, N);
        
        /* LT_EXPR tests with mode toggling */
        vector_lt_int32(a32, b32, out32, N, mode);
        vector_lt_float(af, bf, outf, N, mode);
        
        /* LE_EXPR tests with mode toggling */
        vector_le_int64(a64, b64, out64, N, mode);
        vector_le_double(ad, bd, outd, N, mode);
        
        /* Multi-dimensional tests */
        vector_md_gt(md_a, md_b, md_out, rows);
        vector_md_le(md_af, md_bf, md_outf, rows, mode);
        
        /* Strided access */
        vector_strided_ge(a32, b32, out32, N, 4);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a32, b32, c32, out_mixed, N);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + (int64_t)outf[i];
        checksum += out64[i] + (int64_t)outd[i] + out_mixed[i];
    }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < M; j++) {
            checksum += md_out[i][j] + (int64_t)md_outf[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Cleanup */
    free(a8); free(b8); free(out8);
    free(a16); free(b16); free(out16);
    free(a32); free(b32); free(out32); free(c32); free(out_mixed);
    free(a64); free(b64); free(out64);
    free(af); free(bf); free(outf);
    free(ad); free(bd); free(outd);
    free(md_a); free(md_b); free(md_out);
    free(md_af); free(md_bf); free(md_outf);
    
    return 0;
}
