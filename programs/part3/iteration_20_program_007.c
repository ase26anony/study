/* Test program to cover SIMT transformation in omp-low.cc
 * Specifically targets lines 2941-2975 in omp-low.cc.gcov
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o simt_test simt_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline))
void target_simt_conditional(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for copy_gimple_seq_and_replace_locals */
        if (data[i] > threshold) {
            data[i] = data[i] * 0.5f;
        } else {
            data[i] = data[i] * 2.0f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(16) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        /* Nested control flow to create interesting GIMPLE sequence */
        float val = a[i] + b[i];
        if (val > 0) {
            if (val < 100.0f) {
                c[i] = val * val;
            } else {
                c[i] = val * 0.1f;
            }
        } else {
            c[i] = -val;
        }
    }
}

__attribute__((noinline))
void target_mixed_clauses(float *x, float *y, int size, int use_simd) {
    if (use_simd) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: x[0:size], y[0:size]) collapse(2)
        for (int i = 0; i < size/2; ++i) {
            for (int j = 0; j < 2; ++j) {
                int idx = i*2 + j;
                x[idx] = x[idx] + y[idx];
                y[idx] = x[idx] * 0.25f;
            }
        }
    } else {
        #pragma omp target teams distribute parallel for \
            map(tofrom: x[0:size], y[0:size])
        for (int i = 0; i < size; ++i) {
            x[i] = x[i] - y[i];
        }
    }
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    
    /* Initialize with test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(N - i);
        array3[i] = 0.0f;
    }
    
    /* Use command-line arguments to vary execution paths */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 4;
    }
    
    /* Execute different target regions to trigger various lowering paths */
    switch (test_case) {
        case 0:
            printf("Running vector scale with SIMD clause\n");
            target_simt_vector_scale(array1, N, 3.14f);
            break;
        case 1:
            printf("Running conditional transformation\n");
            target_simt_conditional(array1, N, THRESHOLD);
            break;
        case 2:
            printf("Running nested control flow\n");
            target_simt_nested_control(array1, array2, array3, N);
            break;
        case 3:
            printf("Running mixed clauses with collapse\n");
            target_mixed_clauses(array1, array2, N, 1);
            break;
    }
    
    /* Additional iterations to increase coverage */
    for (int iter = 0; iter < 3; ++iter) {
        int size = N / (iter + 1);
        if (size < 32) size = 32;
        
        #pragma omp target teams distribute parallel for \
            map(tofrom: array1[0:size]) if(size > 64)
        for (int i = 0; i < size; ++i) {
            array1[i] = array1[i] + (float)iter;
        }
    }
    
    /* Verify computation (prevent dead code elimination) */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += array1[i] + array3[i];
    }
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
