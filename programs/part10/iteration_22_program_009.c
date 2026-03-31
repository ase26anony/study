/* test_vector_builtin.c - Coverage test for targhooks.cc default_builtin_vectorized_function */
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

/* Prevent inlining and enforce vectorization-friendly optimizations */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sinf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cosf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_expf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_logf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i] + 1.0f);  /* sqrt(x+1) to avoid domain errors */
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrt(in[i] + 1.0);  /* sqrt(x+1) to avoid domain errors */
    }
}

int main(void) {
    const int n = 256;
    float checksum_f = 0.0f;
    double checksum_d = 0.0;
    
    /* Initialize arrays with pattern data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i + 1) * 0.1f;      /* Values from 0.1 to 25.6 */
        in_d[i] = (i + 1) * 0.1;       /* Same for double */
    }
    
    /* Test all float built-in functions */
    test_vector_sinf(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_cosf(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_expf(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_logf(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    test_vector_sqrtf(in_f, out_f, n);
    for (int i = 0; i < n; i++) checksum_f += out_f[i];
    
    /* Test all double built-in functions */
    test_vector_sin(in_d, out_d, n);
    for (int i = 0; i < n; i++) checksum_d += out_d[i];
    
    test_vector_cos(in_d, out_d, n);
    for (int i = 0; i < n; i++) checksum_d += out_d[i];
    
    test_vector_exp(in_d, out_d, n);
    for (int i = 0; i < n; i++) checksum_d += out_d[i];
    
    test_vector_sqrt(in_d, out_d, n);
    for (int i = 0; i < n; i++) checksum_d += out_d[i];
    
    /* Use volatile sinks to prevent optimization */
    global_sink = checksum_f;
    global_sink_d = checksum_d;
    
    /* Print results to ensure computations aren't eliminated */
    printf("Float checksum: %f\n", checksum_f);
    printf("Double checksum: %lf\n", checksum_d);
    
    return 0;
}
