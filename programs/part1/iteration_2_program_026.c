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
    double exclusive_scan = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
    }
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:partial_sum)
        for (int i = 0; i < n; i++) {
            partial_sum += arr[i];
            
            /* Trigger OMP_CLAUSE_INCLUSIVE */
            #pragma omp scan inclusive(partial_sum)
            arr[i] += partial_sum;
        }
        
        #pragma omp for reduction(+:exclusive_scan)
        for (int i = 0; i < n; i++) {
            /* Trigger OMP_CLAUSE_EXCLUSIVE */
            #pragma omp scan exclusive(exclusive_scan)
            arr[i] += exclusive_scan;
            exclusive_scan += arr[i];
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block->values[0:N]) \
                           map(to: block->indices) \
                           map(to: block->result)
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: block->values[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + i;
        block->indices[i] = i % M;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values)
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:total) \
                lastprivate(last_val) linear(i:1)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double val = matrix[i * cols + j];
            total += val * (i + j);
            last_val = i * cols + j;
            
            /* Nested parallel region inside loop - complex for gimplifier */
            if (val > 100.0) {
                #pragma omp parallel if(rows > 500) num_threads(2)
                {
                    #pragma omp for reduction(+:total)
                    for (int k = 0; k < M; k++) {
                        total += k * 0.1;
                    }
                }
            }
        }
    }
    
    /* SIMD loop with conditionals - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:total) linear(i:1)
    for (int i = 0; i < rows * cols; i++) {
        if (matrix[i] > 50.0) {
            total += matrix[i] * 2.0;
        } else {
            total += matrix[i] * 0.5;
        }
    }
}

/* Function 4: Mixed directives to trigger _SCANTEMP_ */
void test_scan_temp(double *arr, int n) {
    double scan_temp = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:scan_temp)
        for (int i = 0; i < n; i++) {
            scan_temp += arr[i];
            
            /* Multiple scan directives */
            #pragma omp scan inclusive(scan_temp)
            arr[i] = scan_temp;
            
            if (i % 10 == 0) {
                #pragma omp scan exclusive(scan_temp)
                arr[i] -= scan_temp;
            }
        }
    }
    
    /* Target region with complex data environment */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr[0:n]) reduction(+:scan_temp)
    for (int i = 0; i < n; i++) {
        scan_temp += arr[i] * 0.1;
    }
}

/* Function 5: Very complex nested OpenMP with all clause types */
void test_comprehensive(struct DataBlock *block, double *arr, int n) {
    double total_reduction = 0.0;
    int linear_var = 0;
    
    /* This should generate multiple internal temporary clauses */
    #pragma omp target teams distribute parallel for \
            map(tofrom: block->values[0:N], arr[0:n]) \
            reduction(+:total_reduction) \
            lastprivate(linear_var) \
            linear(linear_var:1) \
            if(n > 1000)
    for (int i = 0; i < n; i++) {
        block->values[i % N] += arr[i];
        total_reduction += arr[i] * i;
        linear_var = i;
        
        /* Nested scan */
        #pragma omp scan inclusive(total_reduction)
        arr[i] = total_reduction;
    }
    
    /* Enter data with structured block */
    #pragma omp enter data map(to: total_reduction)
    
    /* Parallel region with task reduction */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total_reduction)
            {
                #pragma omp task in_reduction(+:total_reduction)
                {
                    total_reduction += 100.0;
                }
            }
        }
    }
    
    #pragma omp exit data map(from: total_reduction)
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    struct DataBlock block;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        block.values[i] = i * 1.5;
        block.indices[i] = i;
        array1[i] = i * 0.3;
    }
    
    for (int i = 0; i < N * M; i++) {
        array2[i] = i * 0.1;
    }
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(array1, N);
    
    #pragma omp parallel if(N > 1000)
    {
        test_enter_data(&block);
    }
    
    test_internal_temporaries(array2, N, M);
    test_scan_temp(array1, N);
    test_comprehensive(&block, array1, N);
    
    /* Final computation and output */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                schedule(dynamic, 16)
    for (int i = 0; i < N; i++) {
        final_result += array1[i] + block.values[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Coverage test completed successfully.\n");
    
    free(array1);
    free(array2);
    
    return 0;
}
