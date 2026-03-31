/* test_openmp_clauses.c
 * This program exercises various OpenMP clauses to trigger
 * pretty-printing of internal clause representations during
 * tree dumps.
 */

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
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        // Test scan inclusive clause
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        
        // Test scan exclusive clause  
        #pragma omp scan exclusive(exclusive_sum)
        exclusive_sum += arr[i] * 0.5;
        
        arr[i] = prefix_sum + exclusive_sum;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    // Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices)        \
                           map(alloc: block->result)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N/2])    \
                map(to: block->indices)
    for (int i = 0; i < N/2; i++) {
        block->values[i] = block->values[i] * 2.0 + block->indices[i];
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values[0:N/2])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    // This complex combination should generate _LOOPTEMP_, _REDUCTEMP_
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) \
                linear(i:1) schedule(dynamic, 16)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double temp = matrix[i * cols + j];
            
            // Conditional inside loop may generate _CONDTEMP_
            if (temp > 0.5) {
                total += temp * temp;
            } else {
                total += temp;
            }
            
            // Nested OpenMP region to increase complexity
            #pragma omp simd reduction(+:total) linear(k:1)
            for (int k = 0; k < 10; k++) {
                total += 0.001 * k;
            }
            
            last_val = i * cols + j;
        }
    }
    
    // Additional SIMD loop with conditionals
    #pragma omp simd reduction(+:total) linear(i:1)
    for (int i = 0; i < rows * cols; i++) {
        // Complex conditional may generate _SCANTEMP_
        double val = matrix[i];
        #pragma omp scan inclusive(total)
        total += (val > 0) ? val : -val;
    }
}

/* Function 4: Mixed directives with if clauses */
void test_mixed_directives(struct DataBlock *block, int threshold) {
    // Conditional parallel region
    #pragma omp parallel if(block->result > threshold) \
                num_threads(4) default(none) shared(block)
    {
        #pragma omp for nowait reduction(+:block->result) \
                    schedule(guided)
        for (int i = 0; i < N; i++) {
            block->result += block->values[i];
        }
        
        #pragma omp single
        {
            // Nested worksharing
            #pragma omp taskloop grainsize(64) \
                        reduction(+:block->result)
            for (int i = 0; i < M; i++) {
                block->result += i * 0.1;
            }
        }
    }
    
    // Target region with complex map clauses
    #pragma omp target map(tofrom: block->result) \
                map(to: block->values) if(N > 1000)
    {
        #pragma omp teams distribute parallel for \
                    reduction(+:block->result)
        for (int i = 0; i < N; i++) {
            block->result += block->values[i] * 0.5;
        }
    }
}

/* Function 5: Test task reduction with scan */
void test_task_reduction(double *arr, int n) {
    double task_sum = 0.0;
    
    #pragma omp parallel
    #pragma omp single
    {
        for (int i = 0; i < n; i += 64) {
            int chunk_size = (i + 64 > n) ? n - i : 64;
            
            #pragma omp task firstprivate(i, chunk_size) \
                        reduction(+:task_sum)            \
                        depend(inout: arr[i])
            {
                double local_sum = 0.0;
                for (int j = 0; j < chunk_size; j++) {
                    local_sum += arr[i + j];
                    
                    // Scan directive in task
                    #pragma omp scan inclusive(local_sum)
                    arr[i + j] = local_sum;
                }
                task_sum += local_sum;
            }
        }
    }
}

int main() {
    // Initialize data
    struct DataBlock block;
    double *matrix = (double*)malloc(N * M * sizeof(double));
    double *array = (double*)malloc(N * sizeof(double));
    
    srand(42);
    for (int i = 0; i < N; i++) {
        block.values[i] = (double)rand() / RAND_MAX;
        block.indices[i] = i;
        array[i] = (double)rand() / RAND_MAX;
    }
    block.result = 0.0;
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    printf("Starting OpenMP tests...\n");
    
    // Call all test functions to trigger various OpenMP clauses
    test_scan_clauses(array, N);
    test_enter_data(&block);
    test_internal_temporaries(matrix, N, M);
    test_mixed_directives(&block, 500);
    test_task_reduction(array, N);
    
    // Final computation and output
    double final_result = block.result;
    for (int i = 0; i < N; i++) {
        final_result += array[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    free(matrix);
    free(array);
    
    return 0;
}
