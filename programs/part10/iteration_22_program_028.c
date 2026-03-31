/* test_vector_builtin.c - Test program to cover vectorized built-in function handling */
#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent optimization */
volatile float global_sink = 0.0f;
volatile double global_sink_d = 0.0;

/* Aligned arrays for vectorization */
float in_f[256] __attribute__((aligned(32)));
float out_f[256] __attribute__((aligned(32)));
double in_d[256] __attribute__((aligned(32)));
double out_d[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're not optimized away */
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
        out[i] = logf(in[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrtf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
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
void test_vector_powf(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);
    }
}

/* Function to compute checksum and prevent dead code elimination */
__attribute__((noinline))
float compute_checksum_float(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
double compute_checksum_double(double *arr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    const int n = 256;
    
    /* Initialize input arrays with pattern-based data */
    for (int i = 0; i < n; i++) {
        in_f[i] = (i + 1) * 0.1f;  /* Avoid zero for log */
        in_d[i] = (i + 1) * 0.1;
    }
    
    /* Test various vectorized built-in functions */
    test_vector_sinf(in_f, out_f, n);
    global_sink += compute_checksum_float(out_f, n);
    
    test_vector_cosf(in_f, out_f, n);
    global_sink += compute_checksum_float(out_f, n);
    
    test_vector_expf(in_f, out_f, n);
    global_sink += compute_checksum_float(out_f, n);
    
    test_vector_logf(in_f, out_f, n);
    global_sink += compute_checksum_float(out_f, n);
    
    test_vector_sqrtf(in_f, out_f, n);
    global_sink += compute_checksum_float(out_f, n);
    
    test_vector_powf(in_f, out_f, n);
    global_sink += compute_checksum_float(out_f, n);
    
    /* Double precision tests */
    test_vector_sin(in_d, out_d, n);
    global_sink_d += compute_checksum_double(out_d, n);
    
    test_vector_cos(in_d, out_d, n);
    global_sink_d += compute_checksum_double(out_d, n);
    
    test_vector_exp(in_d, out_d, n);
    global_sink_d += compute_checksum_double(out_d, n);
    
    /* Print results to ensure computations aren't optimized away */
    printf("Float checksum: %f\n", (double)global_sink);
    printf("Double checksum: %f\n", global_sink_d);
    
    return 0;
}
