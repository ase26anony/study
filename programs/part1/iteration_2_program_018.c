/* Test program to trigger uncovered OpenMP clause pretty-printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 2000
#define M 100

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
    }
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:partial_sum)
        for (int i = 1; i < n; i++) {
            // This should generate scan clauses
            #pragma omp scan inclusive(partial_sum)
            partial_sum += arr[i];
            
            // Exclusive scan in same loop
            double temp = exclusive_sum;
            #pragma omp scan exclusive(exclusive_sum)
            exclusive_sum += arr[i-1];
            arr[i] += temp;
        }
    }
    
    printf("Scan results: inclusive=%f, exclusive=%f\n", partial_sum, exclusive_sum);
}

/* Function 2: Uses enter data with to clause */
void test_enter_data(struct DataBlock *block) {
    // Force generation of enter data with to clause
    #pragma omp target enter data map(to: block->values[0:N]) \
        map(to: block->indices) depend(inout: block)
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] *= 2.0;
        block->indices[i] = i;
    }
    
    #pragma omp target exit data map(from: block->values[0:N]) \
        map(release: block->indices)
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double last_val = 0.0;
    double reduction_sum = 0.0;
    
    // Complex nested loops with multiple clauses
    #pragma omp parallel for collapse(2) reduction(+:reduction_sum) \
        lastprivate(last_val) linear(i:1)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            matrix[idx] = (i + j) * 0.1;
            reduction_sum += matrix[idx];
            
            // Conditional inside parallel region
            if (matrix[idx] > 10.0) {
                #pragma omp atomic
                last_val += matrix[idx];
            }
        }
    }
    
    // Additional SIMD loop with conditionals
    #pragma omp simd reduction(+:reduction_sum) linear(i:1)
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] += 0.5;
        if (matrix[i] < 0) {
            matrix[i] = 0;
        }
        reduction_sum += matrix[i];
    }
    
    printf("Internal temps test: sum=%f, last=%f\n", reduction_sum, last_val);
}

/* Function 4: Mixed constructs with conditional compilation */
void test_mixed_constructs(struct DataBlock *block, int threshold) {
    double local_sum = 0.0;
    
    // Conditional parallel region
    #pragma omp parallel if(threshold > 1000) reduction(+:local_sum) \
        num_threads(4)
    {
        #pragma omp for nowait
        for (int i = 0; i < N; i++) {
            block->values[i] = i * 0.25;
            local_sum += block->values[i];
        }
        
        #pragma omp single
        {
            block->result = local_sum;
        }
    }
    
    // Target region with complex mapping
    #pragma omp target map(tofrom: block->values[0:N/2]) \
        map(to: block->indices[0:N/2]) if(threshold > 500)
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < N/2; i++) {
            block->values[i] += block->indices[i] * 0.01;
        }
    }
    
    printf("Mixed constructs: block result=%f\n", block->result);
}

/* Function 5: Task-based approach */
void test_task_temporaries(double *data, int n) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < n; i++) {
                #pragma omp task depend(inout: data[i]) \
                    firstprivate(i) shared(data)
                {
                    data[i] = omp_get_thread_num() * 100.0 + i;
                    
                    // Nested task to increase complexity
                    #pragma omp task if(i % 2 == 0)
                    {
                        data[i] *= 1.1;
                    }
                }
            }
        }
    }
}

int main() {
    printf("Starting OpenMP coverage test...\n");
    
    // Allocate test data
    double *array1 = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!array1 || !matrix || !block) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        block->values[i] = 0.0;
        block->indices[i] = 0;
    }
    block->result = 0.0;
    
    // Test all functions to trigger different OpenMP clauses
    test_scan_clauses(array1, N);
    test_enter_data(block);
    test_internal_temporaries(matrix, N, M);
    test_mixed_constructs(block, N);
    test_task_temporaries(array1, N/10);
    
    // Final reduction for verification
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + block->values[i];
    }
    
    // Add matrix sum
    #pragma omp parallel for reduction(+:final_sum) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            final_sum += matrix[i * M + j];
        }
    }
    
    printf("Final verification sum: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    // Cleanup
    free(array1);
    free(matrix);
    free(block);
    
    return 0;
}
