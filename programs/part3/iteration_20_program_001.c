/* test_omp_simt_lowering.c
 * Designed to trigger SIMT transformation in omp-low.cc
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower test_omp_simt_lowering.c -o test_simt
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough body for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) + 1.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Multiple conditional paths to create interesting GIMPLE */
        if (i % 2 == 0) {
            c[i] = a[i] + b[i];
        } else if (i % 3 == 0) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = fmaxf(a[i], b[i]);
        }
    }
}

__attribute__((noinline))
void target_mixed_directives(float *arr, int size, int mode) {
    /* Different loop configurations based on mode */
    if (mode == 0) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: arr[0:size]) collapse(2)
        for (int i = 0; i < size/2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int idx = i * 2 + j;
                arr[idx] = arr[idx] + (float)(i + j);
            }
        }
    } else {
        #pragma omp target teams distribute parallel for \
            map(tofrom: arr[0:size]) num_teams(2)
        for (int i = 0; i < size; ++i) {
            arr[i] = sinf(arr[i]) * cosf(arr[i]);
        }
    }
}

float compute_checksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    /* Initialize test data */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i;
        data2[i] = (float)(N - i);
        data3[i] = (float)(i * 2);
    }
    
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Execute multiple target regions to increase coverage probability */
    printf("Testing SIMT lowering with mode %d\n", mode);
    
    /* Test 1: Basic SIMD clause with vector scaling */
    target_simt_vector_scale(data1, N, 2.5f);
    printf("Checksum after scaling: %f\n", compute_checksum(data1, N));
    
    /* Test 2: Conditional execution within loop */
    target_simt_conditional_update(data2, N, THRESHOLD);
    printf("Checksum after conditional: %f\n", compute_checksum(data2, N));
    
    /* Test 3: Multiple arrays with complex control flow */
    target_simt_nested_control(data1, data2, result, N);
    printf("Checksum after nested control: %f\n", compute_checksum(result, N));
    
    /* Test 4: Mode-dependent transformation */
    for (int iter = 0; iter < 3; ++iter) {
        target_mixed_directives(data3, N, (mode + iter) % 2);
    }
    printf("Checksum after mixed directives: %f\n", compute_checksum(data3, N));
    
    /* Verify results aren't optimized away */
    float final_sum = compute_checksum(data1, N) + 
                     compute_checksum(data2, N) + 
                     compute_checksum(data3, N) + 
                     compute_checksum(result, N);
    printf("Final verification sum: %f\n", final_sum);
    
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
