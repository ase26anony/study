/* test_vector_builtin.c - Test program to cover vectorized built-in function flag setting */
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

/* Double precision variants */
double in_d[256] __attribute__((aligned(32)));
double out_sin_d[256] __attribute__((aligned(32)));
double out_cos_d[256] __attribute__((aligned(32)));
double out_exp_d[256] __attribute__((aligned(32)));
double out_sqrt_d[256] __attribute__((aligned(32)));

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
        out[i] = sqrtf(in[i] + 1.0f);  /* sqrtf requires positive input */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);   /* logf requires positive input */
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

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrt(in[i] + 1.0);
    }
}

/* Mixed operations to trigger multiple vectorized built-ins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple math operations */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x));  /* Use fabsf to ensure positive sqrt input */
    }
}

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    double checksum_d = 0.0;
    
    /* Initialize input arrays with pattern-based data */
    for (int i = 0; i < n; i++) {
        in[i] = i * 0.1f;
        in_d[i] = i * 0.1;
    }
    
    /* Test all vectorized built-in functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_pow(in, out_pow, n);
    
    /* Double precision tests */
    test_vector_sin_d(in_d, out_sin_d, n);
    test_vector_cos_d(in_d, out_cos_d, n);
    test_vector_exp_d(in_d, out_exp_d, n);
    test_vector_sqrt_d(in_d, out_sqrt_d, n);
    
    /* Mixed operations test */
    test_vector_mixed(in, out_sin, n);  /* Reuse out_sin array */
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_d += out_sin_d[i] + out_cos_d[i] + out_exp_d[i] + out_sqrt_d[i];
    }
    
    /* Use volatile global to ensure computations aren't optimized away */
    global_sink = checksum;
    global_sink += (float)checksum_d;
    
    printf("Checksum (float): %f\n", checksum);
    printf("Checksum (double): %lf\n", checksum_d);
    printf("Global sink: %f\n", (float)global_sink);
    
    return 0;
}
