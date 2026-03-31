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

/* LT_EXPR variants - with potential operand swapping */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_int32(int32_t *restrict a, int32_t *restrict b,
                           int32_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] < b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] < b[i]) ? a[i] + b[i] : a[i] - b[i];
        }
    } else {
        /* Swapped order: b[i] < a[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] < a[i]) ? b[i] + a[i] : b[i] - a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_lt_float(float *restrict a, float *restrict b,
                           float *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        /* Complex expression to force tree analysis */
        float temp = (mode ? (a[i] < b[i]) : (b[i] < a[i])) 
                     ? a[i] * b[i] 
                     : a[i] / (b[i] + 1.0f);
        out[i] = temp;
    }
}

/* LE_EXPR variants - with operand swapping */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_int64(int64_t *restrict a, int64_t *restrict b,
                           int64_t *restrict out, int mode) {
    if (mode) {
        /* Original order: a[i] <= b[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (a[i] <= b[i]) ? a[i] | b[i] : a[i] & b[i];
        }
    } else {
        /* Swapped order: b[i] <= a[i] */
        for (int i = 0; i < N; i++) {
            out[i] = (b[i] <= a[i]) ? b[i] | a[i] : b[i] & a[i];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_le_double(double *restrict a, double *restrict b,
                            double *restrict out, int mode) {
    for (int i = 0; i < N; i++) {
        /* Nested conditional with swapped operands */
        if (mode) {
            out[i] = (a[i] <= b[i]) ? a[i] : b[i];
        } else {
            out[i] = (b[i] <= a[i]) ? b[i] : a[i];
        }
    }
}

/* Multi-dimensional array comparison */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_gt(int32_t a[][M], int32_t b[][M], int32_t out[][M]) {
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            out[i][j] = (a[i][j] > b[i][j]) ? a[i][j] : b[i][j];
        }
    }
}

__attribute__((optimize("O3", "tree-vectorize")))
static void vector_md_le(float a[][M], float b[][M], float out[][M], int mode) {
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            if (mode) {
                out[i][j] = (a[i][j] <= b[i][j]) ? a[i][j] * 2.0f : b[i][j] / 2.0f;
            } else {
                out[i][j] = (b[i][j] <= a[i][j]) ? b[i][j] * 2.0f : a[i][j] / 2.0f;
            }
        }
    }
}

/* Strided access pattern */
__attribute__((optimize("O3", "tree-vectorize")))
static void vector_strided_ge(int32_t *restrict a, int32_t *restrict b,
                             int32_t *restrict out) {
    const int stride = 4;
    for (int i = 0; i < N; i += stride) {
        out[i] = (a[i] >= b[i]) ? a[i] : b[i];
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int8_t *a8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *b8 = __builtin_alloca(N * sizeof(int8_t));
    int8_t *out8 = __builtin_alloca(N * sizeof(int8_t));
    
    int16_t *a16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *b16 = __builtin_alloca(N * sizeof(int16_t));
    int16_t *out16 = __builtin_alloca(N * sizeof(int16_t));
    
    int32_t *a32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *b32 = __builtin_alloca(N * sizeof(int32_t));
    int32_t *out32 = __builtin_alloca(N * sizeof(int32_t));
    
    int64_t *a64 = __builtin_alloca(N * sizeof(int64_t));
    int64_t *b64 = __builtin_alloca(N * sizeof(int64_t));
    int64_t *out64 = __builtin_alloca(N * sizeof(int64_t));
    
    float *af = __builtin_alloca(N * sizeof(float));
    float *bf = __builtin_alloca(N * sizeof(float));
    float *outf = __builtin_alloca(N * sizeof(float));
    
    double *ad = __builtin_alloca(N * sizeof(double));
    double *bd = __builtin_alloca(N * sizeof(double));
    double *outd = __builtin_alloca(N * sizeof(double));
    
    /* Multi-dimensional arrays */
    int32_t md_a[N/M][M];
    int32_t md_b[N/M][M];
    int32_t md_out[N/M][M];
    
    float md_af[N/M][M];
    float md_bf[N/M][M];
    float md_outf[N/M][M];
    
    init_arrays(a8, b8, a16, b16, a32, b32, a64, b64, af, bf, ad, bd);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            md_a[i][j] = (int32_t)rand_int();
            md_b[i][j] = (int32_t)rand_int();
            md_af[i][j] = rand_float() * 100.0f;
            md_bf[i][j] = rand_float() * 100.0f;
        }
    }
    
    int64_t checksum = 0;
    
    /* Toggle mode to trigger swapped operand paths */
    for (int mode = 0; mode <= 1; mode++) {
        /* GT_EXPR tests */
        vector_gt_int8(a8, b8, out8);
        vector_gt_float(af, bf, outf);
        
        /* GE_EXPR tests */
        vector_ge_int16(a16, b16, out16);
        vector_ge_double(ad, bd, outd);
        
        /* LT_EXPR tests with mode-dependent operand order */
        vector_lt_int32(a32, b32, out32, mode);
        vector_lt_float(af, bf, outf, mode);
        
        /* LE_EXPR tests with mode-dependent operand order */
        vector_le_int64(a64, b64, out64, mode);
        vector_le_double(ad, bd, outd, mode);
        
        /* Multi-dimensional tests */
        vector_md_gt(md_a, md_b, md_out);
        vector_md_le(md_af, md_bf, md_outf, mode);
        
        /* Strided access test */
        vector_strided_ge(a32, b32, out32);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out8[i] + out16[i] + out32[i] + out64[i];
        checksum += (int64_t)outf[i] + (int64_t)outd[i];
    }
    
    for (int i = 0; i < N/M; i++) {
        for (int j = 0; j < M; j++) {
            checksum += md_out[i][j] + (int64_t)md_outf[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
