/* Test program to cover SIMT transformation in omp-low.cc */
/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o test_simt test_simt.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_add(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline))
void target_simt_conditional_scale(float *data, int n, float scale) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n])
    for (int i = 0; i < n; ++i) {
        if (data[i] > THRESHOLD) {
            data[i] = data[i] * scale;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_simt_complex_loop(float *in1, float *in2, float *out, int n, int mode) {
    #pragma omp target teams distribute parallel for \
        map(to: in1[0:n], in2[0:n], mode) map(from: out[0:n])
    for (int i = 0; i < n; ++i) {
        float temp;
        if (mode == 0) {
            temp = in1[i] * in2[i];
        } else if (mode == 1) {
            temp = in1[i] + in2[i];
        } else {
            temp = in1[i] - in2[i];
        }
        /* Nested control flow to create more complex GIMPLE */
        if (temp > 0) {
            out[i] = sqrtf(temp);
        } else {
            out[i] = -sqrtf(-temp);
        }
    }
}

__attribute__((noinline))
void target_simt_nested_if(float *arr, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:n])
    for (int i = 0; i < n; ++i) {
        /* Multiple nested conditions to create interesting GIMPLE */
        if (i % 2 == 0) {
            if (arr[i] > 0.0f) {
                arr[i] = arr[i] * 2.0f;
            } else {
                arr[i] = arr[i] * 0.5f;
            }
        } else {
            if (arr[i] < 0.0f) {
                arr[i] = arr[i] - 1.0f;
            } else {
                arr[i] = arr[i] + 1.0f;
            }
        }
    }
}

float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    float *d = (float *)malloc(N * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.7f;
        d[i] = (float)(i % 100) * 10.0f;
    }
    
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 3;
    }
    
    /* Call target functions multiple times with different parameters */
    printf("Testing SIMT transformations...\n");
    
    /* Test 1: Simple vector addition with simd clause */
    target_simt_vector_add(a, b, c, N);
    printf("Checksum 1: %f\n", compute_checksum(c, N));
    
    /* Test 2: Conditional scaling - triggers SIMT without explicit simd clause */
    target_simt_conditional_scale(d, N, 2.0f);
    printf("Checksum 2: %f\n", compute_checksum(d, N));
    
    /* Test 3: Complex loop with mode selection */
    target_simt_complex_loop(a, b, c, N, mode);
    printf("Checksum 3 (mode=%d): %f\n", mode, compute_checksum(c, N));
    
    /* Test 4: Nested if statements with simd clause */
    target_simt_nested_if(d, N);
    printf("Checksum 4: %f\n", compute_checksum(d, N));
    
    /* Repeat with different sizes to increase coverage */
    for (int size = 256; size <= 512; size += 256) {
        float *small_a = (float *)malloc(size * sizeof(float));
        float *small_b = (float *)malloc(size * sizeof(float));
        float *small_c = (float *)malloc(size * sizeof(float));
        
        for (int i = 0; i < size; ++i) {
            small_a[i] = (float)i;
            small_b[i] = (float)(size - i);
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(to: small_a[0:size], small_b[0:size]) map(from: small_c[0:size])
        for (int i = 0; i < size; ++i) {
            small_c[i] = small_a[i] * small_b[i];
        }
        
        printf("Small size %d checksum: %f\n", size, compute_checksum(small_c, size));
        
        free(small_a);
        free(small_b);
        free(small_c);
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
