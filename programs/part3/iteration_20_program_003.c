/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines that generate
 * IFN_GOMP_USE_SIMT and restructure loops for GPU offloading.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_simt test_simt_lowering.c
 * 
 * Run with: ./test_simt [kernel_type] [size]
 *   kernel_type: 0=vector add, 1=conditional update, 2=scaling with math
 *   size: array size (default 1024)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define DEFAULT_SIZE 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void kernel_vector_add(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline)) 
void kernel_conditional_update(float *data, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < n; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > THRESHOLD) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f + 1.0f;
        }
    }
}

__attribute__((noinline))
void kernel_scaling_with_math(float *arr, int n, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:n]) \
        num_teams(2) thread_limit(256)
    for (int i = 0; i < n; ++i) {
        /* Vectorizable math operations */
        arr[i] = sinf(arr[i]) * scale + cosf(arr[i]);
    }
}

/* Helper to verify results */
float verify_sum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int kernel_type = 0;
    int n = DEFAULT_SIZE;
    
    /* Parse command line arguments to vary execution paths */
    if (argc > 1) kernel_type = atoi(argv[1]);
    if (argc > 2) n = atoi(argv[2]);
    if (n <= 0) n = DEFAULT_SIZE;
    
    printf("Testing SIMT lowering with kernel_type=%d, size=%d\n", 
           kernel_type, n);
    
    /* Allocate and initialize test data */
    float *a = (float *)malloc(n * sizeof(float));
    float *b = (float *)malloc(n * sizeof(float));
    float *c = (float *)malloc(n * sizeof(float));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < n; ++i) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)i * 0.7f;
        c[i] = 0.0f;
    }
    
    /* Execute different kernels based on input to increase coverage */
    switch (kernel_type) {
        case 0:
            printf("Running vector addition kernel\n");
            kernel_vector_add(a, b, c, n);
            printf("Result checksum: %f\n", verify_sum(c, n));
            break;
            
        case 1:
            printf("Running conditional update kernel\n");
            /* Initialize with varied data */
            for (int i = 0; i < n; ++i) {
                c[i] = (float)(i % 1000);
            }
            kernel_conditional_update(c, n);
            printf("Result checksum: %f\n", verify_sum(c, n));
            break;
            
        case 2:
            printf("Running scaling with math kernel\n");
            for (int i = 0; i < n; ++i) {
                c[i] = (float)i * 0.01f;
            }
            kernel_scaling_with_math(c, n, 3.14f);
            printf("Result checksum: %f\n", verify_sum(c, n));
            break;
            
        default:
            /* Run all kernels sequentially for maximum coverage */
            printf("Running all kernels sequentially\n");
            for (int iter = 0; iter < 3; ++iter) {
                printf("  Iteration %d: ", iter);
                for (int i = 0; i < n; ++i) {
                    a[i] = (float)i * 1.5f;
                    b[i] = (float)i * 0.7f;
                    c[i] = (float)(i % 1000);
                }
                
                if (iter == 0) {
                    kernel_vector_add(a, b, c, n);
                    printf("Vector add checksum: %f\n", verify_sum(c, n));
                } else if (iter == 1) {
                    kernel_conditional_update(c, n);
                    printf("Conditional update checksum: %f\n", verify_sum(c, n));
                } else {
                    kernel_scaling_with_math(c, n, 2.0f);
                    printf("Scaling checksum: %f\n", verify_sum(c, n));
                }
            }
            break;
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    printf("Test completed successfully\n");
    return 0;
}
