/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 2000
#define M 100

/* Structure to test complex data mappings */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Scan directives with inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double prefix_sum[N];
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        #pragma omp for
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
            #pragma omp scan inclusive(local_sum)
            prefix_sum[i] = local_sum;
        }
    }
    
    /* Also test exclusive scan */
    double exclusive_prefix[N];
    #pragma omp parallel
    {
        double local_sum = 0.0;
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp scan exclusive(local_sum)
            exclusive_prefix[i] = local_sum;
            local_sum += arr[i];
        }
    }
    
    printf("Scan test completed. Final sum: %f\n", partial_sum);
}

/* Function 2: Enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Create data on host */
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 0.5;
        block->indices[i] = i;
    }
    
    /* Use enter data with to clause */
    #pragma omp enter data map(to: block[:1]) \
        map(to: block->values[:N], block->indices[:N])
    
    /* Perform computation on target if available */
    #pragma omp target if(omp_get_num_devices() > 0) \
        map(tofrom: block->result) \
        map(to: block->values[:N])
    {
        block->result = 0.0;
        #pragma omp teams distribute parallel for reduction(+:block->result)
        for (int i = 0; i < N; i++) {
            block->result += block->values[i];
        }
    }
    
    #pragma omp exit data map(from: block->result) \
        map(release: block->values[:N], block->indices[:N])
    
    printf("Enter data test completed. Result: %f\n", block->result);
}

/* Function 3: Complex loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    double last_val = 0.0;
    
    /* Complex parallel loop with multiple clauses */
    #pragma omp parallel for reduction(+:total) lastprivate(last_val) \
        linear(i:1) collapse(2) if(rows > 100)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double val = matrix[i * cols + j];
            total += val;
            if (i == rows - 1 && j == cols - 1) {
                last_val = val;
            }
        }
    }
    
    /* Nested parallelism with reduction */
    #pragma omp parallel reduction(+:total)
    {
        #pragma omp for nowait
        for (int i = 0; i < rows; i++) {
            double row_sum = 0.0;
            #pragma omp simd reduction(+:row_sum) linear(j:1)
            for (int j = 0; j < cols; j++) {
                row_sum += matrix[i * cols + j];
            }
            total += row_sum;
        }
    }
    
    printf("Internal temporaries test completed. Total: %f, Last: %f\n", 
           total, last_val);
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ */
void test_simd_conditionals(double *a, double *b, double *c, int n) {
    #pragma omp parallel for simd simdlen(4) if(n > 500)
    for (int i = 0; i < n; i++) {
        /* Complex conditional to potentially generate _CONDTEMP_ */
        if (i % 3 == 0) {
            c[i] = a[i] * b[i];
        } else if (i % 3 == 1) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
    
    /* Another SIMD with reduction */
    double sum = 0.0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    
    printf("SIMD conditionals test completed. Sum: %f\n", sum);
}

/* Function 5: Target regions with complex mappings */
void test_target_complex(double *input, double *output, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: input[:size]) map(from: output[:size]) \
        if(size > 1000)
    for (int i = 0; i < size; i++) {
        output[i] = input[i] * input[i];
    }
    
    /* Nested target region */
    double reduction_result = 0.0;
    #pragma omp target teams distribute parallel for \
        reduction(+:reduction_result) \
        map(to: output[:size]) map(tofrom: reduction_result)
    for (int i = 0; i < size; i++) {
        reduction_result += output[i];
    }
    
    printf("Target complex test completed. Reduction: %f\n", reduction_result);
}

int main() {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize test data */
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    double *input = (double*)malloc(N * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    
    struct DataBlock block;
    
    for (int i = 0; i < N; i++) {
        array[i] = (i + 1) * 0.1;
        a[i] = i * 0.2;
        b[i] = i * 0.3;
        input[i] = i * 0.4;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (i % 100) * 0.01;
    }
    
    /* Call all test functions */
    test_scan_clauses(array, N);
    test_enter_data(&block);
    test_internal_temporaries(matrix, N, M);
    test_simd_conditionals(a, b, c, N);
    test_target_complex(input, output, N);
    
    /* Final reduction across all results */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
        schedule(dynamic, 16) if(N > 100)
    for (int i = 0; i < N; i++) {
        final_result += array[i] + c[i] + output[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(a);
    free(b);
    free(c);
    free(input);
    free(output);
    
    return 0;
}
