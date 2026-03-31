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
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum) private(exclusive_prefix)
    for (int i = 0; i < n; i++) {
        // Scan inclusive clause
        #pragma omp scan inclusive(partial_sum)
        partial_sum += arr[i];
        arr[i] = partial_sum;
        
        // Scan exclusive clause  
        #pragma omp scan exclusive(exclusive_prefix)
        exclusive_prefix += arr[i] * 0.5;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices) \
                           map(to: block->result)
    
    // Use the data
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N/2])
    for (int i = 0; i < N/2; i++) {
        block->values[i] *= 2.0;
    }
    
    #pragma omp exit data map(from: block->values[0:N/2]) \
                          map(release: block->indices) \
                          map(from: block->result)
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double sum = 0.0;
    double last_val = 0.0;
    
    // This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ clauses
    #pragma omp parallel for reduction(+:sum) lastprivate(last_val) \
                linear(i:1) collapse(2) if(n > 500)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            double temp = a[i] * b[j];
            sum += temp;
            if (j == M-1) last_val = temp;
        }
    }
    
    // Another complex case with SIMD
    #pragma omp simd reduction(+:sum) linear(i:2) \
                simdlen(8) if(n > 1000)
    for (int i = 0; i < n; i += 2) {
        sum += a[i] * 3.14;
    }
}

/* Function 4: Conditional temporaries with scan */
void test_conditional_temporaries(double *arr, int n) {
    double scan_var = 0.0;
    int flag = 0;
    
    #pragma omp parallel for reduction(+:scan_var) lastprivate(flag)
    for (int i = 0; i < n; i++) {
        // Conditional inside loop - may generate _CONDTEMP_
        if (arr[i] > 0.5) {
            #pragma omp scan inclusive(scan_var)
            scan_var += arr[i];
            flag = i;
        } else {
            #pragma omp atomic
            scan_var -= 0.1;
        }
    }
}

/* Function 5: Target regions with complex mappings */
void test_target_temporaries(struct DataBlock *block) {
    double local_sum = 0.0;
    
    #pragma omp target teams distribute parallel for \
                map(to: block->values) \
                map(tofrom: local_sum) \
                reduction(+:local_sum) \
                if(N > 1000)
    for (int i = 0; i < N; i++) {
        local_sum += block->values[i];
        block->indices[i] = i;
    }
    
    block->result = local_sum;
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock data_block;
    
    // Initialize data
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
        data_block.values[i] = array1[i];
        data_block.indices[i] = i;
    }
    data_block.result = 0.0;
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    // Test 1: Scan clauses
    test_scan_clauses(array1, N);
    
    // Test 2: Enter data with to mapper
    test_enter_data(&data_block);
    
    // Test 3: Internal temporaries
    test_internal_temporaries(array1, array2, N);
    
    // Test 4: Conditional temporaries
    test_conditional_temporaries(array2, N);
    
    // Test 5: Target temporaries
    test_target_temporaries(&data_block);
    
    // Final reduction for verification
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                if(N > 500) schedule(dynamic, 16)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Data block result: %f\n", data_block.result);
    
    free(array1);
    free(array2);
    
    return 0;
}
