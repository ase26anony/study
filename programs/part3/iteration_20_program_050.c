/* Test program to trigger SIMT transformation in omp-low.cc */
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
        /* Complex enough body with conditional to create interesting GIMPLE */
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
        /* Multiple conditions to create complex control flow */
        if (i % 2 == 0) {
            c[i] = a[i] + b[i];
        } else if (i % 3 == 0) {
            c[i] = a[i] - b[i];
        } else {
            c[i] = a[i] * b[i];
        }
    }
}

__attribute__((noinline))
void target_simt_multi_clause(float *arr, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) collapse(1) \
        num_teams(16) thread_limit(128) \
        private(size) shared(arr)
    for (int i = 0; i < size; ++i) {
        /* Vectorizable operation */
        arr[i] = arr[i] * 3.14159f + 2.71828f;
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
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with test data */
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(i * 2);
        array3[i] = (float)(i * 3);
        result[i] = 0.0f;
    }
    
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 4;
    }
    
    /* Execute different target regions based on input */
    for (int iter = 0; iter < 3; ++iter) {
        switch ((test_case + iter) % 4) {
            case 0:
                printf("Test 0: SIMD vector scaling\n");
                target_simt_vector_scale(array1, N, 2.5f);
                printf("Checksum: %f\n", compute_checksum(array1, N));
                break;
                
            case 1:
                printf("Test 1: Conditional update with threshold\n");
                target_simt_conditional_update(array2, N, THRESHOLD);
                printf("Checksum: %f\n", compute_checksum(array2, N));
                break;
                
            case 2:
                printf("Test 2: Nested control flow\n");
                target_simt_nested_control(array1, array2, result, N);
                printf("Checksum: %f\n", compute_checksum(result, N));
                break;
                
            case 3:
                printf("Test 3: Multiple clauses\n");
                target_simt_multi_clause(array3, N);
                printf("Checksum: %f\n", compute_checksum(array3, N));
                break;
        }
    }
    
    /* Test with different sizes to trigger different code paths */
    for (int size = 256; size <= 512; size += 256) {
        float *small_array = (float *)malloc(size * sizeof(float));
        for (int i = 0; i < size; ++i) {
            small_array[i] = (float)(i * i);
        }
        
        #pragma omp target teams distribute parallel for \
            map(tofrom: small_array[0:size])
        for (int i = 0; i < size; ++i) {
            small_array[i] = logf(small_array[i] + 1.0f);
        }
        
        printf("Size %d checksum: %f\n", size, compute_checksum(small_array, size));
        free(small_array);
    }
    
    free(array1);
    free(array2);
    free(array3);
    free(result);
    
    return 0;
}
