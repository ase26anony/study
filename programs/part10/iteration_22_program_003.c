/* test_vector_builtin.c
 * Test program to cover default_builtin_vectorized_function in targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -fdump-tree-vect-details -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Prevent optimizations from removing our computations */
static volatile float volatile_sink = 0.0f;
static volatile double volatile_sink_d = 0.0;

/* Aligned arrays to enable vectorization */
static float in_f[256] __attribute__((aligned(32)));
static float out_f[256] __attribute__((aligned(32)));
static double in_d[256] __attribute__((aligned(32)));
static double out_d[256] __attribute__((aligned(32)));

/* Initialize arrays with pattern data */
__attribute__((constructor))
static void init_arrays(void) {
    for (int i = 0; i < 256; i++) {
        in_f[i] = i * 0.1f;
        in_d[i] = i * 0.1;
    }
}

/* Vectorized sin function for floats - noinline to ensure separate analysis */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sinf(in_f[i]);
    }
    /* Use result to prevent dead code elimination */
    volatile_sink += out_f[0];
}

/* Vectorized cos function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = cosf(in_f[i]);
    }
    volatile_sink += out_f[1];
}

/* Vectorized exp function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = expf(in_f[i]);
    }
    volatile_sink += out_f[2];
}

/* Vectorized log function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_log(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = logf(in_f[i] + 1.0f); /* Add 1 to avoid log(0) */
    }
    volatile_sink += out_f[3];
}

/* Vectorized sqrt function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sqrt(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = sqrtf(fabsf(in_f[i]) + 0.1f); /* Ensure positive input */
    }
    volatile_sink += out_f[4];
}

/* Vectorized pow function for floats */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_pow(void) {
    for (int i = 0; i < 256; i++) {
        out_f[i] = powf(fabsf(in_f[i]) + 0.1f, 2.0f);
    }
    volatile_sink += out_f[5];
}

/* Double precision versions to trigger different vectorization paths */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_sin_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = sin(in_d[i]);
    }
    volatile_sink_d += out_d[0];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_cos_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = cos(in_d[i]);
    }
    volatile_sink_d += out_d[1];
}

__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_exp_double(void) {
    for (int i = 0; i < 256; i++) {
        out_d[i] = exp(in_d[i]);
    }
    volatile_sink_d += out_d[2];
}

/* Function with restrict pointers to help vectorizer */
__attribute__((noinline, optimize("O3,ffast-math")))
void test_vector_restrict(float *__restrict a, const float *__restrict b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = sinf(b[i]) + cosf(b[i]);
    }
    volatile_sink += a[0];
}

/* Main function that calls all test cases */
int main(void) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    /* Call all vectorization test functions */
    test_vector_sin();
    test_vector_cos();
    test_vector_exp();
    test_vector_log();
    test_vector_sqrt();
    test_vector_pow();
    
    test_vector_sin_double();
    test_vector_cos_double();
    test_vector_exp_double();
    
    /* Test with restrict pointers */
    test_vector_restrict(out_f, in_f, 256);
    
    /* Compute checksums to ensure all results are used */
    for (int i = 0; i < 256; i++) {
        sum_f += out_f[i];
        sum_d += out_d[i];
    }
    
    /* Print results to prevent optimization */
    printf("Float checksum: %f\n", sum_f);
    printf("Double checksum: %f\n", sum_d);
    printf("Volatile sinks: %f, %f\n", (float)volatile_sink, (float)volatile_sink_d);
    
    return 0;
}
