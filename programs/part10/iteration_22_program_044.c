/* test_vector_builtin.c
 * Test program to cover vectorized built-in function creation in GCC's targhooks.cc
 * Compile with: gcc -O3 -ffast-math -ftree-vectorize -march=native -c test_vector_builtin.c
 */

#include <math.h>
#include <stdio.h>

/* Global volatile sink to prevent dead code elimination */
volatile float global_sink = 0.0f;

/* Aligned arrays for vectorization */
float __attribute__((aligned(32))) in_array[256];
float __attribute__((aligned(32))) out_sin[256];
float __attribute__((aligned(32))) out_cos[256];
float __attribute__((aligned(32))) out_exp[256];
float __attribute__((aligned(32))) out_sqrt[256];
float __attribute__((aligned(32))) out_log[256];
float __attribute__((aligned(32))) out_pow[256];

/* Initialize input array with pattern */
void __attribute__((noinline)) init_array(void) {
    for (int i = 0; i < 256; i++) {
        /* Use values in valid ranges for all functions */
        in_array[i] = (i + 1) * 0.01f + 0.1f;  /* Ensure > 0 for log/sqrt */
    }
}

/* Test vectorized sinf */
void __attribute__((noinline, optimize("O3,ffast-math"))) test_vector_sin(void) {
    for (int i = 0; i < 256; i++) {
        out_sin[i] = sinf(in_array[i]);
    }
}

/* Test vectorized cosf */
void __attribute__((noinline, optimize("O3,ffast-math"))) test_vector_cos(void) {
    for (int i = 0; i < 256; i++) {
        out_cos[i] = cosf(in_array[i]);
    }
}

/* Test vectorized expf */
void __attribute__((noinline, optimize("O3,ffast-math"))) test_vector_exp(void) {
    for (int i = 0; i < 256; i++) {
        out_exp[i] = expf(in_array[i]);
    }
}

/* Test vectorized sqrtf */
void __attribute__((noinline, optimize("O3,ffast-math"))) test_vector_sqrt(void) {
    for (int i = 0; i < 256; i++) {
        out_sqrt[i] = sqrtf(in_array[i]);
    }
}

/* Test vectorized logf */
void __attribute__((noinline, optimize("O3,ffast-math"))) test_vector_log(void) {
    for (int i = 0; i < 256; i++) {
        out_log[i] = logf(in_array[i]);
    }
}

/* Test vectorized powf */
void __attribute__((noinline, optimize("O3,ffast-math"))) test_vector_pow(void) {
    for (int i = 0; i < 256; i++) {
        out_pow[i] = powf(in_array[i], 2.0f);
    }
}

/* Test with restrict pointers for better vectorization */
void __attribute__((noinline, optimize("O3,ffast-math"))) 
test_vector_restrict(float * __restrict__ out, const float * __restrict__ in) {
    for (int i = 0; i < 256; i++) {
        out[i] = sinf(in[i]) + cosf(in[i]);
    }
}

/* Test double precision for wider coverage */
double __attribute__((aligned(32))) in_double[256];
double __attribute__((aligned(32))) out_double[256];

void __attribute__((noinline, optimize("O3,ffast-math"))) test_vector_double(void) {
    for (int i = 0; i < 256; i++) {
        out_double[i] = sin(in_double[i]) * cos(in_double[i]);
    }
}

/* Main function that aggregates results to prevent optimization */
int main(void) {
    float checksum = 0.0f;
    
    /* Initialize arrays */
    init_array();
    
    /* Initialize double array */
    for (int i = 0; i < 256; i++) {
        in_double[i] = (i + 1) * 0.01 + 0.1;
    }
    
    /* Call all vectorization test functions */
    test_vector_sin();
    test_vector_cos();
    test_vector_exp();
    test_vector_sqrt();
    test_vector_log();
    test_vector_pow();
    
    /* Test with restrict qualifier */
    float __attribute__((aligned(32))) temp[256];
    test_vector_restrict(temp, in_array);
    
    /* Test double precision */
    test_vector_double();
    
    /* Aggregate results to prevent dead code elimination */
    for (int i = 0; i < 256; i++) {
        checksum += out_sin[i] + out_cos[i] + out_exp[i] + 
                   out_sqrt[i] + out_log[i] + out_pow[i] + 
                   temp[i] + (float)out_double[i];
    }
    
    /* Store in volatile global to ensure computation isn't optimized away */
    global_sink = checksum;
    
    /* Print checksum to ensure observable behavior */
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
