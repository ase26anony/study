/* test_omp_clause_coverage.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for clauses: inclusive, exclusive, enter, and internal temporaries.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -o test test_omp_clause_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 500

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Tests scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        // Test inclusive scan
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        
        // Test exclusive scan
        double temp = arr[i] * 2.0;
        #pragma omp scan exclusive(exclusive_sum)
        exclusive_sum += temp;
        
        arr[i] = prefix_sum + exclusive_sum;
    }
}

/* Function 2: Tests enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    // Create device data environment with to mapper
    #pragma omp enter data map(to: block[0:1]) \
        map(to: block->values[0:N]) \
        map(to: block->indices[0:N])
    
    // Use the data on device
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[0:N]) \
        map(to: block->indices[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] += block->indices[i] * 0.5;
    }
    
    #pragma omp exit data map(from: block[0:1]) \
        map(from: block->values[0:N])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    // This complex construct should generate _LOOPTEMP_, _REDUCTEMP_, etc.
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) \
        linear(i:1) schedule(dynamic, 16)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double val = matrix[i * cols + j];
            total += val;
            
            // Conditional inside loop may generate _CONDTEMP_
            if (val > 0.5) {
                #pragma omp atomic
                last_val = i * cols + j;
            }
        }
    }
    
    // Additional SIMD loop with conditionals
    #pragma omp simd reduction(+:total) linear(i:1) \
        simdlen(8) safelen(16)
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = matrix[i] / total;
    }
}

/* Function 4: Target region with complex data clauses */
void test_target_region(struct DataBlock *block, double *output) {
    double local_sum = 0.0;
    
    #pragma omp target teams distribute parallel for \
        map(to: block->values[0:N]) \
        map(from: output[0:M]) \
        reduction(+:local_sum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < M; i++) {
        double sum = 0.0;
        for (int j = 0; j < 10; j++) {
            sum += block->values[(i + j) % N];
        }
        output[i] = sum;
        local_sum += sum;
    }
    
    block->result = local_sum;
}

/* Function 5: Mixed directives with if clauses */
void test_conditional_pragmas(double *data, int size, int threshold) {
    // Parallel region with conditional compilation
    #pragma omp parallel if(size > threshold) num_threads(4)
    {
        #pragma omp for nowait
        for (int i = 0; i < size; i++) {
            data[i] = data[i] * 2.0;
        }
        
        #pragma omp barrier
        
        // Task with depend clauses
        #pragma omp task depend(inout: data[0:size/2])
        {
            for (int i = 0; i < size/2; i++) {
                data[i] += 1.0;
            }
        }
        
        #pragma omp task depend(inout: data[size/2:size/2])
        {
            for (int i = size/2; i < size; i++) {
                data[i] -= 1.0;
            }
        }
        
        #pragma omp taskwait
    }
}

/* Main function orchestrates all tests */
int main() {
    // Initialize test data
    double *array1 = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    double *output = (double*)malloc(M * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!array1 || !matrix || !output || !block) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        block->values[i] = (double)i * 0.1;
        block->indices[i] = i;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    // Call all test functions to trigger various OpenMP constructs
    test_scan_clauses(array1, N);
    
    test_enter_data(block);
    
    test_internal_temporaries(matrix, N, M);
    
    test_target_region(block, output);
    
    test_conditional_pragmas(array1, N, 500);
    
    // Final reduction for verification
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + block->values[i];
    }
    
    for (int i = 0; i < M; i++) {
        final_sum += output[i];
    }
    
    printf("Final result: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    // Cleanup
    free(array1);
    free(matrix);
    free(output);
    free(block);
    
    return 0;
}
