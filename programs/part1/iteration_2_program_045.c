/* test_openmp_coverage.c
 * Designed to trigger uncovered pretty-printing logic for OpenMP clauses
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -o test test_openmp_coverage.c
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

/* Function 1: Tests scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        // Test scan inclusive
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        
        // Test scan exclusive
        double temp = arr[i] * 2.0;
        #pragma omp scan exclusive(exclusive_sum)
        exclusive_sum += temp;
        
        arr[i] = prefix_sum + exclusive_sum;
    }
}

/* Function 2: Tests enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp target enter data map(to: block[0:1]) \
        map(to: block->values[0:N], block->indices[0:N])
    
    // Also test with explicit to clause
    #pragma omp target enter data map(to: block->result)
    
    // Complex enter with conditional
    if (block->result > 0) {
        #pragma omp target enter data map(to: block->values[N/2:N/2])
    }
}

/* Function 3: Complex loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double red_sum = 0.0;
    double last_val = 0.0;
    
    // This complex loop should generate _LOOPTEMP_, _REDUCTEMP_ clauses
    #pragma omp parallel for simd reduction(+:red_sum) lastprivate(last_val) \
        linear(i:1) collapse(2) if(n > 500)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            double temp = a[i] * b[j];
            red_sum += temp;
            
            // Conditional inside simd may generate _CONDTEMP_
            if (temp > 0.5) {
                last_val = temp;
            } else {
                last_val = -temp;
            }
        }
    }
    
    // Nested reduction with scan
    #pragma omp parallel
    {
        double local_sum = 0.0;
        #pragma omp for reduction(+:red_sum) nowait
        for (int i = 0; i < n; i++) {
            local_sum += a[i];
            #pragma omp scan inclusive(local_sum)
        }
        
        // Another reduction to force temporary generation
        #pragma omp for reduction(+:red_sum)
        for (int i = 0; i < n; i++) {
            red_sum += b[i];
        }
    }
}

/* Function 4: Target region with complex data environment */
void test_target_regions(struct DataBlock *block) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[0:N]) \
        reduction(+:block->result) \
        if(N > 1000)
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + i;
        block->result += block->values[i];
    }
    
    // Scan in target region
    double scan_temp = 0.0;
    #pragma omp target teams distribute parallel for \
        map(tofrom: scan_temp)
    for (int i = 0; i < N; i++) {
        #pragma omp scan inclusive(scan_temp)
        scan_temp += block->values[i];
    }
}

/* Function 5: Tests _SCANTEMP_ generation */
void test_scantemp_generation(double *arr, int n) {
    double sum1 = 0.0, sum2 = 0.0;
    
    #pragma omp parallel for reduction(+:sum1, sum2)
    for (int i = 0; i < n; i++) {
        // Multiple scans to potentially generate _SCANTEMP_
        #pragma omp scan inclusive(sum1)
        sum1 += arr[i];
        
        #pragma omp scan exclusive(sum2)
        sum2 += arr[i] * 0.5;
        
        arr[i] = sum1 + sum2;
    }
}

int main() {
    // Initialize data
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock block;
    
    srand(42);
    for (int i = 0; i < N; i++) {
        array1[i] = (double)rand() / RAND_MAX;
        array2[i] = (double)rand() / RAND_MAX;
        block.values[i] = array1[i];
        block.indices[i] = i;
    }
    block.result = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    // Call all test functions to trigger various OpenMP constructs
    test_scan_clauses(array1, N);
    test_enter_data(&block);
    test_internal_temporaries(array1, array2, N);
    test_target_regions(&block);
    test_scantemp_generation(array2, N);
    
    // Final computation with mixed clauses
    double final_result = 0.0;
    #pragma omp parallel for simd reduction(+:final_result) \
        lastprivate(block) if(N > 100) linear(i:1)
    for (int i = 0; i < N; i++) {
        final_result += array1[i] + array2[i];
        if (i == N-1) {
            block.result = final_result;
        }
    }
    
    printf("Final result: %f\n", final_result);
    printf("Block result: %f\n", block.result);
    
    // Cleanup
    free(array1);
    free(array2);
    
    return 0;
}
