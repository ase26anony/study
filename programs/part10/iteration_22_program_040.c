/* test_vector_builtin.c - Test program to cover vectorized built-in function handling */

#include <stdio.h>
#include <math.h>

/* Global volatile variable to prevent optimization */
volatile float g_checksum = 0.0f;
volatile double g_dchecksum = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorized sin function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sinf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

/* Vectorized cos function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cosf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

/* Vectorized exp function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_expf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

/* Vectorized log function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_logf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);  /* +1 to avoid log(0) */
    }
}

/* Vectorized sqrt function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f);  /* Ensure positive input */
    }
}

/* Vectorized pow function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_powf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(fabsf(in[i]) + 1.0f, 2.0f);
    }
}

/* Double precision versions to trigger different vectorized builtins */

/* Vectorized sin function for doubles */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

/* Vectorized cos function for doubles */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

/* Vectorized exp function for doubles */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

/* Vectorized sqrt function for doubles */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrt(fabs(in[i]) + 0.1);
    }
}

/* Complex loop with multiple builtins to increase coverage probability */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_mixed_builtins(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple builtins to potentially trigger different vectorized versions */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x));
    }
}

int main(void) {
    const int n = 256;
    
    /* Initialize input arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i * 0.1f) - 12.8f;  /* Range: -12.8 to 12.7 */
        in_d[i] = (i * 0.1) - 12.8;    /* Same range for doubles */
    }
    
    /* Test all float vectorized builtins */
    test_vector_sinf(in_f, out_f, n);
    test_vector_cosf(in_f, out_f, n);
    test_vector_expf(in_f, out_f, n);
    test_vector_logf(in_f, out_f, n);
    test_vector_sqrtf(in_f, out_f, n);
    test_vector_powf(in_f, out_f, n);
    test_mixed_builtins(in_f, out_f, n);
    
    /* Test all double vectorized builtins */
    test_vector_sin(in_d, out_d, n);
    test_vector_cos(in_d, out_d, n);
    test_vector_exp(in_d, out_d, n);
    test_vector_sqrt(in_d, out_d, n);
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        g_checksum += out_f[i];
        g_dchecksum += out_d[i];
    }
    
    /* Print results to ensure computations aren't optimized away */
    printf("Float checksum: %f\n", (float)g_checksum);
    printf("Double checksum: %lf\n", (double)g_dchecksum);
    
    return 0;
}
