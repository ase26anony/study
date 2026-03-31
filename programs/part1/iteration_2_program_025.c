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
void test_internal_temporaries(double *a, double *b, int n) {
    double last_val = 0.0;
    double reduction_sum = 0.0;
    
    /* This complex construct should generate _LOOPTEMP_ and _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:reduction_sum) \
                             lastprivate(last_val) \
                             linear(i:1) \
                             collapse(2) \
                             schedule(dynamic, 16)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            double temp = a[i] * b[j];
            reduction_sum += temp;
            
            if (j == M-1) {
                last_val = temp;
            }
        }
    }
    
    /* SIMD loop with conditionals - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:reduction_sum) \
                      linear(i:1) \
                      safelen(8)
    for (int i = 0; i < n; i++) {
        double val = a[i];
        if (val > 0.5) {
            val *= 2.0;
        } else {
            val /= 2.0;
        }
        reduction_sum += val;
        a[i] = val;
    }
    
    printf("Reduction sum: %f, Last value: %f\n", reduction_sum, last_val);
}

/* Function 4: Mixed directives with scan for _SCANTEMP_ */
void test_scan_temporaries(double *arr, int n) {
    double scan_var = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:scan_var) nowait
        for (int i = 0; i < n; i++) {
            scan_var += arr[i];
        }
        
        #pragma omp barrier
        
        #pragma omp for ordered(1)
        for (int i = 0; i < n; i++) {
            #pragma omp ordered depend(sink: i-1)
            arr[i] += scan_var;
            #pragma omp ordered depend(source)
            scan_var = arr[i];
        }
    }
}

/* Function 5: Target region with complex data environment */
void test_target_complex(struct DataBlock *block, int threshold) {
    #pragma omp target teams distribute parallel for \
            if(target: threshold > 500) \
            map(tofrom: block->values) \
            map(to: block->indices) \
            reduction(+: block->result) \
            num_teams(4) thread_limit(128)
    for (int i = 0; i < N; i++) {
        block->values[i] = block->indices[i] * 1.5;
        block->result += block->values[i];
    }
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock data_block;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = i * 0.1;
        array2[i] = i * 0.2;
        data_block.values[i] = i * 0.3;
        data_block.indices[i] = i;
    }
    data_block.result = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Test 1: Scan clauses (inclusive/exclusive) */
    test_scan_clauses(array1, N);
    
    /* Test 2: Enter data with to mapper */
    test_enter_data(&data_block);
    
    /* Test 3: Complex constructs for internal temporaries */
    test_internal_temporaries(array1, array2, N);
    
    /* Test 4: Scan temporaries */
    test_scan_temporaries(array2, N);
    
    /* Test 5: Target with conditional */
    test_target_complex(&data_block, N);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                         if(N > 1000) \
                         schedule(guided)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + data_block.values[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Data block result: %f\n", data_block.result);
    
    free(array1);
    free(array2);
    
    return 0;
}
