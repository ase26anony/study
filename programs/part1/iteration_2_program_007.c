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

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_scan = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
        partial_sum += arr[i];
    }
    
    /* Trigger OMP_CLAUSE_INCLUSIVE */
    #pragma omp parallel for
    for (int i = 1; i < n; i++) {
        #pragma omp scan inclusive(partial_sum)
        partial_sum += arr[i];
    }
    
    /* Trigger OMP_CLAUSE_EXCLUSIVE */
    exclusive_scan = 0.0;
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        double temp = exclusive_scan;
        #pragma omp scan exclusive(exclusive_scan)
        arr[i] += exclusive_scan;
        exclusive_scan += arr[i] * 0.1;
    }
}

/* Function 2: Uses enter data with to clause */
void test_enter_data(struct DataBlock *block) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block->values[0:N]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + i;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values)
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double sum = 0.0;
    int last_val = 0;
    
    /* Complex construct likely to generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:sum) lastprivate(last_val) \
                linear(i:1) schedule(dynamic, 16)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = (i + j) * 0.25;
            sum += matrix[i * cols + j];
            if (i == rows - 1 && j == cols - 1) {
                last_val = i * cols + j;
            }
        }
    }
    
    /* Additional complexity with if clause */
    #pragma omp parallel for reduction(+:sum) if(rows > 500)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] += sum * 0.01;
        }
    }
}

/* Function 4: SIMD with conditionals to generate _CONDTEMP_ */
void test_simd_conditionals(double *a, double *b, double *c, int n) {
    #pragma omp simd reduction(+:a[0:n]) linear(i:1) \
                simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        /* Complex conditional that might generate _CONDTEMP_ */
        if (i % 3 == 0) {
            a[i] = b[i] * 2.0;
        } else if (i % 3 == 1) {
            a[i] = b[i] + c[i];
        } else {
            a[i] = b[i] - c[i];
        }
        
        /* Nested conditional for more complexity */
        a[i] = (a[i] > 100.0) ? 100.0 : 
               (a[i] < -100.0) ? -100.0 : a[i];
    }
}

/* Function 5: Scan with temporaries (_SCANTEMP_) */
void test_scan_temporaries(double *arr, int n) {
    double scan_temp = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for simd reduction(inscan,+:scan_temp)
        for (int i = 0; i < n; i++) {
            double val = arr[i] * 0.5;
            
            #pragma omp scan exclusive(scan_temp)
            arr[i] = val + scan_temp;
            scan_temp += val;
        }
    }
}

/* Main function that calls all test functions */
int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock block;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = i * 1.0;
        array2[i] = i * 2.0;
        array3[i] = i * 0.5;
    }
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(array1, N);
    
    /* Conditional compilation path */
    #pragma omp parallel if(N > 100)
    {
        test_enter_data(&block);
    }
    
    test_internal_temporaries(matrix, N, M);
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: array2[0:N])
    for (int i = 0; i < N; i++) {
        array2[i] = array2[i] * 3.14;
    }
    
    test_simd_conditionals(array1, array2, array3, N);
    test_scan_temporaries(array3, N);
    
    /* Final reduction for verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                schedule(guided) num_threads(4)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + array3[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(matrix);
    
    return 0;
}
