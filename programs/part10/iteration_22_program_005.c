/* test_vector_builtin.c - Test program to cover vectorized built-in function creation */
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

/* Test functions with noinline to ensure they're not optimized away */
/* Each function tests a different built-in math function */

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
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(fabsf(in[i]) + 1.0f); /* Ensure positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(fabsf(in[i]) + 0.5f, 2.0f);
    }
}

/* Double precision versions to test more built-ins */
double in_d[256] __attribute__((aligned(32)));
double out_sin_d[256] __attribute__((aligned(32)));
double out_cos_d[256] __attribute__((aligned(32)));

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

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    double checksum_d = 0.0;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in[i] = (i * 0.1f) - 12.8f; /* Range: -12.8 to 12.7 */
        in_d[i] = (i * 0.1) - 12.8;
    }
    
    /* Call all vectorized test functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_pow(in, out_pow, n);
    test_vector_sin_d(in_d, out_sin_d, n);
    test_vector_cos_d(in_d, out_cos_d, n);
    
    /* Compute checksums to ensure results are used */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_d += out_sin_d[i] + out_cos_d[i];
        global_sink += out_sin[i]; /* Use volatile global */
    }
    
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %lf\n", checksum_d);
    printf("Global sink: %f\n", (float)global_sink);
    
    return 0;
}
