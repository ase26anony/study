/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 2000
#define M 100

/* Structure to test complex data mappings */
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
        block->values[i] = block->indices[i] * 2.0;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values[0:N/2])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double tmp = 0.0;
    int last_val = 0;
    
    /* This complex construct should generate _LOOPTEMP_ and _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:tmp) lastprivate(last_val) \
                linear(i:1) collapse(2) schedule(dynamic, 16)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            double local = a[i] * b[j];
            tmp += local;
            
            #pragma omp simd reduction(+:tmp) linear(k:1)
            for (int k = 0; k < 10; k++) {
                tmp += k * 0.1;
            }
            
            last_val = i * j;
        }
    }
    
    /* Another construct with conditionals to generate _CONDTEMP_ */
    #pragma omp parallel for simd reduction(+:tmp) \
                if(n > 1000) schedule(static)
    for (int i = 0; i < n; i++) {
        if (a[i] > 0.5) {
            tmp += a[i] * 2.0;
        } else {
            tmp += a[i] * 0.5;
        }
    }
}

/* Function 4: Mixed clauses for _SCANTEMP_ generation */
void test_scan_temp(double *arr, int n) {
    double scan_var = 0.0;
    
    #pragma omp parallel for reduction(+:scan_var)
    for (int i = 0; i < n; i++) {
        double temp = arr[i];
        
        #pragma omp scan inclusive(scan_var)
        arr[i] = scan_var;
        scan_var += temp;
        
        /* Nested scan to increase complexity */
        #pragma omp scan exclusive(temp)
        if (i % 2 == 0) {
            arr[i] += temp;
        }
    }
}

/* Function 5: Target regions with complex mappings */
void test_target_complex(struct DataBlock *block) {
    #pragma omp target enter data map(to: block[0:2])
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: block[0:2].values[0:N]) \
            reduction(+:block[0].result, block[1].result)
    for (int i = 0; i < N; i++) {
        block[0].values[i] = i * 1.5;
        block[1].values[i] = i * 2.5;
        block[0].result += block[0].values[i];
        block[1].result += block[1].values[i];
    }
    
    #pragma omp target exit data map(from: block[0:2].result) \
                          map(release: block[0:2])
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(2 * sizeof(struct DataBlock));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
        blocks[0].indices[i] = i;
        blocks[1].indices[i] = N - i;
    }
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(&blocks[0]);
    test_internal_temporaries(array1, array2, N);
    test_scan_temp(array2, N);
    test_target_complex(blocks);
    
    /* Final reduction and output */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                if(N > 500) schedule(guided)
    for (int i = 0; i < N; i++) {
        final_result += array1[i] + array2[i];
    }
    
    final_result += blocks[0].result + blocks[1].result;
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(blocks);
    
    return 0;
}
