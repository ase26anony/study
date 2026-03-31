/* Test program to trigger SIMT transformation in omp-low.cc lines 2941-2975 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary target region */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:dynamic_N], b[0:dynamic_N]) \
        map(from: result[0:dynamic_N]) \
        private(i) shared(a, b, result) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < dynamic_N; i++) {
        result[i] = a[i] + b[i];
    }
}

/* Alternative pattern with explicit nesting */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int rows = N / 16;
    volatile int cols = 16;
    
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        num_teams(rows)
    {
        #pragma omp distribute parallel for simd collapse(2) \
            private(i, j) shared(a, b, result)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                if (idx < N) {
                    result[idx] = a[idx] * b[idx];
                }
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    #pragma omp target map(to: a[0:size], b[0:size]) map(from: result[0:size])
    {
        #pragma omp parallel for simd
        for (int i = 0; i < size; i++) {
            result[i] = a[i] - b[i];
        }
    }
}

/* Helper function to verify results */
int verify_results(int N, float *cpu_result, float *gpu_result, float tolerance) {
    for (int i = 0; i < N; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > tolerance) {
            printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                   i, cpu_result[i], gpu_result[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    /* Use non-constant sizes to prevent compile-time optimization */
    int base_size = 1024;
    int sizes[] = {256, 512, 1024, 2048};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        printf("Testing with N = %d\n", N);
        
        /* Dynamic allocation prevents constant propagation */
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *result_gpu = (float *)malloc(N * sizeof(float));
        float *result_cpu = (float *)malloc(N * sizeof(float));
        
        if (!a || !b || !result_gpu || !result_cpu) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        /* Initialize with non-trivial patterns */
        for (int i = 0; i < N; i++) {
            a[i] = (float)(i % 100) * 0.1f;
            b[i] = (float)((i + 1) % 100) * 0.2f;
        }
        
        /* Compute reference on CPU */
        for (int i = 0; i < N; i++) {
            result_cpu[i] = a[i] + b[i];
        }
        
        /* Test 1: Basic SIMT pattern */
        printf("  Testing basic SIMT pattern...\n");
        compute_simt(N, a, b, result_gpu);
        
        if (!verify_results(N, result_cpu, result_gpu, 1e-6f)) {
            printf("  Basic SIMT test failed!\n");
        } else {
            printf("  Basic SIMT test passed\n");
        }
        
        /* Test 2: Nested pattern */
        printf("  Testing nested pattern...\n");
        compute_nested_simt(N, a, b, result_gpu);
        
        /* Update CPU reference for multiplication */
        for (int i = 0; i < N; i++) {
            result_cpu[i] = a[i] * b[i];
        }
        
        if (!verify_results(N, result_cpu, result_gpu, 1e-6f)) {
            printf("  Nested pattern test failed!\n");
        } else {
            printf("  Nested pattern test passed\n");
        }
        
        /* Test 3: Target simd pattern */
        printf("  Testing target simd pattern...\n");
        compute_target_simd(N, a, b, result_gpu);
        
        /* Update CPU reference for subtraction */
        for (int i = 0; i < N; i++) {
            result_cpu[i] = a[i] - b[i];
        }
        
        if (!verify_results(N, result_cpu, result_gpu, 1e-6f)) {
            printf("  Target simd test failed!\n");
        } else {
            printf("  Target simd test passed\n");
        }
        
        /* Clean up */
        free(a);
        free(b);
        free(result_gpu);
        free(result_cpu);
    }
    
    /* Additional test with conditional execution path */
    printf("\nTesting conditional execution path...\n");
    {
        int N = 512;
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *result = (float *)malloc(N * sizeof(float));
        
        for (int i = 0; i < N; i++) {
            a[i] = (float)i;
            b[i] = (float)(N - i);
        }
        
        /* Use conditional to potentially affect context decisions */
        int use_simt = 1;
        
        if (use_simt) {
            compute_simt(N, a, b, result);
        } else {
            /* Fallback CPU path */
            for (int i = 0; i < N; i++) {
                result[i] = a[i] + b[i];
            }
        }
        
        /* Verify */
        int correct = 1;
        for (int i = 0; i < N; i++) {
            if (fabs(result[i] - (a[i] + b[i])) > 1e-6f) {
                correct = 0;
                break;
            }
        }
        
        printf("Conditional test %s\n", correct ? "passed" : "failed");
        
        free(a);
        free(b);
        free(result);
    }
    
    /* Dead code with alternative syntax to expose compiler to different forms */
    if (0) {  /* Never executed, but parsed by compiler */
        int dummy_N = 64;
        float dummy_a[64], dummy_b[64], dummy_result[64];
        
        /* Alternative syntax 1: Combined construct with line continuation */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: dummy_result[0:dummy_N]) \
            num_teams(2)
        for (int i = 0; i < dummy_N; i++) {
            dummy_result[i] = i;
        }
        
        /* Alternative syntax 2: Separate pragmas */
        #pragma omp target teams distribute
        #pragma omp parallel for simd
        for (int i = 0; i < dummy_N; i++) {
            dummy_result[i] = i * 2;
        }
    }
    
    printf("\nAll tests completed\n");
    return 0;
}
