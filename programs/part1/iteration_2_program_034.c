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
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum) private(exclusive_prefix)
    for (int i = 0; i < n; i++) {
        // Test inclusive scan
        #pragma omp scan inclusive(partial_sum)
        partial_sum += arr[i];
        
        // Test exclusive scan
        exclusive_prefix = partial_sum - arr[i];
        #pragma omp scan exclusive(exclusive_prefix)
        
        arr[i] = partial_sum + exclusive_prefix;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block[:1]) \
        map(to: block->values[:N]) \
        map(to: block->indices[:N])
    
    // Use the data in target region
    #pragma omp target map(tofrom: block->result) \
        map(to: block->values[:N]) \
        map(to: block->indices[:N])
    {
        block->result = 0.0;
        for (int i = 0; i < N; i++) {
            block->result += block->values[i] * block->indices[i];
        }
    }
    
    #pragma omp exit data map(from: block[:1])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    // Complex loop with multiple clauses to generate _LOOPTEMP_, _REDUCTEMP_
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) \
        linear(i:1) schedule(dynamic)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            total += matrix[i * cols + j];
            last_val = matrix[i * cols + j];
            
            // Nested parallel region to increase complexity
            #pragma omp parallel if(rows > 100) num_threads(2)
            {
                #pragma omp for reduction(+:total) nowait
                for (int k = 0; k < 10; k++) {
                    total += 0.1;
                }
            }
        }
    }
    
    // SIMD loop with conditionals to potentially generate _CONDTEMP_
    #pragma omp simd reduction(+:total) linear(i:1) \
        simdlen(8) if(rows > 500)
    for (int i = 0; i < rows; i++) {
        if (matrix[i] > 0.5) {
            total += matrix[i] * 2.0;
        } else {
            total += matrix[i] * 0.5;
        }
    }
}

/* Function 4: Mixed OpenMP constructs with scan directive */
void test_mixed_constructs(double *data, int n) {
    double scan_buffer[N];
    
    #pragma omp target teams distribute parallel for \
        map(to: data[:n]) map(from: scan_buffer[:n]) \
        reduction(+:scan_buffer[:n])
    for (int i = 0; i < n; i++) {
        scan_buffer[i] = data[i];
        
        // Scan directive in target region
        #pragma omp scan inclusive(scan_buffer[i])
        if (i > 0) {
            scan_buffer[i] += scan_buffer[i-1];
        }
    }
    
    // Taskloop with reduction and scan
    #pragma omp taskloop reduction(+:scan_buffer[:n]) \
        grainsize(64) if(n > 1000)
    for (int i = 0; i < n; i++) {
        #pragma omp scan exclusive(scan_buffer[i])
        data[i] = scan_buffer[i];
    }
}

/* Function 5: Complex reduction patterns for _REDUCTEMP_ generation */
void test_complex_reductions(struct DataBlock *blocks, int num_blocks) {
    double global_sum = 0.0;
    int global_max = 0;
    
    #pragma omp parallel for reduction(+:global_sum) \
        reduction(max:global_max) \
        lastprivate(blocks) \
        schedule(guided)
    for (int b = 0; b < num_blocks; b++) {
        double block_sum = 0.0;
        
        // Nested reduction
        #pragma omp parallel for reduction(+:block_sum) \
            if(N > 500) num_threads(4)
        for (int i = 0; i < N; i++) {
            block_sum += blocks[b].values[i];
            if (blocks[b].indices[i] > global_max) {
                #pragma omp atomic write
                global_max = blocks[b].indices[i];
            }
        }
        
        global_sum += block_sum;
        blocks[b].result = block_sum;
    }
}

int main() {
    // Initialize test data
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(3 * sizeof(struct DataBlock));
    
    if (!array || !matrix || !blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with test data
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
        for (int j = 0; j < M; j++) {
            matrix[i * M + j] = (double)(i + j) / (N + M);
        }
    }
    
    for (int b = 0; b < 3; b++) {
        for (int i = 0; i < N; i++) {
            blocks[b].values[i] = (double)(b * N + i) / (3 * N);
            blocks[b].indices[i] = i;
        }
        blocks[b].result = 0.0;
    }
    
    printf("Starting OpenMP tests...\n");
    
    // Test 1: Scan clauses
    test_scan_clauses(array, N);
    
    // Test 2: Enter data with to mapper
    test_enter_data(&blocks[0]);
    
    // Test 3: Internal temporaries
    test_internal_temporaries(matrix, N, M);
    
    // Test 4: Mixed constructs
    test_mixed_constructs(array, N);
    
    // Test 5: Complex reductions
    test_complex_reductions(blocks, 3);
    
    // Compute final result
    double final_result = 0.0;
    for (int i = 0; i < N; i++) {
        final_result += array[i];
    }
    
    for (int b = 0; b < 3; b++) {
        final_result += blocks[b].result;
    }
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    // Cleanup
    free(array);
    free(matrix);
    free(blocks);
    
    return 0;
}
