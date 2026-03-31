/* test_omp_clause_coverage.c
 * Designed to trigger GCC's tree pretty-printer for specific OpenMP clauses
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

/* Function 1: Tests scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        exclusive_sum = prefix_sum;
        
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] += exclusive_sum;
        
        prefix_sum += arr[i];
        
        #pragma omp scan inclusive(prefix_sum)
    }
}

/* Function 2: Tests enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block[:1]) \
        map(to: block->values[:N/2])  // This should trigger OMP_CLAUSE_ENTER with TO set
    
    #pragma omp target enter data map(to: block->indices[:N]) \
        map(alloc: block->result)
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 1.5;
        block->indices[i] = i;
    }
    
    #pragma omp target exit data map(from: block->result) \
        map(release: block->indices[:N])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double last_val = 0.0;
    double reduction_sum = 0.0;
    
    /* This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ clauses */
    #pragma omp parallel for collapse(2) reduction(+:reduction_sum) \
        lastprivate(last_val) linear(i:1)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double temp = matrix[i * cols + j];
            reduction_sum += temp;
            
            if (j == cols - 1) {
                last_val = temp;
            }
            
            /* Nested OpenMP region to increase complexity */
            #pragma omp simd reduction(+:reduction_sum)
            for (int k = 0; k < M; k++) {
                reduction_sum += 0.001 * k;
            }
        }
    }
    
    /* Additional SIMD with conditional to potentially generate _CONDTEMP_ */
    #pragma omp simd reduction(+:reduction_sum)
    for (int i = 0; i < rows * cols; i++) {
        if (matrix[i] > 100.0) {
            reduction_sum += matrix[i] * 2.0;
        } else {
            reduction_sum += matrix[i];
        }
    }
}

/* Function 4: Mixed directives with scan for _SCANTEMP_ generation */
void test_mixed_directives(double *data, int size) {
    double scan_var = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:scan_var) nowait
        for (int i = 0; i < size; i++) {
            scan_var += data[i];
        }
        
        #pragma omp barrier
        
        #pragma omp for reduction(+:scan_var)
        for (int i = 0; i < size; i++) {
            #pragma omp scan inclusive(scan_var)
            data[i] = scan_var;
        }
    }
    
    /* Target region with complex data environment */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[:size]) if(size > 500)
    for (int i = 0; i < size; i++) {
        data[i] = data[i] * 2.0 + omp_get_thread_num();
    }
}

/* Function 5: Tests taskloop with grainsize for additional clause complexity */
void test_taskloop(double *arr, int n) {
    #pragma omp taskloop grainsize(32) \
        reduction(+:arr[:n/2])  // Array reduction for complexity
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * (i + 1);
        
        /* Nested task to create more complex gimplification */
        #pragma omp task if(i % 100 == 0)
        {
            arr[i] += 1.0;
        }
    }
}

int main() {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize test data */
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !block) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        array1[i] = i * 0.5;
        block->values[i] = i * 1.0;
        block->indices[i] = i;
    }
    
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            array2[i * M + j] = i * j * 0.1;
        }
    }
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(array1, N);
    
    /* Conditional compilation path */
    if (N > 1000) {
        #pragma omp parallel if(N > 1000)
        {
            test_enter_data(block);
        }
    } else {
        test_enter_data(block);
    }
    
    test_internal_temporaries(array2, N, M);
    test_mixed_directives(array1, N);
    test_taskloop(array1, N);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + block->values[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    #pragma omp target exit data map(release: block[:1])
    
    free(array1);
    free(array2);
    free(block);
    
    return 0;
}
