/* test_vector_builtin.c - Test program to cover vectorized built-in function hooks */

#include <math.h>
#include <stdio.h>

/* Global volatile sink to prevent optimization */
volatile float global_sink = 0.0f;
volatile double global_sink_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Prevent inlining to ensure separate vectorization contexts */
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
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);  /* Avoid log(0), triggers vectorized logf */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i] + 1.0f);  /* Avoid sqrt(0), triggers vectorized sqrtf */
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);  /* Triggers vectorized sin lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);  /* Triggers vectorized cos lookup */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);  /* Triggers vectorized exp lookup */
    }
}

/* Mixed operations to trigger multiple vectorized built-ins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain of math operations - each may need vectorized version */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val * 0.5f);
        val = logf(fabsf(val) + 1.0f);
        out[i] = sqrtf(val + 1.0f);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_f[i] = (i + 1) * 0.1f;  /* Non-zero, non-uniform values */
        in_d[i] = (i + 1) * 0.1;
    }
}

/* Compute checksum to ensure results are used */
float compute_checksum_f(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

double compute_checksum_d(double *arr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    init_arrays();
    
    /* Test single-precision vectorized built-ins */
    test_vector_sin(in_f, out_f, 256);
    global_sink += compute_checksum_f(out_f, 256);
    
    test_vector_cos(in_f, out_f, 256);
    global_sink += compute_checksum_f(out_f, 256);
    
    test_vector_exp(in_f, out_f, 256);
    global_sink += compute_checksum_f(out_f, 256);
    
    test_vector_log(in_f, out_f, 256);
    global_sink += compute_checksum_f(out_f, 256);
    
    test_vector_sqrt(in_f, out_f, 256);
    global_sink += compute_checksum_f(out_f, 256);
    
    /* Test double-precision vectorized built-ins */
    test_vector_sin_d(in_d, out_d, 256);
    global_sink_d += compute_checksum_d(out_d, 256);
    
    test_vector_cos_d(in_d, out_d, 256);
    global_sink_d += compute_checksum_d(out_d, 256);
    
    test_vector_exp_d(in_d, out_d, 256);
    global_sink_d += compute_checksum_d(out_d, 256);
    
    /* Test mixed operations */
    test_vector_mixed(in_f, out_f, 256);
    global_sink += compute_checksum_f(out_f, 256);
    
    /* Print results to prevent dead code elimination */
    printf("Float checksum: %f\n", (float)global_sink);
    printf("Double checksum: %lf\n", (double)global_sink_d);
    
    return 0;
}
