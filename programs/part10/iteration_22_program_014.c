/* test_vector_builtin.c - Test program to cover vectorized built-in function hooks */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in_array[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision arrays for testing double built-ins */
double in_double[256] __attribute__((aligned(32)));
double out_sin_d[256] __attribute__((aligned(32)));
double out_cos_d[256] __attribute__((aligned(32)));

/* Vectorized sin function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

/* Vectorized cos function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

/* Vectorized exp function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

/* Vectorized sqrt function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(in[i] + 1.0f);  /* Add 1 to avoid sqrt(negative) */
    }
}

/* Vectorized log function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(in[i] + 1.0f);  /* Add 1 to avoid log(0) */
    }
}

/* Vectorized pow function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i] + 1.0f, 2.0f);  /* Square the values */
    }
}

/* Vectorized sin function for doubles */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

/* Vectorized cos function for doubles */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_d(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_array[i] = (i % 32) * 0.1f + 0.01f;  /* Range: 0.01 to 3.21 */
        in_double[i] = (i % 32) * 0.1 + 0.01;    /* Same for doubles */
    }
}

/* Compute checksum to ensure computations aren't optimized away */
float compute_checksum(void) {
    float sum = 0.0f;
    
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + 
               out_sqrt[i] + out_log[i] + out_pow[i];
    }
    
    /* Also include double precision results */
    for (int i = 0; i < 256; i++) {
        sum += (float)(out_sin_d[i] + out_cos_d[i]);
    }
    
    return sum;
}

int main(void) {
    /* Initialize input arrays */
    init_arrays();
    
    /* Call all vectorized test functions */
    test_vector_sin(in_array, out_sin, 256);
    test_vector_cos(in_array, out_cos, 256);
    test_vector_exp(in_array, out_exp, 256);
    test_vector_sqrt(in_array, out_sqrt, 256);
    test_vector_log(in_array, out_log, 256);
    test_vector_pow(in_array, out_pow, 256);
    test_vector_sin_d(in_double, out_sin_d, 256);
    test_vector_cos_d(in_double, out_cos_d, 256);
    
    /* Compute and print checksum */
    float checksum = compute_checksum();
    global_sink = checksum;  /* Store in volatile to prevent optimization */
    
    printf("Vectorized built-in test checksum: %f\n", checksum);
    
    /* Also print a few sample values to verify correctness */
    printf("Sample values - sin[0]=%f, cos[0]=%f, exp[0]=%f\n", 
           out_sin[0], out_cos[0], out_exp[0]);
    
    return 0;
}
