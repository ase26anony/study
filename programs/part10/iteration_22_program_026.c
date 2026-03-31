/* test_vector_builtin.c
 * This program triggers vectorization of built-in math functions,
 * causing GCC to call default_builtin_vectorized_function and
 * execute the flag-setting code in targhooks.cc lines 981-990.
 */

#include <stdio.h>
#include <math.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float in[256] __attribute__((aligned(32)));
float out_sin[256] __attribute__((aligned(32)));
float out_cos[256] __attribute__((aligned(32)));
float out_exp[256] __attribute__((aligned(32)));
float out_sqrt[256] __attribute__((aligned(32)));
float out_log[256] __attribute__((aligned(32)));
float out_pow[256] __attribute__((aligned(32)));

/* Double precision variants */
double in_double[256] __attribute__((aligned(32)));
double out_sin_double[256] __attribute__((aligned(32)));
double out_cos_double[256] __attribute__((aligned(32)));

/* Vectorization test functions with noinline to ensure they're not optimized away */
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
        out[i] = logf(in[i] + 1.0f);  /* Avoid log(0), triggers vectorized logf */
    }
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = powf(in[i], 2.0f);  /* Triggers vectorized powf lookup */
    }
}

/* Double precision variants to trigger different vectorized builtins */
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

/* Mixed operations to increase coverage probability */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_mixed(float *restrict in, float *restrict out, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple builtins in one loop - may trigger multiple lookups */
        float x = sinf(in[i]);
        float y = cosf(in[i]);
        out[i] = sqrtf(x*x + y*y);  /* hypot approximation */
    }
}

/* Initialize arrays with pattern data */
void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        /* Use values that are safe for all math functions */
        in[i] = (i + 1) * 0.01f;  /* Range: 0.01 to 2.56 */
        in_double[i] = (i + 1) * 0.01;
    }
}

/* Compute checksum to ensure all results are used */
float compute_checksum(void) {
    float sum = 0.0f;
    for (int i = 0; i < 256; i++) {
        sum += out_sin[i] + out_cos[i] + out_exp[i] + out_sqrt[i] +
               out_log[i] + out_pow[i];
    }
    
    /* Add double precision results */
    for (int i = 0; i < 256; i++) {
        sum += (float)(out_sin_double[i] + out_cos_double[i]);
    }
    
    return sum;
}

int main(void) {
    init_arrays();
    
    /* Call all vectorization test functions */
    test_vector_sin(in, out_sin, 256);
    test_vector_cos(in, out_cos, 256);
    test_vector_exp(in, out_exp, 256);
    test_vector_sqrt(in, out_sqrt, 256);
    test_vector_log(in, out_log, 256);
    test_vector_pow(in, out_pow, 256);
    
    test_vector_sin_double(in_double, out_sin_double, 256);
    test_vector_cos_double(in_double, out_cos_double, 256);
    
    /* Mixed operation test */
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
