/* test_vector_builtin.c - Coverage test for targhooks.cc default_builtin_vectorized_function */

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

/* Double precision arrays */
double din[256] __attribute__((aligned(32)));
double dout_sin[256] __attribute__((aligned(32)));
double dout_cos[256] __attribute__((aligned(32)));
double dout_exp[256] __attribute__((aligned(32)));

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
        out[i] = sqrtf(in[i] + 1.0f); /* Add 1 to avoid sqrt(negative) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 2.0f);
    }
}

/* Double precision versions */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_dsin(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_dcos(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_dexp(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = exp(in[i]);
    }
}

/* Mixed operations to trigger multiple vectorized builtins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple math operations */
        float val = in[i];
        val = sinf(val);
        val = cosf(val);
        val = expf(val);
        val = sqrtf(fabsf(val)); /* Use fabs to ensure positive */
        out[i] = val;
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in[i] = i * 0.1f;
        din[i] = i * 0.1;
    }
}

/* Compute checksum to ensure computations aren't optimized away */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + 
               out_sqrt[i] + out_log[i] + out_pow[i];
    }
    
    double dsum = 0.0;
    for (int i = 0; i < 256; i++) {
        dsum += dout_sin[i] + dout_cos[i] + dout_exp[i];
    }
    
    return sum + (float)dsum;
}

int main(void) {
    /* Initialize arrays */
    init_arrays();
    
    /* Call all vectorization test functions */
    test_vector_sin(in, out_sin, 256);
    test_vector_cos(in, out_cos, 256);
    test_vector_exp(in, out_exp, 256);
    test_vector_sqrt(in, out_sqrt, 256);
    test_vector_log(in, out_log, 256);
    test_vector_pow(in, out_pow, 256);
    
    /* Double precision tests */
    test_vector_dsin(din, dout_sin, 256);
    test_vector_dcos(din, dout_cos, 256);
    test_vector_dexp(din, dout_exp, 256);
    
    /* Mixed operations test */
    float mixed_out[256] __attribute__((aligned(32)));
    test_vector_mixed(in, mixed_out, 256);
    
    /* Compute and print checksum to prevent dead code elimination */
    float checksum = compute_checksum();
    
    /* Add mixed results to checksum */
    for (int i = 0; i < 256; i++) {
        checksum += mixed_out[i];
    }
    
    /* Store in volatile global to ensure side effect */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
