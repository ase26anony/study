/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 2000
#define M 100

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double sum;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
        
        /* Use exclusive scan in conditional block */
        if (i % 2 == 0) {
            #pragma omp scan exclusive(exclusive_sum)
            exclusive_sum += arr[i];
        }
    }
    
    printf("Scan test: prefix_sum = %f\n", prefix_sum);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Create device data environment with to mapper */
    #pragma omp enter data map(to: block[0:M]) \
        to(block[0].values[0:N]) \
        to(block[0].indices[0:N])
    
    /* Use the data on device if target is available */
    #ifdef _OPENMP
    #pragma omp target if(omp_get_num_devices() > 0) \
        map(tofrom: block[0:M])
    #endif
    {
        for (int i = 0; i < M; i++) {
            block[i].sum = 0.0;
            for (int j = 0; j < N; j++) {
                block[i].sum += block[i].values[j];
            }
        }
    }
    
    #pragma omp exit data map(from: block[0:M])
    
    printf("Enter data test completed\n");
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* Complex reduction with lastprivate and linear - may generate _LOOPTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:total) \
        lastprivate(last_val) linear(i:1)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double temp = matrix[i * cols + j];
            total += temp * temp;
            last_val = i * cols + j;
            
            /* Nested parallel region inside loop - may generate _REDUCTEMP_ */
            #pragma omp parallel for reduction(+:total) if(i > 100)
            for (int k = 0; k < 10; k++) {
                total += 0.001 * k;
            }
        }
    }
    
    /* SIMD loop with conditionals - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:total) linear(i:1)
    for (int i = 0; i < rows * cols; i++) {
        if (matrix[i] > 0.5) {
            total += 1.0;
        } else {
            total -= 0.5;
        }
    }
    
    printf("Internal temporaries test: total = %f, last_val = %d\n", total, last_val);
}

/* Function 4: Scan with reduction in complex loop */
void test_scan_reduction(double *arr, int n) {
    double scan_result = 0.0;
    double reduction_result = 0.0;
    
    /* Loop that may generate _SCANTEMP_ */
    #pragma omp parallel for reduction(+:reduction_result) \
        private(scan_result)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(scan_result)
        scan_result += arr[i];
        
        reduction_result += scan_result;
        
        /* Conditional scan to generate different paths */
        if (i % 3 == 0) {
            #pragma omp scan exclusive(scan_result)
            scan_result *= 0.9;
        }
    }
    
    printf("Scan reduction test: result = %f\n", reduction_result);
}

/* Function 5: Target regions with complex data clauses */
void test_target_regions(struct DataBlock *blocks, int num_blocks) {
    double device_total = 0.0;
    
    #pragma omp target teams distribute parallel for \
        map(to: blocks[0:num_blocks]) \
        map(tofrom: device_total) \
        reduction(+:device_total) \
        if(num_blocks > 50)
    for (int b = 0; b < num_blocks; b++) {
        for (int i = 0; i < N; i++) {
            device_total += blocks[b].values[i] * blocks[b].indices[i];
        }
    }
    
    /* Enter data with multiple to clauses */
    #pragma omp enter data map(to: blocks) \
        to(blocks[0].sum) \
        to(blocks[0].values[0:N/2])
    
    printf("Target regions test: device_total = %f\n", device_total);
}

int main() {
    /* Initialize test data */
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(10 * sizeof(struct DataBlock));
    
    if (!array || !matrix || !blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with test data */
    for (int i = 0; i < N; i++) {
        array[i] = (i + 1) * 0.01;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (double)rand() / RAND_MAX;
    }
    
    for (int b = 0; b < 10; b++) {
        blocks[b].sum = 0.0;
        for (int i = 0; i < N; i++) {
            blocks[b].values[i] = (double)rand() / RAND_MAX;
            blocks[b].indices[i] = i;
        }
    }
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array, N);
    test_enter_data(blocks);
    test_internal_temporaries(matrix, N, M);
    test_scan_reduction(array, N);
    test_target_regions(blocks, 10);
    
    /* Final reduction across all results */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
        lastprivate(array) if(N > 1000)
    for (int i = 0; i < N; i++) {
        final_result += array[i];
        if (i == N - 1) {
            array[i] = final_result;
        }
    }
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(blocks);
    
    return 0;
}
