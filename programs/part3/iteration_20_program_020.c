/* test_simt_lowering.c
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o test_simt test_simt_lowering.c
 * Run with: ./test_simt [kernel_type] [size]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define DEFAULT_SIZE 1024
#define NUM_KERNELS 3

/* Prevent inlining to keep target regions intact */
__attribute__((noinline, optimize("no-inline")))
void kernel_vector_add(float* a, float* b, float* c, int n, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n], scale) map(from: c[0:n]) \
        num_teams(8) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        /* Simple vectorizable operation with conditional */
        float temp = a[i] + b[i];
        if (temp > 0.0f) {
            c[i] = temp * scale;
        } else {
            c[i] = temp * (scale * 0.5f);
        }
    }
}

__attribute__((noinline, optimize("no-inline")))
void kernel_stencil_3pt(float* in, float* out, int n, float alpha) {
    #pragma omp target teams distribute parallel for simd \
        map(to: in[0:n], alpha) map(from: out[0:n]) \
        num_teams(16) thread_limit(64)
    for (int i = 1; i < n-1; ++i) {
        /* 3-point stencil with function call simulation */
        float left = in[i-1];
        float center = in[i];
        float right = in[i+1];
        
        /* Complex enough to generate interesting GIMPLE */
        out[i] = alpha * center + (1.0f - alpha) * (left + right) * 0.5f;
        
        /* Additional conditional to create control flow */
        if (out[i] > 1.0f) {
            out[i] = 1.0f / (1.0f + expf(-out[i]));
        }
    }
    
    /* Handle boundaries */
    out[0] = in[0];
    out[n-1] = in[n-1];
}

__attribute__((noinline, optimize("no-inline")))
void kernel_reduction_like(float* data, int n, int iter) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:n]) reduction(+:iter) \
        num_teams(4) thread_limit(256)
    for (int i = 0; i < n; ++i) {
        /* Loop with multiple statements to create complex sequence */
        float x = data[i];
        
        /* Polynomial evaluation - vectorizable */
        x = x * x * 0.5f + x * 2.0f + 1.0f;
        
        /* Conditional update with nested expression */
        if (i % 2 == 0) {
            data[i] = x * sinf((float)i * 0.01f);
        } else {
            data[i] = x * cosf((float)i * 0.01f);
        }
        
        /* Use the iteration parameter */
        data[i] += (float)iter * 0.001f;
    }
}

/* Helper to initialize arrays */
void init_array(float* arr, int n, float base, float step) {
    for (int i = 0; i < n; ++i) {
        arr[i] = base + step * i;
    }
}

/* Verification function */
float verify_sum(float* arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    int kernel_type = 0;
    int size = DEFAULT_SIZE;
    
    /* Parse command line arguments */
    if (argc > 1) {
        kernel_type = atoi(argv[1]) % NUM_KERNELS;
    }
    if (argc > 2) {
        size = atoi(argv[2]);
        if (size <= 0) size = DEFAULT_SIZE;
    }
    
    printf("Testing SIMT lowering with kernel %d, size %d\n", kernel_type, size);
    
    /* Allocate and initialize test data */
    float* a = (float*)malloc(size * sizeof(float));
    float* b = (float*)malloc(size * sizeof(float));
    float* c = (float*)malloc(size * sizeof(float));
    
    assert(a && b && c);
    
    init_array(a, size, 0.5f, 0.1f);
    init_array(b, size, -0.3f, 0.05f);
    
    /* Execute different kernels based on input */
    float checksum = 0.0f;
    
    for (int repeat = 0; repeat < 3; ++repeat) {
        switch (kernel_type) {
            case 0:
                kernel_vector_add(a, b, c, size, 1.5f + repeat * 0.1f);
                checksum = verify_sum(c, size);
                printf("Kernel 0, repeat %d: checksum = %f\n", repeat, checksum);
                break;
                
            case 1:
                init_array(a, size, 0.0f, 0.02f);
                kernel_stencil_3pt(a, c, size, 0.7f);
                checksum = verify_sum(c, size);
                printf("Kernel 1, repeat %d: checksum = %f\n", repeat, checksum);
                break;
                
            case 2:
                init_array(a, size, 1.0f, -0.01f);
                kernel_reduction_like(a, size, repeat);
                checksum = verify_sum(a, size);
                printf("Kernel 2, repeat %d: checksum = %f\n", repeat, checksum);
                break;
        }
        
        /* Vary size slightly to trigger different code paths */
        if (repeat == 1) {
            size = (size * 9) / 10;  /* 90% of original */
            free(a); free(b); free(c);
            a = (float*)malloc(size * sizeof(float));
            b = (float*)malloc(size * sizeof(float));
            c = (float*)malloc(size * sizeof(float));
            assert(a && b && c);
            init_array(a, size, 0.2f, 0.15f);
            init_array(b, size, -0.1f, 0.08f);
        }
    }
    
    /* Test all kernels in sequence for maximum coverage */
    if (argc > 3 && atoi(argv[3]) == 1) {
        printf("\nTesting all kernels sequentially:\n");
        int small_size = 256;
        float* test1 = (float*)malloc(small_size * sizeof(float));
        float* test2 = (float*)malloc(small_size * sizeof(float));
        float* test3 = (float*)malloc(small_size * sizeof(float));
        
        init_array(test1, small_size, 1.0f, 0.01f);
        init_array(test2, small_size, 0.5f, 0.02f);
        
        kernel_vector_add(test1, test2, test3, small_size, 2.0f);
        printf("  Kernel 0 result: %f\n", verify_sum(test3, small_size));
        
        kernel_stencil_3pt(test1, test3, small_size, 0.5f);
        printf("  Kernel 1 result: %f\n", verify_sum(test3, small_size));
        
        kernel_reduction_like(test1, small_size, 5);
        printf("  Kernel 2 result: %f\n", verify_sum(test1, small_size));
        
        free(test1); free(test2); free(test3);
    }
    
    free(a); free(b); free(c);
    
    printf("Test completed successfully.\n");
    return 0;
}
