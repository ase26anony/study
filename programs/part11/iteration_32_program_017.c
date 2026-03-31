/* test_omp_simt.c - Program to trigger SIMT transformation in GCC's omp-low.cc */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Define a vector type to test mixed data type handling */
typedef float v4sf __attribute__((vector_size(16)));

/* Function with conditional SIMD execution */
void process_array_conditional(int use_simd, float* data, int n, float* sum) {
    if (use_simd) {
        /* This conditional SIMD block may trigger IFN_GOMP_USE_SIMT */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n]) map(tofrom: sum[0:1]) \
            reduction(+:sum[0])
        for (int i = 0; i < n; i++) {
            /* Complex data-dependent operation with potential early exit */
            if (data[i] > 1000.0f) {
                /* This break creates control flow that SIMT must handle */
                data[i] = 1000.0f;
            }
            data[i] = data[i] * 2.0f + 1.0f;
            sum[0] += data[i];
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < n; i++) {
            if (data[i] > 1000.0f) {
                data[i] = 1000.0f;
            }
            data[i] = data[i] * 2.0f + 1.0f;
            sum[0] += data[i];
        }
    }
}

/* Function with unconditional SIMD and complex access patterns */
void process_array_unconditional(float* data, int n, double* dbl_sum) {
    /* SIMD loop with safelen clause and non-unit stride */
    #pragma omp simd safelen(8) aligned(data: 16) linear(i:1) \
        reduction(+:dbl_sum[0])
    for (int i = 0; i < n/2; i++) {
        /* Non-contiguous access pattern */
        int idx = i * 2;
        if (idx < n) {
            /* Mixed precision computation */
            double temp = (double)data[idx] * 3.14159;
            data[idx] = (float)(temp * 2.0);
            dbl_sum[0] += temp;
            
            /* Additional conditional inside SIMD loop */
            if (data[idx] > 500.0f) {
                data[idx] = 500.0f;
            }
        }
    }
    
    /* Nested loop where inner loop is SIMD */
    #pragma omp parallel for simd collapse(2) private(v4sf)
    for (int i = 0; i < n/4; i++) {
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < n) {
                /* Using vector type operations */
                v4sf vec_data;
                for (int k = 0; k < 4; k++) {
                    vec_data[k] = data[idx] * (k + 1);
                }
                data[idx] = vec_data[0] + vec_data[1] + vec_data[2] + vec_data[3];
            }
        }
    }
}

/* Function with indirect indexing */
void process_indirect(float* data, int* indices, int n, float* result) {
    /* SIMD with indirect memory access pattern */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        int idx = indices[i % n];
        if (idx >= 0 && idx < n) {
            result[i] = data[idx] * 2.0f + (float)i;
        } else {
            result[i] = 0.0f;
        }
    }
}

int main(int argc, char** argv) {
    const int N = 1024;
    float* data = (float*)aligned_alloc(16, N * sizeof(float));
    float* result = (float*)aligned_alloc(16, N * sizeof(float));
    int* indices = (int*)malloc(N * sizeof(int));
    
    if (!data || !result || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        data[i] = (float)(i % 100);
        indices[i] = (i * 3) % N;
        result[i] = 0.0f;
    }
    
    /* Runtime condition to control SIMD execution */
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    
    float sum = 0.0f;
    double dbl_sum = 0.0;
    
    /* Process with conditional SIMD execution */
    process_array_conditional(use_simd, data, N, &sum);
    
    /* Always process with unconditional SIMD */
    process_array_unconditional(data, N, &dbl_sum);
    
    /* Process with indirect indexing */
    process_indirect(data, indices, N, result);
    
    /* Aggregate results to prevent dead code elimination */
    printf("Results:\n");
    printf("  Sum: %f\n", sum);
    printf("  Double Sum: %lf\n", dbl_sum);
    printf("  Sample data[0]: %f\n", data[0]);
    printf("  Sample data[100]: %f\n", data[100]);
    printf("  Sample result[50]: %f\n", result[50]);
    
    /* Verify results are reasonable */
    if (isnan(sum) || isinf(sum)) {
        fprintf(stderr, "Invalid sum detected\n");
    }
    
    free(data);
    free(result);
    free(indices);
    
    return 0;
}
