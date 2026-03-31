/* Test program to trigger uncovered pretty-printing of OpenMP clauses */
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
    
    #pragma omp parallel for reduction(+:partial_sum) \
        private(exclusive_prefix) schedule(static)
    for (int i = 0; i < n; i++) {
        /* Scan inclusive clause */
        #pragma omp scan inclusive(partial_sum)
        partial_sum += arr[i];
        arr[i] = partial_sum;
        
        /* Scan exclusive clause */
        #pragma omp scan exclusive(exclusive_prefix)
        exclusive_prefix += arr[i] * 0.5;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *data) {
    /* Enter data with to clause */
    #pragma omp enter data map(to: data[0:M]) \
        map(to: data[0].values) \
        map(to: data[0].indices)
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            data[i].values[j] = i * 1000.0 + j;
            data[i].indices[j] = i * N + j;
        }
    }
    
    #pragma omp exit data map(from: data[0:M])
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
            double temp = matrix[i * cols + j];
            total += temp * temp;
            
            /* Conditional inside loop may generate _CONDTEMP_ */
            if (temp > 0.5) {
                #pragma omp atomic
                last_val = i * cols + j;
            }
        }
    }
    
    /* SIMD loop with conditional */
    #pragma omp simd reduction(+:total) linear(i:1) \
        safelen(16)
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = matrix[i] * 2.0;
        if (matrix[i] < 0) {
            matrix[i] = 0.0;  /* May generate _CONDTEMP_ */
        }
    }
}

/* Function 4: Mixed clauses for scan temp generation */
void test_scan_temporaries(double *arr, int n) {
    double scan_temp = 0.0;
    
    #pragma omp parallel for reduction(+:scan_temp) \
        schedule(dynamic)
    for (int i = 0; i < n; i++) {
        /* Multiple scan directives */
        #pragma omp scan inclusive(scan_temp)
        scan_temp += arr[i];
        
        #pragma omp scan exclusive(scan_temp)
        arr[i] = scan_temp * 0.1;
    }
}

/* Function 5: Target regions with complex data environment */
void test_target_complex(struct DataBlock *data, int count) {
    double reduction_var = 0.0;
    
    /* Conditional parallel region */
    #pragma omp parallel if(count > 500) \
        reduction(+:reduction_var) num_threads(4)
    {
        #pragma omp for nowait
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < N; j++) {
                reduction_var += data[i].values[j];
            }
        }
        
        /* Nested target region */
        #pragma omp target teams distribute parallel for \
            map(tofrom: data[0:count]) \
            reduction(+:reduction_var) \
            if(count > 100)
        for (int i = 0; i < count; i++) {
            data[i].result = reduction_var / (count * N);
        }
    }
}

/* Main function orchestrates all tests */
int main() {
    /* Initialize test data */
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *data = (struct DataBlock*)malloc(M * sizeof(struct DataBlock));
    
    if (!array || !matrix || !data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            data[i].values[j] = 0.0;
            data[i].indices[j] = 0;
        }
        data[i].result = 0.0;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array, N);
    
    test_enter_data(data);
    
    test_internal_temporaries(matrix, N, M);
    
    test_scan_temporaries(array, N);
    
    test_target_complex(data, M);
    
    /* Final computation and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        schedule(guided)
    for (int i = 0; i < N; i++) {
        final_sum += array[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(data);
    
    return 0;
}
