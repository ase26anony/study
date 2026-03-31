/* test_vector_builtin.c
 * This program triggers vectorization of built-in math functions,
 * causing GCC to call default_builtin_vectorized_function and
 * execute the flag-setting code for vectorized built-in declarations.
 */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays to enable vector loads/stores */
float in[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision variants for additional coverage */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));

/* Vectorized sinf function - noinline to ensure separate analysis */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sinf(in[i]);
    }
}

/* Vectorized cosf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cosf(in[i]);
    }
}

/* Vectorized expf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = expf(in[i]);
    }
}

/* Vectorized sqrtf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sqrtf(fabsf(in[i]) + 0.1f); /* Ensure positive input */
    }
}

/* Vectorized logf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = logf(fabsf(in[i]) + 1.0f); /* Ensure positive input */
    }
}

/* Vectorized powf function */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(fabsf(in[i]) + 0.5f, 2.0f);
    }
}

/* Double precision variants */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = sin(in[i]);
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(double *restrict in, double *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = cos(in[i]);
    }
}

/* Mixed operations to trigger multiple vectorized built-ins */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Chain multiple built-ins to increase coverage chance */
        float x = sinf(in[i]);
        x = cosf(x);
        x = expf(x);
        out[i] = sqrtf(fabsf(x) + 0.1f);
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in[i] = (i - 128) * 0.01f; /* Range: -1.28 to 1.27 */
        in_double[i] = (i - 128) * 0.01;
    }
}

/* Compute checksum to ensure results are used */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + 
               out_sqrt[i] + out_log[i] + out_pow[i];
    }
    
    double sum_double = 0.0;
    for (int i = 0; i < 256; i++) {
        sum_double += out_sin_double[i] + out_cos_double[i];
    }
    
    return sum + (float)sum_double;
}

int main(void) {
    /* Initialize input arrays */
    init_arrays();
    
    /* Call vectorized functions with different built-ins */
    test_vector_sin(in, out_sin, 256);
    test_vector_cos(in, out_cos, 256);
    test_vector_exp(in, out_exp, 256);
    test_vector_sqrt(in, out_sqrt, 256);
    test_vector_log(in, out_log, 256);
    test_vector_pow(in, out_pow, 256);
    
    /* Double precision tests */
    test_vector_sin_double(in_double, out_sin_double, 256);
    test_vector_cos_double(in_double, out_cos_double, 256);
    
    /* Mixed operations test */
    float mixed_out[256] __attribute__((aligned(32)));
    test_vector_mixed(in, mixed_out, 256);
    
    /* Compute and print checksum to prevent optimization */
    float checksum = compute_checksum();
    
    /* Add mixed results to checksum */
    for (int i = 0; i < 256; i++) {
        checksum += mixed_out[i];
    }
    
    /* Store in volatile global to ensure computation isn't eliminated */
    global_sink = checksum;
    
    printf("Checksum: %f\n", checksum);
    printf("Global sink: %f\n", (float)global_sink);
    
    return 0;
}
