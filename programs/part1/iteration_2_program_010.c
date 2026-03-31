/* Test program to trigger uncovered lines in tree-pretty-print.cc
   Specifically targets OMP_CLAUSE_INCLUSIVE, EXCLUSIVE, ENTER, and internal temps */
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

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
        partial_sum += arr[i];
    }
    
    /* This should trigger OMP_CLAUSE_INCLUSIVE */
    double inclusive_scan = 0.0;
    #pragma omp parallel for reduction(+:inclusive_scan)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(inclusive_scan)
        inclusive_scan += arr[i];
        arr[i] = inclusive_scan;
    }
    
    /* This should trigger OMP_CLAUSE_EXCLUSIVE */
    double exclusive_scan = 0.0;
    #pragma omp parallel for reduction(+:exclusive_scan)
    for (int i = 0; i < n; i++) {
        double temp = arr[i];
        #pragma omp scan exclusive(exclusive_scan)
        arr[i] = exclusive_scan;
        exclusive_scan += temp;
    }
}

/* Function 2: Uses enter data with to clause */
void test_enter_data(struct DataBlock *block) {
    /* This should trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block->values[0:N]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: block->values[0:N]) \
            map(to: block->indices[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + block->indices[i];
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values[0:N], block->indices)
}

/* Function 3: Complex nested loops to generate internal temps */
void test_internal_temps(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* Complex reduction with lastprivate and linear - may generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) \
                linear(i:1) if(rows > 100)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = (i + j) * 0.1;
            total += matrix[i * cols + j];
            if (i == rows - 1 && j == cols - 1) {
                last_val = i * cols + j;
            }
        }
    }
    
    /* SIMD with conditional - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:total) simdlen(8)
    for (int i = 0; i < rows * cols; i++) {
        if (matrix[i] > 50.0) {  /* Conditional inside SIMD */
            matrix[i] = 50.0;
        }
        total += matrix[i];
    }
    
    /* Scan with reduction - may generate _SCANTEMP_ */
    double scan_temp = 0.0;
    #pragma omp parallel for reduction(+:scan_temp)
    for (int i = 0; i < rows * cols; i++) {
        #pragma omp scan inclusive(scan_temp)
        scan_temp += matrix[i];
        matrix[i] = scan_temp;
    }
}

/* Function 4: Mixed constructs with target regions */
void test_target_regions(struct DataBlock *blocks, int num_blocks) {
    /* Complex mapping that may generate various internal temps */
    #pragma omp target teams distribute parallel for \
            map(tofrom: blocks[0:num_blocks]) \
            reduction(+:blocks[0].result) \
            lastprivate(num_blocks)
    for (int i = 0; i < num_blocks; i++) {
        double block_sum = 0.0;
        for (int j = 0; j < N; j++) {
            block_sum += blocks[i].values[j];
        }
        blocks[i].result = block_sum;
        
        /* Nested scan inside target region */
        double inner_scan = 0.0;
        #pragma omp scan exclusive(inner_scan)
        for (int j = 0; j < N; j++) {
            inner_scan += blocks[i].values[j];
            blocks[i].values[j] = inner_scan;
        }
    }
}

/* Main function orchestrates all tests */
int main() {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Allocate test data */
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(2 * sizeof(struct DataBlock));
    
    if (!array || !matrix || !blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array[i] = 0.0;
        blocks[0].values[i] = i * 1.5;
        blocks[0].indices[i] = i;
        blocks[1].values[i] = i * 2.5;
        blocks[1].indices[i] = N - i - 1;
    }
    blocks[0].result = 0.0;
    blocks[1].result = 0.0;
    
    /* Execute tests to trigger various OpenMP clauses */
    test_scan_clauses(array, N);
    
    test_enter_data(&blocks[0]);
    
    test_internal_temps(matrix, N, M);
    
    /* Conditional compilation path */
    int use_target = 1;
    if (use_target) {
        test_target_regions(blocks, 2);
    }
    
    /* Final reduction and output */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                if(N > 500)  /* Conditional to create different gimplification */
    for (int i = 0; i < N; i++) {
        final_result += array[i] + blocks[0].values[i];
    }
    
    final_result += blocks[0].result + blocks[1].result;
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(blocks);
    
    return 0;
}
