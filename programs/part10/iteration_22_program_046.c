/* test_vector_builtin.c - Test program to cover vectorized built-in function handling */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision variants */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));
double out_exp_double[256] __attribute__((aligned(32)));
double out_sqrt_double[256] __attribute__((aligned(32)));

/* Test functions with noinline to ensure they're not optimized away */
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

/* Double precision variants */
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

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_double(double *restrict src, double *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = exp(src[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt_double(double *restrict src, double *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = sqrt(src[i]);
    }
}

/* Mixed operations to trigger multiple vectorized built-ins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple built-in calls */
        float val = src[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        val = sqrtf(fabsf(val)); /* Use fabsf to ensure positive sqrt input */
        dst[i] = val;
    }
}

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    /* Initialize input arrays with pattern-based data */
    for (int i = 0; i < n; i++) {
        in[i] = (i + 1) * 0.1f; /* Avoid zero for log/pow */
        in_double[i] = (i + 1) * 0.1;
    }
    
    /* Call all vectorization test functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_pow(in, out_pow, n);
    
    /* Double precision tests */
    test_vector_sin_double(in_double, out_sin_double, n);
    test_vector_cos_double(in_double, out_cos_double, n);
    test_vector_exp_double(in_double, out_exp_double, n);
    test_vector_sqrt_double(in_double, out_sqrt_double, n);
    
    /* Mixed operations test */
    test_vector_mixed(in, out_sin, n); /* Reuse out_sin array */
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_double += out_sin_double[i] + out_cos_double[i] + 
                          out_exp_double[i] + out_sqrt_double[i];
    }
    
    /* Store to volatile global to ensure computation isn't optimized away */
    global_sink = checksum;
    
    /* Print results to ensure observable side effects */
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %lf\n", checksum_double);
    
    return 0;
}
