/* test_vector_builtin.c - Test program to cover vectorized built-in function flags */
#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays to enable vectorization */
float in[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Test functions with vectorization-friendly loops */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = sinf(src[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = cosf(src[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = expf(src[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = sqrtf(src[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = logf(src[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = powf(src[i], 2.0f);
    }
}

/* Double precision versions to trigger different vectorized builtins */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(double *restrict src, double *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = sin(src[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(double *restrict src, double *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = cos(src[i]);
    }
}

int main(void) {
    const int n = 256;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in[i] = (i + 1) * 0.1f; /* Avoid zero for sqrt/log */
        in_double[i] = (i + 1) * 0.1;
    }
    
    /* Call all vectorization test functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_pow(in, out_pow, n);
    test_vector_sin_double(in_double, out_sin_double, n);
    test_vector_cos_double(in_double, out_cos_double, n);
    
    /* Compute checksum to ensure all loops execute */
    float checksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i] +
                   (float)(out_sin_double[i] + out_cos_double[i]);
    }
    
    /* Use volatile global to prevent optimization */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
