/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline, optimize("no-inline")))
void target_simt_vector_add(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline, optimize("no-inline")))
void target_simt_conditional_update(float *data, int n, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) map(to: threshold) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < n; ++i) {
        /* Complex enough body to generate interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * data[i] / 3.14f;
        }
    }
}

__attribute__((noinline, optimize("no-inline")))
void target_nested_simt(float *a, float *b, float *c, int n, int mode) {
    /* Multiple SIMD clauses and complex mapping */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], mode) map(from: c[0:n]) \
        collapse(2) num_teams(16)
    for (int i = 0; i < n/2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i*2 + j;
            if (mode == 0) {
                c[idx] = a[idx] * b[idx] - 1.0f;
            } else {
                c[idx] = a[idx] + b[idx] * 2.0f;
            }
        }
    }
}

__attribute__((noinline, optimize("no-inline")))
void target_with_structured_data(struct point {
    float x, y, z;
} *points, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: points[0:n]) \
        num_teams(4) thread_limit(64)
    for (int i = 0; i < n; ++i) {
        /* Vectorizable operations on structured data */
        points[i].x = points[i].x * 1.5f;
        points[i].y = points[i].y + 2.0f;
        points[i].z = sqrtf(points[i].z * points[i].z);
    }
}

float verify_sum(float *data, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int test_size = N;
    int num_iterations = 1;
    
    /* Use command line to vary parameters */
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = N;
    }
    if (argc > 2) {
        num_iterations = atoi(argv[2]);
        if (num_iterations <= 0) num_iterations = 1;
    }
    
    printf("Testing SIMT transformation with size=%d, iterations=%d\n", 
           test_size, num_iterations);
    
    /* Allocate test arrays */
    float *a = (float *)malloc(test_size * sizeof(float));
    float *b = (float *)malloc(test_size * sizeof(float));
    float *c = (float *)malloc(test_size * sizeof(float));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < test_size; ++i) {
        a[i] = (float)i;
        b[i] = (float)(test_size - i);
        c[i] = 0.0f;
    }
    
    /* Run multiple target regions with different configurations */
    for (int iter = 0; iter < num_iterations; ++iter) {
        printf("Iteration %d:\n", iter + 1);
        
        /* Test 1: Simple vector addition with simd clause */
        target_simt_vector_add(a, b, c, test_size);
        float sum1 = verify_sum(c, test_size);
        printf("  Vector add checksum: %f\n", sum1);
        
        /* Test 2: Conditional update without explicit simd clause */
        memcpy(c, a, test_size * sizeof(float));
        target_simt_conditional_update(c, test_size, THRESHOLD);
        float sum2 = verify_sum(c, test_size);
        printf("  Conditional update checksum: %f\n", sum2);
        
        /* Test 3: Nested loop with collapse and simd */
        if (test_size >= 4) {
            target_nested_simt(a, b, c, test_size, iter % 2);
            float sum3 = verify_sum(c, test_size);
            printf("  Nested SIMD checksum: %f\n", sum3);
        }
        
        /* Test 4: Structured data */
        if (iter == 0) {
            struct point *points = (struct point *)malloc(test_size * sizeof(struct point));
            for (int i = 0; i < test_size; ++i) {
                points[i].x = (float)i;
                points[i].y = (float)i * 2.0f;
                points[i].z = (float)i * 3.0f;
            }
            target_with_structured_data(points, test_size);
            float struct_sum = 0.0f;
            for (int i = 0; i < test_size; ++i) {
                struct_sum += points[i].x + points[i].y + points[i].z;
            }
            printf("  Structured data checksum: %f\n", struct_sum);
            free(points);
        }
        
        /* Vary data slightly each iteration */
        for (int i = 0; i < test_size; ++i) {
            a[i] += 0.1f;
            b[i] -= 0.05f;
        }
    }
    
    /* Final verification */
    printf("\nFinal verification:\n");
    target_simt_vector_add(a, b, c, test_size);
    float final_sum = verify_sum(c, test_size);
    printf("Final checksum: %f\n", final_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
