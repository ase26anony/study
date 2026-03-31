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
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
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

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N/2]) \
                map(to: block->indices)
    for (int i = 0; i < N/2; i++) {
        block->values[i] += block->indices[i] * 0.5;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values[0:N/2])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, double *c, int n) {
    double tmp = 0.0;
    int last_i = 0, last_j = 0;
    
    /* This complex construct should generate _LOOPTEMP_ and _REDUCTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:tmp) \
                lastprivate(last_i, last_j) linear(k:1)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int k = i * n + j;
            a[k] = b[i] * c[j];
            tmp += a[k];
            last_i = i;
            last_j = j;
        }
    }
    
    /* Another complex reduction with conditionals */
    double cond_sum = 0.0;
    #pragma omp parallel for simd reduction(+:cond_sum) \
                if(n > 1000)
    for (int i = 0; i < n * n; i++) {
        if (a[i] > 0.5) {
            cond_sum += a[i];
        }
    }
}

/* Function 4: Mixed clauses for _CONDTEMP_ and _SCANTEMP_ generation */
void test_mixed_clauses(int *arr, int n) {
    int scan_var = 0;
    int reduction_var = 0;
    
    #pragma omp parallel for simd reduction(+:reduction_var) \
                linear(scan_var:1)
    for (int i = 0; i < n; i++) {
        int temp = arr[i];
        
        /* Conditional inside SIMD loop may generate _CONDTEMP_ */
        #pragma omp simd reduction(+:reduction_var)
        for (int j = 0; j < M; j++) {
            if (temp > j) {
                reduction_var += temp - j;
            }
        }
        
        scan_var = i;
        #pragma omp scan inclusive(scan_var)
        arr[i] = scan_var;
    }
}

/* Function 5: Target regions with complex data clauses */
void test_target_regions(struct DataBlock *block1, struct DataBlock *block2) {
    #pragma omp target enter data map(to: block1[0:1]) \
                                  map(alloc: block2[0:1])
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block1->values) \
                map(to: block2->values) \
                reduction(+: block1->result)
    for (int i = 0; i < N; i++) {
        block1->values[i] = block1->values[i] * 2.0 + block2->values[i];
        block1->result += block1->values[i];
    }
    
    #pragma omp target exit data map(from: block1->result) \
                                 map(release: block1, block2)
}

int main() {
    /* Initialize data */
    double *array1 = (double*)malloc(N * N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    int *int_array = (int*)malloc(N * sizeof(int));
    
    struct DataBlock block1, block2;
    
    for (int i = 0; i < N; i++) {
        array2[i] = i * 0.1;
        array3[i] = i * 0.05;
        int_array[i] = i % 100;
        block1.values[i] = i * 0.01;
        block1.indices[i] = i;
        block2.values[i] = i * 0.02;
        block2.indices[i] = N - i;
    }
    
    for (int i = 0; i < N * N; i++) {
        array1[i] = 0.0;
    }
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(array2, N);
    test_enter_data(&block1);
    test_internal_temporaries(array1, array2, array3, N);
    test_mixed_clauses(int_array, N);
    test_target_regions(&block1, &block2);
    
    /* Final computation with reduction */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                if(N > 500) lastprivate(block1.result)
    for (int i = 0; i < N; i++) {
        final_result += array2[i] + block1.values[i];
        block1.result = final_result;
    }
    
    printf("Final result: %f\n", final_result);
    printf("Block1 result: %f\n", block1.result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(int_array);
    
    return 0;
}
