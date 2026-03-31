/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered block that generates
 * IFN_GOMP_USE_SIMT and restructures loops for GPU offloading.
 *
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -o test_simt test_simt_lowering.c
 * Run with: ./test_simt [kernel_type] [size]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEFAULT_SIZE 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void kernel_vector_scale(float *arr, int N) {
    /* Explicit simd clause to strongly hint at SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:N]) num_teams(4) thread_limit(128)
    for (int i = 0; i < N; ++i) {
        arr[i] = arr[i] * 3.14159f + 1.0f;
    }
}

__attribute__((noinline))
void kernel_conditional_update(float *data, int N, float threshold) {
    /* Complex control flow inside loop - increases GIMPLE sequence complexity */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:N]) map(to: threshold) num_teams(8)
    for (int i = 0; i < N; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void kernel_nested_if(float *a, float *b, float *c, int N) {
    /* Multiple nested conditionals to create more complex GIMPLE */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) num_teams(16)
    for (int i = 0; i < N; ++i) {
        float val = a[i] + b[i];
        if (val > 100.0f) {
            if (val < 200.0f) {
                c[i] = val * 2.0f;
            } else {
                c[i] = val * 1.5f;
            }
        } else {
            c[i] = val * 0.8f;
        }
    }
}

__attribute__((noinline))
void kernel_multi_array(float *x, float *y, float *z, int N) {
    /* Multiple arrays with different access patterns */
    #pragma omp target teams distribute parallel for simd \
        map(to: x[0:N], y[0:N]) map(tofrom: z[0:N]) num_teams(32)
    for (int i = 0; i < N; ++i) {
        z[i] = x[i] * y[i] + (i % 10) * 0.1f;
    }
}

/* Helper function to verify results */
float verify_sum(float *arr, int N) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int kernel_type = 0;  /* 0: scale, 1: conditional, 2: nested, 3: multi */
    int N = DEFAULT_SIZE;
    
    /* Parse command line arguments to vary execution paths */
    if (argc > 1) {
        kernel_type = atoi(argv[1]) % 4;
    }
    if (argc > 2) {
        N = atoi(argv[2]);
        if (N <= 0) N = DEFAULT_SIZE;
        if (N > 100000) N = 100000; /* Reasonable limit */
    }
    
    printf("Testing SIMT lowering with kernel_type=%d, N=%d\n", kernel_type, N);
    
    /* Allocate and initialize test data */
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)(i % 100) * 1.5f;
        data2[i] = (float)((i + 50) % 100) * 0.8f;
        data3[i] = 0.0f;
    }
    
    /* Execute multiple times with different kernels to increase coverage */
    for (int iter = 0; iter < 3; ++iter) {
        int current_kernel = (kernel_type + iter) % 4;
        
        switch (current_kernel) {
            case 0:
                printf("Iteration %d: Running vector_scale kernel\n", iter);
                kernel_vector_scale(data1, N);
                printf("  Checksum: %f\n", verify_sum(data1, N));
                break;
                
            case 1:
                printf("Iteration %d: Running conditional_update kernel\n", iter);
                kernel_conditional_update(data2, N, THRESHOLD);
                printf("  Checksum: %f\n", verify_sum(data2, N));
                break;
                
            case 2:
                printf("Iteration %d: Running nested_if kernel\n", iter);
                kernel_nested_if(data1, data2, data3, N);
                printf("  Checksum: %f\n", verify_sum(data3, N));
                break;
                
            case 3:
                printf("Iteration %d: Running multi_array kernel\n", iter);
                kernel_multi_array(data1, data2, data3, N);
                printf("  Checksum: %f\n", verify_sum(data3, N));
                break;
        }
        
        /* Reinitialize partially to create varying data patterns */
        if (iter < 2) {
            for (int i = 0; i < N; i += 7) {
                data1[i] += 10.0f;
                data2[i] *= 0.9f;
            }
        }
    }
    
    /* Final verification */
    float final_sum = verify_sum(data1, N) + verify_sum(data2, N) + verify_sum(data3, N);
    printf("Final combined checksum: %f\n", final_sum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
