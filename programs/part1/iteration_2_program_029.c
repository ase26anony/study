/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* Structure to test complex data mapping */
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
    
    /* This should trigger OMP_CLAUSE_INCLUSIVE and OMP_CLAUSE_EXCLUSIVE */
    double scan_temp = 0.0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:partial_sum) nowait
        for (int i = 0; i < n; i++) {
            #pragma omp scan inclusive(partial_sum)
            partial_sum += arr[i];
            prefix_sum[i] = partial_sum;
        }
        
        #pragma omp for reduction(+:scan_temp)
        for (int i = 0; i < n; i++) {
            #pragma omp scan exclusive(scan_temp)
            scan_temp += arr[i] * 0.5;
        }
    }
    
    printf("Scan test complete: final sum = %f, scan_temp = %f\n", 
           partial_sum, scan_temp);
}

/* Function 2: Enter data with to mapper */
void test_enter_data(struct DataBlock *data) {
    /* This should trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: data->values[0:N]) \
                           map(to: data->indices) \
                           map(to: data->result)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: data->values[0:N])
    for (int i = 0; i < N; i++) {
        data->values[i] = data->values[i] * 2.0 + i;
    }
    
    #pragma omp exit data map(from: data->result) \
                          map(release: data->values)
    
    printf("Enter data test: data->values[0] = %f\n", data->values[0]);
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* Complex nested pragmas to potentially generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel reduction(+:total) private(last_val)
    {
        #pragma omp for collapse(2) reduction(+:total) lastprivate(last_val) \
                    linear(i:1) nowait
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                total += matrix[i * cols + j];
                if (i == rows - 1 && j == cols - 1) {
                    last_val = matrix[i * cols + j];
                }
            }
        }
        
        /* Additional reduction with complex expression */
        double local_sum = 0.0;
        #pragma omp for reduction(+:local_sum) schedule(dynamic)
        for (int i = 0; i < rows * cols; i++) {
            local_sum += matrix[i] * (i % 10);
        }
        total += local_sum;
    }
    
    /* SIMD with conditionals - may generate _CONDTEMP_ */
    double simd_result = 0.0;
    #pragma omp simd reduction(+:simd_result) linear(i:1) \
                simdlen(8) if(rows > 100)
    for (int i = 0; i < rows * cols; i++) {
        if (matrix[i] > 0.5) {  /* Conditional inside SIMD */
            simd_result += matrix[i];
        } else {
            simd_result -= matrix[i] * 0.1;
        }
    }
    
    printf("Internal temporaries test: total = %f, simd_result = %f, last = %d\n",
           total, simd_result, last_val);
}

/* Function 4: Scan with temporaries - may generate _SCANTEMP_ */
void test_scan_temporaries(double *arr, int n) {
    double scan_array[N];
    double temp = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for simd reduction(inscan,+:temp)
        for (int i = 0; i < n; i++) {
            #pragma omp scan exclusive(temp)
            temp += arr[i];
            scan_array[i] = temp;
            
            /* Additional computation that might create scan temporaries */
            #pragma omp scan inclusive(temp)
            temp += arr[i] * 0.1;
        }
    }
    
    printf("Scan temporaries test: scan_array[%d] = %f\n", n-1, scan_array[n-1]);
}

/* Function 5: Target regions with complex data environment */
void test_target_complex(struct DataBlock *data, double *output) {
    /* Complex map clauses */
    #pragma omp target teams distribute parallel for \
                map(to: data[0:1]) \
                map(tofrom: output[0:N]) \
                map(alloc: data->indices[0:N/2]) \
                if(N > 500)
    for (int i = 0; i < N; i++) {
        output[i] = data->values[i] * data->indices[i % N];
        if (i % 100 == 0) {
            data->result += output[i];
        }
    }
    
    /* Nested target with reduction */
    #pragma omp target teams distribute parallel for reduction(+:data->result) \
                map(tofrom: data->result)
    for (int i = 0; i < N; i += 2) {
        data->result += output[i] * 0.5;
    }
    
    printf("Target complex test: data->result = %f\n", data->result);
}

int main() {
    /* Initialize test data */
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    struct DataBlock data_block;
    
    srand(42);
    for (int i = 0; i < N; i++) {
        array[i] = (double)rand() / RAND_MAX;
        output[i] = 0.0;
        data_block.values[i] = (double)rand() / RAND_MAX;
        data_block.indices[i] = i;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    data_block.result = 0.0;
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to trigger different OpenMP constructs */
    test_scan_clauses(array, N);
    test_enter_data(&data_block);
    test_internal_temporaries(matrix, N, M);
    test_scan_temporaries(array, N);
    test_target_complex(&data_block, output);
    
    /* Final reduction across all results */
    double final_result = data_block.result;
    #pragma omp parallel for reduction(+:final_result) if(N > 1000)
    for (int i = 0; i < N; i++) {
        final_result += output[i] * 0.01;
    }
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(output);
    
    return 0;
}
