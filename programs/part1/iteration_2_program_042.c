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
    double prefix_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i;
    }
    
    prefix_sum = 0.0;
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
    }
    
    prefix_sum = 0.0;
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = n-1; i >= 0; i--) {
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
    }
}

/* Function 2: Enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Create data on host */
    for (int i = 0; i < N; i++) {
        block->values[i] = (double)i * 2.0;
        block->indices[i] = i;
    }
    
    /* Enter data to target with to mapper */
    #pragma omp enter data map(to: block->values[0:N], block->indices[0:N]) \
                           map(to: block->result)
    
    /* Use the data on target (if supported) */
    #pragma omp target if(omp_get_num_devices() > 0) \
                       map(tofrom: block->values[0:N])
    {
        #pragma omp parallel for
        for (int i = 0; i < N; i++) {
            block->values[i] *= 1.5;
        }
    }
    
    #pragma omp exit data map(from: block->values[0:N], block->indices[0:N]) \
                          map(from: block->result)
}

/* Function 3: Complex loops with reduction, lastprivate, linear */
/* This should generate _LOOPTEMP_, _REDUCTEMP_ clauses */
void test_complex_reductions(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_i = 0, last_j = 0;
    
    /* Complex nested loop with multiple clauses */
    #pragma omp parallel for collapse(2) reduction(+:total) \
                lastprivate(last_i, last_j) linear(i:1) private(j)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = (double)(i * j);
            total += matrix[i * cols + j];
            last_i = i;
            last_j = j;
        }
    }
    
    /* Another complex reduction pattern */
    double partial_sums[M];
    #pragma omp parallel
    {
        #pragma omp for reduction(+:total) nowait
        for (int i = 0; i < rows; i++) {
            double row_sum = 0.0;
            for (int j = 0; j < cols; j++) {
                row_sum += matrix[i * cols + j];
            }
            total += row_sum;
        }
        
        #pragma omp for
        for (int k = 0; k < M; k++) {
            partial_sums[k] = 0.0;
            for (int i = 0; i < rows; i++) {
                partial_sums[k] += matrix[i * cols + (k % cols)];
            }
        }
    }
}

/* Function 4: SIMD with conditionals (potential _CONDTEMP_) */
void test_simd_with_conditionals(double *a, double *b, double *c, int n) {
    #pragma omp simd linear(i:1) reduction(+:a[0:n])
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            a[i] = b[i] * 2.0;
        } else {
            a[i] = b[i] / 2.0;
        }
        c[i] = a[i] + (double)i;
    }
    
    /* SIMD with scan for _SCANTEMP_ */
    double running_sum = 0.0;
    #pragma omp simd reduction(inscan,+:running_sum)
    for (int i = 0; i < n; i++) {
        b[i] = running_sum;
        #pragma omp scan exclusive(running_sum)
        running_sum += a[i];
    }
}

/* Function 5: Target regions with complex mappings */
void test_target_regions(struct DataBlock *block, double *output) {
    /* Complex target region with multiple map types */
    #pragma omp target teams distribute parallel for \
                map(to: block->values[0:N]) \
                map(from: output[0:N]) \
                map(alloc: block->indices[0:N]) \
                reduction(+:block->result)
    for (int i = 0; i < N; i++) {
        output[i] = block->values[i] * (double)block->indices[i];
        block->result += output[i];
    }
    
    /* Nested target region */
    #pragma omp target if(N > 500) map(tofrom: block->result)
    {
        #pragma omp parallel for reduction(+:block->result)
        for (int i = 0; i < N/2; i++) {
            block->result += (double)i;
        }
    }
}

int main() {
    printf("Starting OpenMP coverage test...\n");
    
    /* Allocate test data */
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !array3 || !matrix || !output || !block) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = 0.0;
        array2[i] = (double)i;
        array3[i] = (double)(i * 2);
    }
    
    block->result = 0.0;
    
    /* Call functions to trigger various OpenMP constructs */
    test_scan_clauses(array1, N);
    
    test_enter_data(block);
    
    test_complex_reductions(matrix, N, M);
    
    test_simd_with_conditionals(array2, array3, array1, N);
    
    test_target_regions(block, output);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) if(N>100)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + array3[i] + output[i];
    }
    
    final_sum += block->result;
    
    printf("Final result: %f\n", final_sum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(matrix);
    free(output);
    free(block);
    
    return 0;
}
