#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 1024
#define CHUNK 128

/* Deterministic pseudo-random generator */
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
static void init_arrays(void) {
    for (int i = 0; i < SIZE; i++) {
        a_int8[i] = (int8_t)(rand_int() % 256 - 128);
        b_int8[i] = (int8_t)(rand_int() % 256 - 128);
        c_int8[i] = (int8_t)(rand_int() % 256 - 128);
        d_int8[i] = (int8_t)(rand_int() % 256 - 128);
        
        a_int16[i] = (int16_t)(rand_int() % 65536 - 32768);
        b_int16[i] = (int16_t)(rand_int() % 65536 - 32768);
        c_int16[i] = (int16_t)(rand_int() % 65536 - 32768);
        d_int16[i] = (int16_t)(rand_int() % 65536 - 32768);
        
        a_int32[i] = (int32_t)rand_int();
        b_int32[i] = (int32_t)rand_int();
        c_int32[i] = (int32_t)rand_int();
        d_int32[i] = (int32_t)rand_int();
        
        a_int64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        b_int64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        c_int64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        d_int64[i] = (int64_t)rand_int() | ((int64_t)rand_int() << 32);
        
        a_float[i] = rand_float() * 1000.0f - 500.0f;
        b_float[i] = rand_float() * 1000.0f - 500.0f;
        c_float[i] = rand_float() * 1000.0f - 500.0f;
        d_float[i] = rand_float() * 1000.0f - 500.0f;
        
        a_double[i] = rand_double() * 1000.0 - 500.0;
        b_double[i] = rand_double() * 1000.0 - 500.0;
        c_double[i] = rand_double() * 1000.0 - 500.0;
        d_double[i] = rand_double() * 1000.0 - 500.0;
    }
}

/* Global arrays */
static int8_t a_int8[SIZE], b_int8[SIZE], c_int8[SIZE], d_int8[SIZE], out_int8[SIZE];
static int16_t a_int16[SIZE], b_int16[SIZE], c_int16[SIZE], d_int16[SIZE], out_int16[SIZE];
static int32_t a_int32[SIZE], b_int32[SIZE], c_int32[SIZE], d_int32[SIZE], out_int32[SIZE];
static int64_t a_int64[SIZE], b_int64[SIZE], c_int64[SIZE], d_int64[SIZE], out_int64[SIZE];
static float a_float[SIZE], b_float[SIZE], c_float[SIZE], d_float[SIZE], out_float[SIZE];
static double a_double[SIZE], b_double[SIZE], c_double[SIZE], d_double[SIZE], out_double[SIZE];

/* GT_EXPR variants with __attribute__((optimize)) */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_int8(int8_t *restrict a, int8_t *restrict b, 
                          int8_t *restrict c, int8_t *restrict d,
                          int8_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] > b[i] ? c[i] : d[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_float(float *restrict a, float *restrict b,
                           float *restrict c, float *restrict d,
                           float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] > b[i] ? c[i] : d[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_double(double *restrict a, double *restrict b,
                            double *restrict c, double *restrict d,
                            double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] > b[i] ? c[i] : d[i];
    }
}

/* GE_EXPR variants */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int16(int16_t *restrict a, int16_t *restrict b,
                           int16_t *restrict c, int16_t *restrict d,
                           int16_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] >= b[i] ? c[i] : d[i];
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_ge_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict c, int32_t *restrict d,
                           int32_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] >= b[i] ? c[i] : d[i];
    }
}

