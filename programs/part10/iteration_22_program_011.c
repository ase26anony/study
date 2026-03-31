/* test_vector_builtin.c
 * This program triggers vectorization of built-in math functions,
 * causing GCC to call default_builtin_vectorized_function and
 * execute the flag-setting code for vectorized built-in declarations.
 */

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

/* Test functions with noinline to ensure they're not optimized away */
/* Each function uses a different built-in math function in a vectorizable loop */

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);  /* Triggers vectorized sinf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);  /* Triggers vectorized cosf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);  /* Triggers vectorized expf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i]);  /* Triggers vectorized sqrtf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);  /* Triggers vectorized logf lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);  /* Triggers vectorized powf lookup */
    }
}

/* Double precision versions to trigger different vectorized built-ins */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);  /* Triggers vectorized sin (double) lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);  /* Triggers vectorized cos (double) lookup */
    }
}

int main(void) {
    const int n = 256;
    float checksum = 0.0f;
    double checksum_double = 0.0;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in[i] = (i + 1) * 0.1f;  /* Range: 0.1 to 25.6 */
        in_double[i] = (i + 1) * 0.1;  /* Same for double */
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
    
    /* Compute checksums to ensure results are used */
    for (int i = 0; i < n; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i];
        checksum_double += out_sin_double[i] + out_cos_double[i];
        
        /* Also use global volatile sink */
        global_sink += out_sin[i] * 0.0001f;
    }
    
    /* Print checksums to prevent optimization */
    printf("Float checksum: %f\n", checksum);
    printf("Double checksum: %f\n", checksum_double);
    printf("Global sink: %f\n", (float)global_sink);
    
    return 0;
}
