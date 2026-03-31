/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>

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
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
        
        if (i > 0) {
            #pragma omp scan exclusive(exclusive_sum)
            exclusive_sum += arr[i-1];
        }
    }
    
    printf("Scan test: prefix_sum = %f, exclusive_sum = %f\n", prefix_sum, exclusive_sum);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Create data on host */
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 0.5;
        block->indices[i] = i;
    }
    
    /* Enter data to target device with to mapper */
    #pragma omp target enter data map(to: block[:1]) \
        map(to: block->values[:N], block->indices[:N])
    
    /* Use the data on target */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] *= 2.0;
    }
    
    #pragma omp target exit data map(from: block->values[:N]) \
        map(release: block[:1], block->indices[:N])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ clauses */
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) \
        linear(i:1) if(rows > 100)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            total += matrix[i * cols + j];
            last_val = matrix[i * cols + j];
        }
    }
    
    /* Another complex reduction with multiple clauses */
    double sum1 = 0.0, sum2 = 0.0;
    #pragma omp parallel for reduction(+:sum1, sum2) \
        lastprivate(last_val) schedule(dynamic, 16)
    for (int i = 0; i < rows * cols; i++) {
        sum1 += matrix[i];
        sum2 += matrix[i] * matrix[i];
        if (i == rows * cols - 1) {
            last_val = matrix[i];
        }
    }
    
    printf("Temporaries test: total = %f, sum1 = %f, sum2 = %f, last = %d\n", 
           total, sum1, sum2, last_val);
}

/* Function 4: SIMD with conditionals to generate _CONDTEMP_ */
void test_simd_conditionals(double *a, double *b, double *c, int n) {
    #pragma omp simd reduction(+:c[0:n]) linear(i:1) \
        simdlen(8) if(n > 500)
    for (int i = 0; i < n; i++) {
        double temp = a[i] + b[i];
        if (temp > 100.0) {
            c[i] += temp * 2.0;
        } else {
            c[i] += temp * 0.5;
        }
    }
    
    /* Nested scan to trigger _SCANTEMP_ */
    double scan_temp = 0.0;
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(scan_temp)
        scan_temp += c[i];
        c[i] = scan_temp;
    }
}

/* Function 5: Target regions with complex data clauses */
void test_target_complex(struct DataBlock *blocks, int num_blocks) {
    /* Combined target and teams distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: blocks[0:num_blocks]) \
        reduction(+:blocks[0].result) \
        if(num_blocks > 1)
    for (int i = 0; i < num_blocks; i++) {
        double block_sum = 0.0;
        for (int j = 0; j < N; j++) {
            block_sum += blocks[i].values[j];
        }
        blocks[i].result = block_sum;
    }
}

int main() {
    /* Initialize test data */
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    struct DataBlock block;
    struct DataBlock blocks[5];
    
    for (int i = 0; i < N; i++) {
        array[i] = (i + 1) * 0.1;
        a[i] = i * 0.2;
        b[i] = i * 0.3;
        c[i] = 0.0;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (i % 100) * 0.01;
    }
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array, N);
    test_enter_data(&block);
    test_internal_temporaries(matrix, N, M);
    test_simd_conditionals(a, b, c, N);
    test_target_complex(blocks, 5);
    
    /* Final reduction for verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        if(N > 100) schedule(guided)
    for (int i = 0; i < N; i++) {
        final_sum += array[i] + c[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(a);
    free(b);
    free(c);
    
    return 0;
}