/* LT_EXPR variants with swapped operands logic */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict c, int64_t *restrict d,
                           int64_t *restrict out, int n, int mode) {
    if (mode) {
        /* Normal order: a[i] < b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = a[i] < b[i] ? c[i] : d[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] - should trigger std::swap logic */
        for (int i = 0; i < n; i++) {
            out[i] = b[i] < a[i] ? c[i] : d[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict c, float *restrict d,
                           float *restrict out, int n, int mode) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            out[i] = a[i] < b[i] ? c[i] : d[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            out[i] = b[i] < a[i] ? c[i] : d[i];
        }
    }
}

/* LE_EXPR variants with swapped operands */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict c, int32_t *restrict d,
                           int32_t *restrict out, int n, int mode) {
    if (mode) {
        /* Normal order: a[i] <= b[i] */
        for (int i = 0; i < n; i++) {
            out[i] = a[i] <= b[i] ? c[i] : d[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < n; i++) {
            out[i] = b[i] <= a[i] ? c[i] : d[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict c, double *restrict d,
                            double *restrict out, int n, int mode) {
    if (mode) {
        for (int i = 0; i < n; i++) {
            out[i] = a[i] <= b[i] ? c[i] : d[i];
        }
    } else {
        for (int i = 0; i < n; i++) {
            out[i] = b[i] <= a[i] ? c[i] : d[i];
        }
    }
}

/* Multi-dimensional array comparison */
static int md_a[CHUNK][CHUNK], md_b[CHUNK][CHUNK], md_c[CHUNK][CHUNK], md_d[CHUNK][CHUNK], md_out[CHUNK][CHUNK];

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_gt_multi_dim(int n, int m, int mode) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (mode) {
                md_out[i][j] = md_a[i][j] > md_b[i][j] ? md_c[i][j] : md_d[i][j];
            } else {
                md_out[i][j] = md_b[i][j] > md_a[i][j] ? md_c[i][j] : md_d[i][j];
            }
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_multi_dim(int n, int m, int mode) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (mode) {
                md_out[i][j] = md_a[i][j] <= md_b[i][j] ? md_c[i][j] : md_d[i][j];
            } else {
                md_out[i][j] = md_b[i][j] <= md_a[i][j] ? md_c[i][j] : md_d[i][j];
            }
        }
    }
}

/* Complex nested conditional with mixed comparisons */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_mixed_comparisons(int32_t *restrict a, int32_t *restrict b,
                                    int32_t *restrict c, int32_t *restrict d,
                                    int32_t *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            out[i] = c[i];
        } else if (a[i] >= b[i] - 10) {
            out[i] = d[i];
        } else if (a[i] < b[i]) {
            out[i] = c[i] + d[i];
        } else if (a[i] <= b[i] + 10) {
            out[i] = c[i] - d[i];
        } else {
            out[i] = 0;
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_lt(float *restrict a, float *restrict b,
                             float *restrict out, int n, int stride) {
    for (int i = 0; i < n; i += stride) {
        out[i] = a[i] < b[i] ? a[i] : b[i];
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(void) {
    int64_t sum = 0;
    
    for (int i = 0; i < SIZE; i++) {
        sum += out_int8[i] + out_int16[i] + out_int32[i] + out_int64[i];
        sum += (int64_t)out_float[i] + (int64_t)out_double[i];
    }
    
    for (int i = 0; i < CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            sum += md_out[i][j];
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize all arrays */
    init_arrays();
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < CHUNK; i++) {
        for (int j = 0; j < CHUNK; j++) {
            md_a[i][j] = rand_int() % 1000;
            md_b[i][j] = rand_int() % 1000;
            md_c[i][j] = rand_int() % 1000;
            md_d[i][j] = rand_int() % 1000;
        }
    }
    
    /* Toggle mode flag to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a_int8, b_int8, c_int8, d_int8, out_int8, SIZE);
        vector_gt_float(a_float, b_float, c_float, d_float, out_float, SIZE);
        vector_gt_double(a_double, b_double, c_double, d_double, out_double, SIZE);
        
        /* GE_EXPR tests */
        vector_ge_int16(a_int16, b_int16, c_int16, d_int16, out_int16, SIZE);
        vector_ge_int32(a_int32, b_int32, c_int32, d_int32, out_int32, SIZE);
        
        /* LT_EXPR tests with mode toggling */
        vector_lt_int64(a_int64, b_int64, c_int64, d_int64, out_int64, SIZE, mode);
        vector_lt_float(a_float, b_float, c_float, d_float, out_float, SIZE, mode);
        
        /* LE_EXPR tests with mode toggling */
        vector_le_int32(a_int32, b_int32, c_int32, d_int32, out_int32, SIZE, mode);
        vector_le_double(a_double, b_double, c_double, d_double, out_double, SIZE, mode);
        
        /* Multi-dimensional tests */
        vector_gt_multi_dim(CHUNK, CHUNK, mode);
        vector_le_multi_dim(CHUNK, CHUNK, mode);
        
        /* Mixed comparisons */
        vector_mixed_comparisons(a_int32, b_int32, c_int32, d_int32, out_int32, SIZE);
        
        /* Strided access */
        vector_strided_lt(a_float, b_float, out_float, SIZE, 2);
    }
    
    /* Compute and print checksum */
    int64_t checksum = compute_checksum();
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
