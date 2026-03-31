/* Test program to trigger uncovered OpenMP clause pretty-printing logic */
#include <stdio.h>
#include <stdlib.h>

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
    double prefix_sum[N] = {0};
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
        
        /* Scan directive with inclusive clause */
        #pragma omp scan inclusive(partial_sum)
        prefix_sum[i] = partial_sum;
    }
    
    /* Reset and test exclusive scan */
    partial_sum = 0.0;
    double exclusive_prefix[N] = {0};
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        double temp = arr[i] * 2.0;
        
        /* Scan directive with exclusive clause */
        #pragma omp scan exclusive(partial_sum)
        exclusive_prefix[i] = partial_sum;
        partial_sum += temp;
    }
}

/* Function 2: Enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Enter data construct with to clause */
    #pragma omp enter data map(to: block->values[0:N]) \
                           map(to: block->indices) \
                           map(to: block->result)
    
    /* Use the data on target */
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + i;
    }
    
    #pragma omp exit data map(from: block->values[0:N]) \
                          map(from: block->result)
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *a, double *b, double *c, int n) {
    double last_val = 0.0;
    double reduction_sum = 0.0;
    
    /* Complex parallel loop with multiple clauses */
    #pragma omp parallel for simd reduction(+:reduction_sum) \
                lastprivate(last_val) linear(i:1) collapse(2) \
                schedule(dynamic, 16)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            c[idx] = a[idx] + b[idx];
            reduction_sum += c[idx];
            
            /* Conditional inside loop to potentially generate _CONDTEMP_ */
            if (c[idx] > 100.0) {
                c[idx] = 100.0;
            }
        }
        last_val = c[i * M + (M-1)];
    }
    
    /* Nested parallelism with reduction */
    #pragma omp parallel reduction(+:reduction_sum)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            reduction_sum += a[i];
        }
        
        /* Another loop with different clauses */
        #pragma omp for reduction(max:reduction_sum) lastprivate(last_val)
        for (int i = 0; i < n; i++) {
            if (b[i] > last_val) {
                last_val = b[i];
            }
            reduction_sum = (reduction_sum > b[i]) ? reduction_sum : b[i];
        }
    }
}

/* Function 4: SIMD with conditionals */
void test_simd_conditionals(double *arr, int n) {
    #pragma omp simd linear(i:1) reduction(+:arr[:n])
    for (int i = 0; i < n; i++) {
        /* Complex conditional to potentially generate _CONDTEMP_ */
        arr[i] = (i % 2 == 0) ? arr[i] * 2.0 : arr[i] / 2.0;
        
        if (arr[i] < 0) {
            arr[i] = -arr[i];
        }
    }
}

/* Function 5: Target regions with complex data clauses */
void test_target_regions(struct DataBlock *block, double *output) {
    /* Target with nested loops and complex mapping */
    #pragma omp target teams distribute parallel for \
                map(to: block->values[0:N]) \
                map(from: output[0:N]) \
                map(alloc: block->indices[0:N/2]) \
                reduction(+:block->result)
    for (int i = 0; i < N; i++) {
        output[i] = block->values[i] * 3.14;
        block->result += output[i];
        
        /* Conditional to generate internal temporaries */
        if (i < N/2) {
            block->indices[i] = i;
        }
    }
}

/* Main function orchestrating all tests */
int main() {
    /* Allocate and initialize test data */
    double *array1 = (double*)malloc(N * M * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    double *array3 = (double*)malloc(N * M * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    /* Initialize arrays */
    for (int i = 0; i < N * M; i++) {
        array1[i] = (double)i / 10.0;
        array2[i] = (double)(i % 100) / 5.0;
        array3[i] = 0.0;
    }
    
    for (int i = 0; i < N; i++) {
        block->values[i] = (double)i / 20.0;
        block->indices[i] = 0;
        output[i] = 0.0;
    }
    block->result = 0.0;
    
    /* Conditional compilation path */
    int use_parallel = 1;
    
    /* Test 1: Scan clauses */
    if (use_parallel) {
        #pragma omp parallel if(N>100)
        {
            test_scan_clauses(block->values, N);
        }
    }
    
    /* Test 2: Enter data with to mapper */
    test_enter_data(block);
    
    /* Test 3: Complex loops for internal temporaries */
    #pragma omp parallel for if(N*M > 10000) schedule(guided)
    for (int chunk = 0; chunk < 4; chunk++) {
        int start = chunk * (N * M / 4);
        int end = (chunk == 3) ? N * M : (chunk + 1) * (N * M / 4);
        test_internal_temporaries(array1 + start, array2 + start, 
                                 array3 + start, end - start);
    }
    
    /* Test 4: SIMD with conditionals */
    test_simd_conditionals(block->values, N);
    
    /* Test 5: Target regions */
    #pragma omp target data map(tofrom: block[0:1])
    {
        test_target_regions(block, output);
    }
    
    /* Compute final result for verification */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                lastprivate(use_parallel)
    for (int i = 0; i < N; i++) {
        final_result += output[i] + block->values[i];
        if (i == N-1) {
            use_parallel = 0;
        }
    }
    
    final_result += block->result;
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(output);
    free(block);
    
    return 0;
}
