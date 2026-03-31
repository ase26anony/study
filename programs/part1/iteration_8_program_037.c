#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 1024
#define M 32

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
    for (int i = 0; i < N; i++) {
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

/* GT_EXPR variants with different data types */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] > b[i] ? a[i] : b[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] > b[i] ? a[i] + b[i] : a[i] - b[i];
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] * 2 : b[i] * 3;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i] ? a[i] : -b[i];
    }
}

/* LT_EXPR variants - direct */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] < b[i] ? a[i] + 1 : b[i] - 1;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            out[i] = a[i] * 2.0f;
        } else {
            out[i] = b[i] / 2.0f;
        }
    }
}

/* LE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] <= b[i] ? a[i] | 0xFF : b[i] & 0xFFFF;
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] <= b[i] ? a[i] * b[i] : a[i] / b[i];
    }
}

/* Swapped operand variants to trigger std::swap logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_swapped(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] < b[i] ? i : -i;
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = b[i] < a[i] ? i * 2 : -i * 2;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_swapped(float *restrict a, float *restrict b,
                             float *restrict out, int mode) {
    if (mode) {
        for (int i = 0; i < N; i++) {
            out[i] = a[i] <= b[i] ? a[i] : b[i];
        }
    } else {
        for (int i = 0; i < N; i++) {
            out[i] = b[i] <= a[i] ? b[i] * 2.0f : a[i] * 3.0f;
        }
    }
}

/* Multi-dimensional array comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_2d(int32_t a[][M], int32_t b[][M], int32_t out[][M]) {
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] > b[i][j] ? 1 : 0;
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_2d(float a[][M], float b[][M], float out[][M]) {
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = a[i][j] <= b[i][j] ? a[i][j] + b[i][j] : a[i][j] - b[i][j];
        }
    }
}

/* Complex nested conditionals */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_complex_mixed(int16_t *restrict a, int16_t *restrict b,
                                int16_t *restrict c, int16_t *restrict d,
                                int16_t *restrict out) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i] + d[i];
        } else if (a[i] >= b[i] - 10) {
            out[i] = c[i] - d[i];
        } else if (a[i] < b[i]) {
            out[i] = c[i] * d[i];
        } else if (a[i] <= b[i] + 10) {
            out[i] = c[i] / (d[i] ? d[i] : 1);
        } else {
            out[i] = 0;
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int8_t a8[N], b8[N], out8[N];
    int16_t a16[N], b16[N], out16[N];
    int32_t a32[N], b32[N], out32[N];
    int64_t a64[N], b64[N], out64[N];
    float af[N], bf[N], outf[N];
    double ad[N], bd[N], outd[N];
    
    /* Multi-dimensional arrays */
    int32_t a32_2d[N/M][M], b32_2d[N/M][M], out32_2d[N/M][M];
    float af_2d[N/M][M], bf_2d[N/M][M], outf_2d[N/M][M];
    
    /* Additional arrays for complex tests */
    int16_t c16[N], d16[N];
    
    /* Initialize all arrays */
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            a32_2d[i][j] = (int32_t)lcg_rand();
            b32_2d[i][j] = (int32_t)lcg_rand();
            af_2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
            bf_2d[i][j] = (float)(lcg_rand() % 1000) / 10.0f;
        }
    }
    
    /* Initialize additional arrays */
    for (int i = 0; i < N; i++) {
        c16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
        d16[i] = (int16_t)(lcg_rand() % 65536 - 32768);
    }
    
    /* Execute all comparison variants */
    vector_gt_int8(a8, b8, out8);
    vector_gt_float(af, bf, outf);
    vector_ge_int16(a16, b16, out16);
    vector_ge_double(ad, bd, outd);
    vector_lt_int32(a32, b32, out32);
    vector_lt_float(af, bf, outf);
    vector_le_int64(a64, b64, out64);
    vector_le_double(ad, bd, outd);
    
    /* Execute swapped operand variants with mode toggle */
    for (int mode = 0; mode < 2; mode++) {
        vector_lt_swapped(a32, b32, out32, mode);
        vector_le_swapped(af, bf, outf, mode);
    }
    
    /* Execute multi-dimensional variants */
    vector_gt_2d(a32_2d, b32_2d, out32_2d);
    vector_le_2d(af_2d, bf_2d, outf_2d);
    
    /* Execute complex mixed conditional */
    vector_complex_mixed(a16, b16, c16, d16, out16);
    
    /* Compute checksum to prevent dead code elimination */
    int64_t checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += out32_2d[i][j] + (int64_t)outf_2d[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
