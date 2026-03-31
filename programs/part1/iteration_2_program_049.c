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
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        // Scan directive with inclusive clause
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
        
        // Scan directive with exclusive clause  
        exclusive_sum = prefix_sum - arr[i];
        #pragma omp scan exclusive(exclusive_sum)
        // Some computation with exclusive scan
        arr[i] += exclusive_sum * 0.5;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    // Enter data with to clause
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices)        \
                           map(alloc: block->result)
    
    // Use the data in target region
    #pragma omp target map(tofrom: block->result) \
                       map(to: block->values[0:N/2], block->indices)
    {
        block->result = 0.0;
        for (int i = 0; i < N/2; i++) {
            block->result += block->values[i] * block->indices[i];
        }
    }
    
    #pragma omp exit data map(release: block->values[0:N/2], block->indices) \
                          map(from: block->result)
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *a, double *b, double *c, int n) {
    double reduction_var = 0.0;
    int lastprivate_var = 0;
    
    // Complex parallel loop with multiple clauses to generate _LOOPTEMP_, _REDUCTEMP_
    #pragma omp parallel for reduction(+:reduction_var) \
                             lastprivate(lastprivate_var) \
                             linear(i:1) \
                             schedule(dynamic, 16) \
                             if(n > 500)
    for (int i = 0; i < n; i++) {
        a[i] = b[i] * c[i];
        reduction_var += a[i];
        lastprivate_var = i;
        
        // Nested loop to increase complexity
        for (int j = 0; j < M; j++) {
            a[i] += 0.001 * j;
        }
    }
    
    // SIMD loop with conditionals to potentially generate _CONDTEMP_
    #pragma omp simd reduction(+:reduction_var) \
                      linear(i:1) \
                      safelen(8)
    for (int i = 0; i < n; i++) {
        if (a[i] > 0.5) {
            b[i] = a[i] * 2.0;
        } else {
            b[i] = a[i] * 0.5;
        }
        reduction_var += b[i];
    }
    
    // Another loop with scan to generate _SCANTEMP_
    double scan_temp = 0.0;
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(scan_temp)
        scan_temp += a[i];
        c[i] = scan_temp;
    }
}

/* Function 4: Target region with complex data clauses */
void test_target_region(struct DataBlock *block1, struct DataBlock *block2) {
    #pragma omp target teams distribute parallel for \
        map(to: block1->values[0:N], block1->indices[0:N]) \
        map(tofrom: block1->result) \
        map(alloc: block2->values[0:N/2]) \
        reduction(+:block1->result) \
        collapse(2) \
        num_teams(4) \
        thread_limit(128)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            block1->values[i] += 0.01 * j;
            block1->result += block1->values[i] * block1->indices[i];
        }
    }
}

/* Main function orchestrates all tests */
int main() {
    // Allocate and initialize data
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    
    struct DataBlock block1, block2;
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(i % 100) / 100.0;
        array3[i] = 0.0;
        block1.values[i] = (double)i;
        block1.indices[i] = i;
        block2.values[i] = (double)(N - i);
        block2.indices[i] = N - i;
    }
    block1.result = 0.0;
    block2.result = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    // Test 1: Scan clauses
    test_scan_clauses(array1, N);
    
    // Test 2: Enter data with to mapper
    test_enter_data(&block1);
    
    // Test 3: Internal temporaries
    test_internal_temporaries(array1, array2, array3, N);
    
    // Test 4: Complex target region
    test_target_region(&block1, &block2);
    
    // Final reduction and output
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                             if(N > 100) \
                             schedule(guided)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + array3[i];
    }
    
    final_sum += block1.result + block2.result;
    
    printf("Final result: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    // Cleanup
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
