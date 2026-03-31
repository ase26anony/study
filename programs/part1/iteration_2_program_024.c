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
    
    printf("Scan test: prefix_sum = %f, exclusive_sum = %f\n", 
           prefix_sum, exclusive_sum);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp target enter data map(to: block[0:1]) \
        map(to: block->values[0:N]) \
        map(to: block->indices[0:N])
    
    #pragma omp target
    {
        for (int i = 0; i < N; i++) {
            block->values[i] = block->indices[i] * 2.0;
        }
        block->result = 0.0;
    }
    
    #pragma omp target exit data map(from: block[0:1]) \
        map(from: block->result)
    
    printf("Enter data test: block->result = %f\n", block->result);
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
            double temp = a[i] * b[j];
            sum += temp;
            if (i == n-1 && j == M-1) {
                last_val = temp;
            }
        }
    }
    
    /* Nested with conditional to potentially generate _CONDTEMP_ */
    #pragma omp parallel
    {
        #pragma omp for simd reduction(+:sum) nowait
        for (int i = 0; i < n; i++) {
            #pragma omp simd reduction(+:sum)
            for (int j = 0; j < M; j++) {
                if (a[i] > 0.5) {
                    sum += b[j];
                }
            }
        }
    }
    
    printf("Internal temporaries test: sum = %f, last_val = %f\n", sum, last_val);
}

/* Function 4: Target region with complex mapping */
void test_target_complex(struct DataBlock *blocks, int num_blocks) {
    double total = 0.0;
    
    #pragma omp target teams distribute parallel for \
        map(to: blocks[0:num_blocks]) \
        map(tofrom: total) \
        reduction(+:total) \
        collapse(2)
    for (int b = 0; b < num_blocks; b++) {
        for (int i = 0; i < N; i++) {
            total += blocks[b].values[i] * blocks[b].indices[i];
        }
    }
    
    printf("Target complex test: total = %f\n", total);
}

/* Function 5: Scan directive with conditionals */
void test_scan_with_condition(double *arr, int n) {
    double running_sum = 0.0;
    double running_max = 0.0;
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(running_sum, running_max)
        running_sum += arr[i];
        if (arr[i] > running_max) {
            running_max = arr[i];
        }
        
        /* This might generate _SCANTEMP_ clauses */
        #pragma omp scan exclusive(running_sum)
        arr[i] = running_sum;
    }
    
    printf("Scan with condition: sum = %f, max = %f\n", running_sum, running_max);
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(3 * sizeof(struct DataBlock));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
    }
    
    for (int b = 0; b < 3; b++) {
        for (int i = 0; i < N; i++) {
            blocks[b].values[i] = (double)(i + b) / N;
            blocks[b].indices[i] = i;
        }
        blocks[b].result = 0.0;
    }
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(&blocks[0]);
    test_internal_temporaries(array1, array2, N);
    test_target_complex(blocks, 3);
    test_scan_with_condition(array2, N);
    
    /* Final reduction for verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        lastprivate(array1) if(N > 100)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Test completed.\n");
    
    free(array1);
    free(array2);
    free(blocks);
    
    return 0;
}
