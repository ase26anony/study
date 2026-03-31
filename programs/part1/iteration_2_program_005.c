/* Test program to trigger uncovered pretty-printing of OpenMP clauses */
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
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum) private(exclusive_prefix)
    for (int i = 0; i < n; i++) {
        // Test inclusive scan
        #pragma omp scan inclusive(partial_sum)
        partial_sum += arr[i];
        
        // Test exclusive scan
        exclusive_prefix = partial_sum - arr[i];
        #pragma omp scan exclusive(exclusive_prefix)
        arr[i] = exclusive_prefix;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block->values[:N]) \
        map(to: block->indices) \
        map(to: block->result)
    
    // Use the data in target region
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[:N]) \
        map(to: block->indices[:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + block->indices[i];
    }
    
    #pragma omp exit data map(from: block->result) \
        map(release: block->values[:N], block->indices[:N])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double sum = 0.0;
    int last_val = 0;
    
    // This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ clauses
    #pragma omp parallel for collapse(2) reduction(+:sum) lastprivate(last_val) \
        linear(i:1) schedule(dynamic, 16)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum += matrix[i * cols + j];
            last_val = matrix[i * cols + j];
            
            // Nested parallel region to increase complexity
            #pragma omp parallel if(rows > 500) num_threads(2)
            {
                #pragma omp for nowait
                for (int k = 0; k < 10; k++) {
                    matrix[i * cols + j] += 0.001 * k;
                }
            }
        }
    }
    
    // SIMD loop with conditionals - may generate _CONDTEMP_
    #pragma omp simd reduction(+:sum) linear(i:1) simdlen(8)
    for (int i = 0; i < rows * cols; i++) {
        if (matrix[i] > 0.5) {
            sum += matrix[i] * 2.0;
        } else {
            sum += matrix[i] * 0.5;
        }
    }
}

/* Function 4: Target region with complex data clauses */
void test_target_complex(struct DataBlock *block, double *output) {
    double local_sum = 0.0;
    
    #pragma omp target teams distribute parallel for \
        map(to: block->values[:N]) \
        map(from: output[:M]) \
        reduction(+:local_sum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < N; i++) {
        double val = block->values[i];
        local_sum += val;
        
        if (i < M) {
            output[i] = val * local_sum;
        }
        
        // Scan directive in target region
        #pragma omp scan inclusive(local_sum)
        block->values[i] = local_sum;
    }
}

/* Function 5: Mixed directives to trigger _SCANTEMP_ */
void test_mixed_directives(int *data, int n) {
    int scan_temp = 0;
    
    #pragma omp parallel for ordered(1)
    for (int i = 0; i < n; i++) {
        #pragma omp ordered depend(sink: i-1)
        data[i] += scan_temp;
        #pragma omp ordered depend(source)
        
        // This should generate scan temporaries
        #pragma omp atomic update
        scan_temp += data[i];
    }
}

int main() {
    // Initialize test data
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * N * sizeof(double));
    int *int_data = (int*)malloc(N * sizeof(int));
    double *output = (double*)malloc(M * sizeof(double));
    
    struct DataBlock block;
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
        int_data[i] = i % 100;
        block.values[i] = (double)(i * 2) / N;
        block.indices[i] = i;
    }
    
    for (int i = 0; i < N * N; i++) {
        matrix[i] = (double)(i % 1000) / 1000.0;
    }
    
    block.result = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    // Call all test functions to trigger different OpenMP constructs
    test_scan_clauses(array, N);
    
    // Conditional compilation path
    #pragma omp parallel if(N > 1000) num_threads(4)
    {
        test_enter_data(&block);
    }
    
    test_internal_temporaries(matrix, N, N);
    test_target_complex(&block, output);
    test_mixed_directives(int_data, N);
    
    // Final reduction for verification
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        schedule(guided) num_threads(omp_get_max_threads())
    for (int i = 0; i < N; i++) {
        final_sum += array[i] + block.values[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    // Cleanup
    free(array);
    free(matrix);
    free(int_data);
    free(output);
    
    return 0;
}
