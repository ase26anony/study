/* test_omp_clause_coverage.c
 * Designed to trigger uncovered pretty-printing logic in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -o test test_omp_clause_coverage.c
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
void scan_test(int n, double *arr) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        // Simulate some computation
        arr[i] = (double)i * 1.5;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        
        #pragma omp scan exclusive(exclusive_sum)
        exclusive_sum += arr[i] * 0.5;
        
        // Use both sums
        arr[i] += prefix_sum - exclusive_sum;
    }
    
    printf("Scan test completed, prefix_sum = %f\n", prefix_sum);
}

/* Function 2: Uses enter data with to mapper */
void enter_data_test(struct DataBlock *block) {
    // Initialize data on host
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 0.01;
        block->indices[i] = i;
    }
    
    // Enter data with to mapper - triggers OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO
    #pragma omp enter data map(to: block[:1]) map(to: block->values[:N], block->indices[:N])
    
    // Also test with explicit to clause
    #pragma omp target enter data map(to: block->result)
    
    printf("Enter data test completed\n");
}

/* Function 3: Complex nested loops to generate internal temporaries */
void complex_loop_test(int n, int m, double *matrix) {
    double total = 0.0;
    int last_val = 0;
    
    // This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ clauses
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) linear(i:1)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            double val = (i * m + j) * 0.1;
            matrix[i * m + j] = val;
            total += val;
            
            // Nested reduction-like operation
            #pragma omp atomic
            total += val * 0.01;
            
            last_val = i * m + j;
        }
    }
    
    // Additional parallel region with if clause
    #pragma omp parallel if(n > 1000) reduction(+:total)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            total += i * 0.001;
        }
    }
    
    printf("Complex loop test completed, total = %f, last_val = %d\n", total, last_val);
}

/* Function 4: SIMD with conditionals to generate _CONDTEMP_ */
void simd_cond_test(int n, double *a, double *b, double *c) {
    #pragma omp simd reduction(+:a[:n]) linear(b:1)
    for (int i = 0; i < n; i++) {
        // Complex conditional that might generate _CONDTEMP_
        if (i % 3 == 0) {
            a[i] = b[i] * 2.0;
        } else if (i % 3 == 1) {
            a[i] = b[i] + c[i];
        } else {
            a[i] = b[i] - c[i];
        }
        
        // Additional computation
        b[i] = a[i] * 0.5;
        c[i] = a[i] * 2.0;
    }
    
    printf("SIMD conditional test completed\n");
}

/* Function 5: Target region with complex mapping */
void target_test(struct DataBlock *block) {
    double local_sum = 0.0;
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: block->values[:N]) \
            reduction(+:local_sum) \
            map(to: block->indices[:N])
    for (int i = 0; i < N; i++) {
        block->values[i] += block->indices[i] * 0.1;
        local_sum += block->values[i];
    }
    
    block->result = local_sum;
    
    // Exit data
    #pragma omp target exit data map(from: block->result)
    
    printf("Target test completed, result = %f\n", block->result);
}

/* Function 6: Scan directive in reduction context (for _SCANTEMP_) */
void scan_reduction_test(int n, double *arr) {
    double scan_result = 0.0;
    
    #pragma omp parallel
    {
        double local_scan = 0.0;
        
        #pragma omp for reduction(+:scan_result)
        for (int i = 0; i < n; i++) {
            double val = arr[i] * (i + 1);
            
            #pragma omp scan inclusive(local_scan)
            local_scan += val;
            
            arr[i] = local_scan;
            scan_result += val;
        }
    }
    
    printf("Scan reduction test completed, scan_result = %f\n", scan_result);
}

int main() {
    printf("Starting OpenMP clause coverage test...\n");
    
    // Allocate test data
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !array3 || !matrix || !block) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        array1[i] = i * 0.5;
        array2[i] = i * 0.3;
        array3[i] = i * 0.7;
    }
    
    // Call test functions
    scan_test(N, array1);
    enter_data_test(block);
    complex_loop_test(N, M, matrix);
    simd_cond_test(N, array1, array2, array3);
    target_test(block);
    scan_reduction_test(N, array1);
    
    // Final computation with mixed directives
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) if(N>100)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + array3[i];
    }
    
    printf("Final sum = %f\n", final_sum);
    printf("Test completed successfully\n");
    
    // Cleanup
    free(array1);
    free(array2);
    free(array3);
    free(matrix);
    free(block);
    
    return 0;
}
