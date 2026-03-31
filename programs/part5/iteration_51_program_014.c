/* simt_transformation_test.c
 * Designed to trigger SIMT transformation in GCC's omp-low.cc
 * Specifically targets lines 2941-2975 and IFN_GOMP_USE_SIMT generation
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_N = N;
    
    /* Primary target: teams distribute parallel for simd with explicit clauses */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: a[0:dynamic_N], b[0:dynamic_N], result[0:dynamic_N]) \
        private(i) shared(result) collapse(2) num_teams(128) thread_limit(64)
    for (int i = 0; i < dynamic_N; i++) {
        for (int j = 0; j < 16; j++) {  /* Inner loop for collapse(2) */
            int idx = i * 16 + j;
            if (idx < dynamic_N) {
                result[idx] = a[idx] * b[idx] + (float)(i + j);
            }
        }
    }
}

/* Alternative function with different nesting pattern */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    volatile int vol_N = N;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target map(tofrom: a[0:vol_N], b[0:vol_N], result[0:vol_N])
    {
        #pragma omp teams num_teams(64)
        {
            #pragma omp distribute parallel for simd \
                private(i) shared(a,b,result) schedule(static, 32)
            for (int i = 0; i < vol_N; i++) {
                result[i] = a[i] * 2.0f + b[i] * 3.0f;
            }
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int size = N;
    
    /* target simd in parallel context */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:size], b[0:size]) map(from: result[0:size]) \
        private(i) reduction(+:result[:size])
    for (int i = 0; i < size; i++) {
        result[i] = 0.0f;
        for (int k = 0; k < 8; k++) {
            result[i] += a[i] * b[i] * (float)k;
        }
    }
}

/* Verification function to prevent dead code elimination */
int verify_results(float *cpu_result, float *gpu_result, int N, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at index %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference implementation */
void compute_cpu_reference(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)(i + j);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent optimization */
    int base_size = 1024;
    if (argc > 1) {
        base_size = atoi(argv[1]);
    }
    
    /* Multiple sizes to test different contexts */
    int sizes[] = {base_size, base_size * 2, base_size / 2};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    int total_errors = 0;
    
    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        if (N <= 0) N = 256;
        
        printf("Testing with N = %d\n", N);
        
        /* Dynamic allocation prevents compile-time optimization */
        float *a = (float *)malloc(N * sizeof(float));
        float *b = (float *)malloc(N * sizeof(float));
        float *gpu_result = (float *)malloc(N * sizeof(float));
        float *cpu_result = (float *)malloc(N * sizeof(float));
        
        if (!a || !b || !gpu_result || !cpu_result) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        /* Initialize data */
        for (int i = 0; i < N; i++) {
            a[i] = (float)(i % 100) * 0.1f;
            b[i] = (float)((i + 37) % 100) * 0.2f;
            gpu_result[i] = 0.0f;
            cpu_result[i] = 0.0f;
        }
        
        /* Call from different contexts to affect current_function_decl */
        if (s % 2 == 0) {
            /* Primary SIMT transformation path */
            compute_simt(N, a, b, gpu_result);
        } else if (s % 3 == 0) {
            /* Alternative nested pattern */
            compute_nested_simt(N, a, b, gpu_result);
        } else {
            /* target simd variant */
            compute_target_simd(N, a, b, gpu_result);
        }
        
        /* Compute CPU reference */
        compute_cpu_reference(N, a, b, cpu_result);
        
        /* Verify results (prevents dead code elimination) */
        int errors = verify_results(cpu_result, gpu_result, N, 0.001f);
        total_errors += errors;
        
        if (errors == 0) {
            printf("  Test passed\n");
        } else {
            printf("  Test failed with %d errors\n", errors);
        }
        
        /* Free memory */
        free(a);
        free(b);
        free(gpu_result);
        free(cpu_result);
    }
    
    /* Conditional block with alternative constructs (never executed but parsed) */
    if (0) {
        /* This block will be parsed but not executed, exposing compiler to syntax */
        int dummy_N = 128;
        float dummy_a[128], dummy_b[128], dummy_result[128];
        
        /* Expose compiler to different syntactic forms */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: dummy_a, dummy_b, dummy_result)
        for (int i = 0; i < dummy_N; i++) {
            dummy_result[i] = dummy_a[i] + dummy_b[i];
        }
        
        /* Another variant with separate pragmas */
        #pragma omp target teams
        {
            #pragma omp distribute parallel for simd
            for (int i = 0; i < dummy_N; i++) {
                dummy_result[i] = dummy_a[i] * dummy_b[i];
            }
        }
    }
    
    printf("\nTotal errors across all tests: %d\n", total_errors);
    
    return total_errors > 0 ? 1 : 0;
}
