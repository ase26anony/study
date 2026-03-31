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
#define NUM_KERNELS 3

/* Prevent inlining to keep target regions intact for lowering */
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
void kernel_conditional_scale(float *data, float threshold, float scale, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) map(to: threshold, scale) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < n; ++i) {
        /* Complex enough control flow to create interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = data[i] * scale;
        } else {
            data[i] = data[i] / scale;
        }
    }
}

__attribute__((noinline))
void kernel_stencil_3pt(float *in, float *out, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: in[0:n]) map(from: out[0:n]) \
        num_teams(16) thread_limit(256)
    for (int i = 1; i < n - 1; ++i) {
        /* Simple stencil computation - vectorizable */
        out[i] = (in[i-1] + in[i] + in[i+1]) * 0.333333f;
    }
    /* Handle boundaries on host */
    out[0] = in[0];
    out[n-1] = in[n-1];
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
    int kernel_type = 0;  /* 0: vector_add, 1: conditional_scale, 2: stencil */
    int size = DEFAULT_SIZE;
    
    /* Parse command line arguments to vary execution paths */
    if (argc > 1) {
        kernel_type = atoi(argv[1]) % NUM_KERNELS;
    }
    if (argc > 2) {
        size = atoi(argv[2]);
        if (size < 16) size = 16;
        if (size > 65536) size = 65536;
    }
    
    printf("Testing SIMT lowering with kernel %d, size %d\n", kernel_type, size);
    
    /* Allocate and initialize test data */
    float *a = (float *)malloc(size * sizeof(float));
    float *b = (float *)malloc(size * sizeof(float));
    float *c = (float *)malloc(size * sizeof(float));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < size; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(size - i) * 0.05f;
        c[i] = 0.0f;
    }
    
    /* Execute different kernels based on input */
    switch (kernel_type) {
        case 0:
            printf("Running vector addition kernel\n");
            kernel_vector_add(a, b, c, size);
            printf("Verification sum: %f\n", verify_sum(c, size));
            break;
            
        case 1:
            printf("Running conditional scaling kernel\n");
            kernel_conditional_scale(a, 5.0f, 2.0f, size);
            printf("Verification sum: %f\n", verify_sum(a, size));
            break;
            
        case 2:
            printf("Running 3-point stencil kernel\n");
            kernel_stencil_3pt(a, c, size);
            printf("Verification sum: %f\n", verify_sum(c, size));
            break;
    }
    
    /* Additional coverage: loop over small sizes to trigger different
     * compilation paths for the same kernel */
    printf("\nTesting multiple sizes for coverage:\n");
    for (int test_size = 64; test_size <= 256; test_size *= 2) {
        float *test_arr = (float *)malloc(test_size * sizeof(float));
        for (int i = 0; i < test_size; ++i) {
            test_arr[i] = (float)i;
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: test_arr[0:test_size])
        for (int i = 0; i < test_size; ++i) {
            test_arr[i] = test_arr[i] * 3.14f + 1.0f;
        }
        
        float sum = verify_sum(test_arr, test_size);
        printf("  Size %d: sum = %f\n", test_size, sum);
        free(test_arr);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    printf("Test completed successfully\n");
    return 0;
}
