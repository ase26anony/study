/* Test program to trigger uncovered OpenMP clause pretty-printing logic */
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
    double partial_sum = 0.0;
    double prefix_sum[N] = {0};
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        #pragma omp for reduction(+:partial_sum) nowait
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
            #pragma omp scan inclusive(local_sum)
            prefix_sum[i] = local_sum;
        }
        
        #pragma omp for reduction(+:partial_sum)
        for (int i = n-1; i >= 0; i--) {
            local_sum -= arr[i];
            #pragma omp scan exclusive(local_sum)
            prefix_sum[i] += local_sum;
        }
    }
    
    printf("Scan test result: %f\n", prefix_sum[n-1]);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp target enter data map(to: block[0:1]) \
        map(to: block->values[0:N]) \
        map(to: block->indices[0:N])
    
    /* Complex mapping with to clause */
    #pragma omp target enter data map(to: block->result)
    
    printf("Enter data test completed\n");
}

/* Function 3: Complex loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double sum = 0.0;
    double last_val = 0.0;
    
    /* This complex construct should generate _LOOPTEMP_ and _REDUCTEMP_ */
    #pragma omp parallel for simd reduction(+:sum) lastprivate(last_val) \
        linear(i:1) collapse(2) if(n > 500)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            a[i * M + j] = b[i * M + j] * 2.0;
            sum += a[i * M + j];
            if (j == M-1) {
                last_val = a[i * M + j];
            }
        }
    }
    
    /* Nested reduction with conditionals - may generate _CONDTEMP_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) map(tofrom: a[0:n*M]) if(n > 100)
    for (int i = 0; i < n * M; i++) {
        if (i % 2 == 0) {
            a[i] += sum;
        } else {
            a[i] -= sum;
        }
    }
    
    printf("Internal temporaries test sum: %f, last: %f\n", sum, last_val);
}

/* Function 4: Scan directive with scan clause temporaries */
void test_scan_temporaries(double *arr, int n) {
    double scan_temp[N] = {0};
    
    #pragma omp parallel
    {
        #pragma omp for simd
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5;
        }
        
        /* This should generate _SCANTEMP_ clauses */
        double local_scan = 0.0;
        #pragma omp for simd reduction(inscan, +:local_scan)
        for (int i = 0; i < n; i++) {
            local_scan += arr[i];
            #pragma omp scan inclusive(local_scan)
            scan_temp[i] = local_scan;
        }
    }
    
    printf("Scan temporary test: %f\n", scan_temp[n-1]);
}

/* Function 5: Mixed clauses in target region */
void test_mixed_clauses(struct DataBlock *block, double *output, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[0:n]) \
        map(to: block->indices[0:n]) \
        reduction(+:block->result) \
        if(n > 200)
    for (int i = 0; i < n; i++) {
        block->values[i] = block->values[i] * block->indices[i];
        block->result += block->values[i];
        output[i] = block->values[i];
    }
    
    printf("Mixed clauses result: %f\n", block->result);
}

int main() {
    double *array1 = (double*)malloc(N * M * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    struct DataBlock data_block;
    
    /* Initialize data */
    for (int i = 0; i < N * M; i++) {
        array1[i] = (double)i / 100.0;
        array2[i] = (double)(i + 1) / 50.0;
    }
    
    for (int i = 0; i < N; i++) {
        data_block.values[i] = (double)i;
        data_block.indices[i] = i % 10;
        output[i] = 0.0;
    }
    data_block.result = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call all test functions to trigger different OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(&data_block);
    test_internal_temporaries(array1, array2, N);
    test_scan_temporaries(array1, N);
    test_mixed_clauses(&data_block, output, N);
    
    /* Final reduction with complex clauses */
    double final_sum = 0.0;
    #pragma omp parallel for simd reduction(+:final_sum) \
        lastprivate(data_block) if(N > 100) linear(i:2)
    for (int i = 0; i < N; i++) {
        final_sum += output[i] + data_block.values[i];
        if (i == N-1) {
            data_block.result = final_sum;
        }
    }
    
    printf("Final result: %f\n", final_sum);
    printf("Data block result: %f\n", data_block.result);
    
    free(array1);
    free(array2);
    free(output);
    
    return 0;
}
