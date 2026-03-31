/* test_vector_builtin.c - Test program to cover vectorized built-in function handling */
#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent optimization */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in_array[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision arrays for additional coverage */
double in_double[256] __attribute__((aligned(32)));
double out_sin_d[256] __attribute__((aligned(32)));
double out_cos_d[256] __attribute__((aligned(32)));

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
        out[i] = sqrtf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);
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

/* Mixed operations to trigger multiple vectorization opportunities */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        val = sqrtf(fabsf(val));
        out[i] = val;
    }
}

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    double checksum_d = 0.0;
    
    /* Initialize input arrays with pattern-based data */
    for (int i = 0; i < n; i++) {
        in_array[i] = (i + 1) * 0.1f;  /* Avoid zero for log/sqrt */
        in_double[i] = (i + 1) * 0.1;
    }
    
    /* Call all vectorization test functions */
    test_vector_sin(in_array, out_sin, n);
    test_vector_cos(in_array, out_cos, n);
    test_vector_exp(in_array, out_exp, n);
    test_vector_sqrt(in_array, out_sqrt, n);
    test_vector_log(in_array, out_log, n);
    test_vector_pow(in_array, out_pow, n);
    test_vector_sin_d(in_double, out_sin_d, n);
    test_vector_cos_d(in_double, out_cos_d, n);
    test_vector_mixed(in_array, out_sin, n);  /* Reuse out_sin as temp */
    
    /* Compute checksums to ensure results are used */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_d += out_sin_d[i] + out_cos_d[i];
    }
    
    /* Use volatile global to prevent dead code elimination */
    global_sink = checksum;
    global_sink += (float)checksum_d;
    
    /* Print results to ensure execution */
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %f\n", checksum_d);
    printf("Total: %f\n", (float)checksum + (float)checksum_d);
    
    return 0;
}
