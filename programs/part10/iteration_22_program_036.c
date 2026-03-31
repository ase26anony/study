/* test_vector_builtin.c - Test program to cover vectorized built-in function flags */

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

/* Prevent inlining to ensure separate vectorization contexts */
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
        dst[i] = logf(src[i] + 1.0f); /* +1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict src, float *restrict dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = powf(src[i] + 1.0f, 2.0f); /* (x+1)^2 */
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
        in[i] = i * 0.1f + 0.01f; /* Avoid exact zeros for log/pow */
        in_double[i] = i * 0.1 + 0.01;
    }
    
    /* Call all vectorized test functions */
    test_vector_sin(in, out_sin, n);
    test_vector_cos(in, out_cos, n);
    test_vector_exp(in, out_exp, n);
    test_vector_sqrt(in, out_sqrt, n);
    test_vector_log(in, out_log, n);
    test_vector_pow(in, out_pow, n);
    test_vector_sin_double(in_double, out_sin_double, n);
    test_vector_cos_double(in_double, out_cos_double, n);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_double += out_sin_double[i] + out_cos_double[i];
    }
    
    /* Use volatile global to ensure computations aren't optimized away */
    global_sink = checksum;
    
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %lf\n", checksum_double);
    
    return 0;
}
