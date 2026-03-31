/* Test program to cover SIMT transformation in omp-low.cc
 * Specifically targets lines 2941-2975 in omp-low.cc.gcov
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -o test_simt test_simt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_add(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int n, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < n; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * data[i] / 3.14f;
        }
    }
}

__attribute__((noinline))
void target_simt_multiplication(float *arr, float factor, int n) {
    /* Using simd clause explicitly */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:n]) \
        num_teams(2) thread_limit(64)
    for (int i = 0; i < n; ++i) {
        arr[i] = arr[i] * factor + 1.0f;
    }
}

__attribute__((noinline))
void target_simt_nested_if(float *a, float *b, int n) {
    /* More complex control flow to exercise copy_gimple_seq_and_replace_locals */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:n]) map(tofrom: b[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            if (a[i] > 0.0f) {
                b[i] = b[i] * 2.0f;
            } else {
                b[i] = b[i] / 2.0f;
            }
        } else {
            b[i] = b[i] + a[i];
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
    
    /* Initialize test data */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.7f;
        c[i] = 0.0f;
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Call target functions multiple times with different configurations */
    for (int iter = 0; iter < 3; ++iter) {
        switch (test_case) {
            case 1:
                target_simt_vector_add(a, b, c, N);
                printf("Vector add checksum: %f\n", compute_checksum(c, N));
                break;
                
            case 2:
                target_simt_conditional_update(a, N, THRESHOLD);
                printf("Conditional update checksum: %f\n", compute_checksum(a, N));
                break;
                
            case 3:
                target_simt_multiplication(b, 3.14f, N);
                printf("Multiplication checksum: %f\n", compute_checksum(b, N));
                break;
                
            default:
                target_simt_nested_if(a, b, N);
                printf("Nested if checksum: %f\n", compute_checksum(b, N));
                break;
        }
        
        /* Vary loop bounds slightly to trigger different transformations */
        int small_n = N / (iter + 2);
        if (small_n > 0) {
            target_simt_vector_add(a, b, c, small_n);
            printf("Small vector add checksum: %f\n", compute_checksum(c, small_n));
        }
    }
    
    /* Test with structured data */
    struct Point {
        float x, y, z;
    };
    
    struct Point *points = (struct Point *)malloc(N * sizeof(struct Point));
    for (int i = 0; i < N; ++i) {
        points[i].x = (float)i;
        points[i].y = (float)i * 2.0f;
        points[i].z = (float)i * 0.5f;
    }
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: points[0:N]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < N; ++i) {
        points[i].x = points[i].x + points[i].y;
        points[i].y = points[i].y * points[i].z;
        points[i].z = sqrtf(points[i].x * points[i].x + points[i].y * points[i].y);
    }
    
    float struct_sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        struct_sum += points[i].x + points[i].y + points[i].z;
    }
    printf("Structured data checksum: %f\n", struct_sum);
    
    free(a);
    free(b);
    free(c);
    free(points);
    
    return 0;
}
