/* test_omp_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT transformation in GCC's
 * OpenMP lowering pass, specifically targeting the uncovered lines in
 * omp-low.cc that handle IFN_GOMP_USE_SIMT generation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline, target("noinline")))
void target_region_simple(float *arr, int size) {
    /* Simple vector scaling with explicit simd clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * 3.14159f + 2.71828f;
    }
}

__attribute__((noinline, target("noinline")))
void target_region_conditional(float *data, int size, float threshold) {
    /* More complex loop with conditional execution inside */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) map(to: threshold) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * data[i] / threshold;
        }
    }
}

__attribute__((noinline, target("noinline")))
void target_region_nested_if(float *a, float *b, float *c, int size) {
    /* Loop with multiple nested conditions to create complex GIMPLE */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(tofrom: c[0:size]) \
        num_teams(16) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        float val = a[i] + b[i];
        if (val > 0.0f) {
            if (val < 100.0f) {
                c[i] = val * 0.5f;
            } else {
                c[i] = val * 2.0f;
            }
        } else {
            c[i] = -val;
        }
        /* Additional operation to ensure non-trivial body */
        c[i] += (i % 16) * 0.01f;
    }
}

__attribute__((noinline, target("noinline")))
void target_region_multiple_clauses(float *x, float *y, float *z, int size) {
    /* Using multiple clauses to test clause processing */
    #pragma omp target teams distribute parallel for \
        map(to: x[0:size], y[0:size]) map(from: z[0:size]) \
        private(i) shared(x, y, z) reduction(+:sum) \
        num_teams(4) num_threads(32)
    for (int i = 0; i < size; ++i) {
        z[i] = x[i] * y[i];
        if (z[i] < 0) {
            z[i] = -z[i];
        }
    }
}

float compute_checksum(float *data, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to control execution path */
    int test_case = 1;
    int iterations = 2;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (test_case < 1 || test_case > 4) test_case = 1;
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = 1;
    }
    
    printf("Running test case %d for %d iterations\n", test_case, iterations);
    
    /* Allocate and initialize test data */
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)(i % 100);
        data2[i] = (float)((i * 3) % 100);
        data3[i] = 0.0f;
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < iterations; ++iter) {
        printf("Iteration %d: ", iter + 1);
        
        switch (test_case) {
            case 1:
                /* Simple SIMD case - most likely to trigger SIMT transformation */
                target_region_simple(data1, N);
                printf("Simple scaling - checksum: %.2f\n", compute_checksum(data1, N));
                break;
                
            case 2:
                /* Conditional execution case */
                target_region_conditional(data1, N, THRESHOLD);
                printf("Conditional - checksum: %.2f\n", compute_checksum(data1, N));
                break;
                
            case 3:
                /* Nested if statements */
                target_region_nested_if(data1, data2, data3, N);
                printf("Nested if - checksum: %.2f\n", compute_checksum(data3, N));
                break;
                
            case 4:
                /* Multiple clauses */
                target_region_multiple_clauses(data1, data2, data3, N);
                printf("Multiple clauses - checksum: %.2f\n", compute_checksum(data3, N));
                break;
        }
        
        /* Modify data slightly between iterations */
        for (int i = 0; i < N; ++i) {
            data1[i] += 0.1f;
            data2[i] += 0.05f;
        }
    }
    
    /* Additional test: Vary the problem size */
    printf("\nTesting varying problem sizes:\n");
    for (int size = 128; size <= 512; size *= 2) {
        float *temp = (float *)malloc(size * sizeof(float));
        for (int i = 0; i < size; ++i) {
            temp[i] = (float)i;
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: temp[0:size]) num_teams(2)
        for (int i = 0; i < size; ++i) {
            temp[i] = temp[i] * 2.0f + 1.0f;
        }
        
        printf("  Size %d: final value[0]=%.2f, [%d]=%.2f\n", 
               size, temp[0], size-1, temp[size-1]);
        
        free(temp);
    }
    
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
