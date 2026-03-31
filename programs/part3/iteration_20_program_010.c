/* Test program to cover SIMT transformation in omp-low.cc */
/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o test_simt test_simt.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
void target_simt_conditional_scale(float *data, float scale, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) map(to: scale)
    for (int i = 0; i < n; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > THRESHOLD) {
            data[i] = data[i] * scale * 2.0f;
        } else {
            data[i] = data[i] * scale * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_if(float *arr, int *mask, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:n]) map(to: mask[0:n])
    for (int i = 0; i < n; ++i) {
        /* Nested conditionals to create more complex GIMPLE */
        if (mask[i] == 1) {
            if (arr[i] > 0) {
                arr[i] = arr[i] * 3.14f;
            } else {
                arr[i] = arr[i] * 1.5f;
            }
        } else if (mask[i] == 2) {
            arr[i] = arr[i] + 100.0f;
        }
    }
}

__attribute__((noinline))
void target_multiple_clauses(float *a, float *b, float *c, int n, float alpha) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], alpha) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        c[i] = alpha * a[i] + b[i];
    }
}

float verify_sum(float *arr, int n) {
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
    int *mask = (int *)malloc(N * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
        d[i] = (float)(i * 3);
        mask[i] = i % 3; /* 0, 1, or 2 */
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Loop over different sizes to increase coverage */
    int sizes[] = {256, 512, 1024};
    int num_sizes = 3;
    
    for (int s = 0; s < num_sizes; ++s) {
        int current_n = sizes[s];
        
        switch (test_case) {
            case 1:
                /* Basic SIMD clause with teams distribute parallel for */
                target_simt_vector_add(a, b, c, current_n);
                printf("Case 1, size %d: sum = %f\n", current_n, verify_sum(c, current_n));
                break;
                
            case 2:
                /* Conditional execution within loop */
                target_simt_conditional_scale(d, 1.5f, current_n);
                printf("Case 2, size %d: sum = %f\n", current_n, verify_sum(d, current_n));
                break;
                
            case 3:
                /* Nested conditionals */
                target_simt_nested_if(a, mask, current_n);
                printf("Case 3, size %d: sum = %f\n", current_n, verify_sum(a, current_n));
                break;
                
            case 4:
                /* Multiple clauses including simd */
                target_multiple_clauses(a, b, c, current_n, 2.0f);
                printf("Case 4, size %d: sum = %f\n", current_n, verify_sum(c, current_n));
                break;
                
            default:
                /* Run all tests sequentially */
                target_simt_vector_add(a, b, c, current_n);
                printf("All tests, size %d, vector_add: sum = %f\n", current_n, verify_sum(c, current_n));
                
                target_simt_conditional_scale(d, 1.5f, current_n);
                printf("All tests, size %d, conditional_scale: sum = %f\n", current_n, verify_sum(d, current_n));
                
                target_simt_nested_if(a, mask, current_n);
                printf("All tests, size %d, nested_if: sum = %f\n", current_n, verify_sum(a, current_n));
                
                target_multiple_clauses(a, b, c, current_n, 2.0f);
                printf("All tests, size %d, multiple_clauses: sum = %f\n", current_n, verify_sum(c, current_n));
                break;
        }
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(mask);
    
    return 0;
}
