/* test_vector_builtin.c
 * This program triggers GCC's vectorization of built-in math functions,
 * causing the compiler to call default_builtin_vectorized_function
 * and execute the flag-setting code for vectorized built-in declarations.
 */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;
volatile double global_sink_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Prevent inlining to ensure separate vectorization contexts */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_float(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);  /* Triggers vectorized sinf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_float(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);  /* Triggers vectorized cosf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_float(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);  /* Triggers vectorized expf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log_float(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i]);  /* Triggers vectorized logf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt_float(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i]);  /* Triggers vectorized sqrtf lookup */
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);  /* Triggers vectorized sin lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);  /* Triggers vectorized cos lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);  /* Triggers vectorized exp lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow_float(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);  /* Triggers vectorized powf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_floor_float(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = floorf(in[i]);  /* Triggers vectorized floorf lookup */
    }
}

int main(void) {
    const int n = 256;
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    /* Initialize arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i + 1) * 0.1f;  /* Avoid zero for log */
        in_d[i] = (i + 1) * 0.1;
    }
    
    /* Test various float built-ins */
    test_vector_sin_float(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_cos_float(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_exp_float(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_log_float(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_sqrt_float(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_pow_float(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_floor_float(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    /* Test double built-ins */
    test_vector_sin_double(in_d, out_d, n);
    for (int i = 0; i < n; i++) checksum_d += out_d[i];
    
    test_vector_cos_double(in_d, out_d, n);
    for (int i = 0; i < n; i++) checksum_d += out_d[i];
    
    test_vector_exp_double(in_d, out_d, n);
    for (int i = 0; i < n; i++) checksum_d += out_d[i];
    
    /* Store to volatile to prevent optimization */
    global_sink = checksum_f;
    global_sink_d = checksum_d;
    
    /* Print results to ensure execution */
    printf("Float checksum: %f\n", checksum_f);
    printf("Double checksum: %f\n", checksum_d);
    
    return 0;
}
