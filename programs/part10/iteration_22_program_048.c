/* test_vector_builtin.c
 * Test program to cover vectorized built-in function declaration flags
 * in GCC's targhooks.cc:default_builtin_vectorized_function
 */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent optimization */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision versions */
double in_d[256] __attribute__((aligned(32)));
double out_sin_d[256] __attribute__((aligned(32)));
double out_cos_d[256] __attribute__((aligned(32)));
double out_exp_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're not optimized away */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i] + 1.0f); /* sqrtf requires positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f); /* logf requires positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 2.0f);
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

/* Mixed operations to trigger multiple vectorization opportunities */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations that should all vectorize */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        val = sqrtf(fabsf(val) + 1.0f);
        out[i] = val;
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in[i] = (i % 128) * 0.1f + 0.1f; /* Range: 0.1 to 12.8 */
        in_d[i] = (i % 128) * 0.1 + 0.1;
    }
}

/* Compute checksum to ensure all computations are used */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + out_sqrt[i] + 
               out_log[i] + out_pow[i];
    }
    
    double sum_d = 0.0;
    for (int i = 0; i < 256; i++) {
        sum_d += out_sin_d[i] + out_cos_d[i] + out_exp_d[i];
    }
    
    return sum + (float)sum_d;
}

int main(void) {
    init_arrays();
    
    /* Call all vectorization test functions */
    test_vector_sin(in, out_sin, 256);
    test_vector_cos(in, out_cos, 256);
    test_vector_exp(in, out_exp, 256);
    test_vector_sqrt(in, out_sqrt, 256);
    test_vector_log(in, out_log, 256);
    test_vector_pow(in, out_pow, 256);
    
    test_vector_sin_d(in_d, out_sin_d, 256);
    test_vector_cos_d(in_d, out_cos_d, 256);
    test_vector_exp_d(in_d, out_exp_d, 256);
    
    /* Mixed operations */
    float out_mixed[256] __attribute__((aligned(32)));
    test_vector_mixed(in, out_mixed, 256);
    
    /* Compute and print checksum to prevent dead code elimination */
    float checksum = compute_checksum();
    
    /* Add mixed results to checksum */
    for (int i = 0; i < 256; i++) {
        checksum += out_mixed[i];
    }
    
    /* Store in volatile global to ensure side effects */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
