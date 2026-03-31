/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Force optimization level for tree dumping */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with array dependencies to force _reductemp_ creation */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) shared(n, flag)
    for (int i = 0; i < n; i++) {
        /* Data-dependent operations to prevent optimization */
        int idx = (i + flag) % n;
        arr1[i] = arr2[idx] * (i + 1);
        sum += arr1[i];
        prod *= (arr1[i] % 10 + 1);  /* Avoid overflow but keep computation */
        if (arr1[i] > max_val) max_val = arr1[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-iteration dependency pattern */
        if (i > 0) {
            arr2[i] = arr1[i-1] + arr2[i];
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, volatile int cond_flag) {
    /* Multiple if clauses with volatile conditions to force _condtemp_ */
    #pragma omp parallel if(cond_flag > 0) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond_flag % 2 == 0)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] *= 2;
                }
            }
            
            #pragma omp task if(cond_flag % 3 == 0)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] /= 2;
                }
            }
        }
    }
    
    /* Nested parallel region with if clause */
    #pragma omp parallel if(cond_flag < 10) num_threads(2)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            arr[i] += i;
        }
    }
    
    __builtin_printf("Conditional temp test complete, cond_flag=%d\n", cond_flag);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int scan_flag) {
    int prefix_sum = 0;
    
    /* Exclusive scan operation - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + scan_flag;
        
        #pragma omp scan exclusive(prefix_sum)
        {
            arr[i] = prefix_sum;
            prefix_sum += val;
        }
    }
    
    /* Another scan pattern with different operator */
    int prefix_max = INT_MIN;
    #pragma omp parallel for reduction(inscan, max:prefix_max)
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        #pragma omp scan exclusive(prefix_max)
        {
            arr[i] = (prefix_max > val) ? prefix_max : val;
            prefix_max = (val > prefix_max) ? val : prefix_max;
        }
    }
    
    __builtin_printf("Scan test complete, final sum=%d, max=%d\n", 
                     prefix_sum, prefix_max);
}

__attribute__((optimize("O2")))
void test_enter_data_clause(int n, volatile int data_flag) {
    /* Use enter data with to clause - should trigger OMP_CLAUSE_ENTER with to modifier */
    int *device_array = (int *)malloc(n * sizeof(int));
    
    if (device_array) {
        /* Initialize array */
        for (int i = 0; i < n; i++) {
            device_array[i] = i * data_flag;
        }
        
        /* Map to device with to clause */
        #pragma omp target enter data map(to: device_array[0:n])
        
        /* Use the data on device */
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < n; i++) {
            device_array[i] *= 2;
        }
        
        /* Retrieve data */
        #pragma omp target exit data map(from: device_array[0:n])
        
        /* Verify and print */
        int checksum = 0;
        for (int i = 0; i < n; i++) {
            checksum += device_array[i];
        }
        __builtin_printf("Enter data test: checksum=%d\n", checksum);
        
        free(device_array);
    }
}

__attribute__((optimize("O2")))
void combined_omp_test(int *arr1, int *arr2, int n, volatile int iter_flag) {
    /* Combined construct to increase clause tree complexity */
    #pragma omp target teams distribute parallel for \
            reduction(+:arr1[:n]) map(tofrom: arr2[:n]) \
            if(iter_flag > 5) num_teams(2) thread_limit(64)
    for (int i = 0; i < n; i++) {
        arr1[i] = arr2[i] * iter_flag;
        arr2[i] = arr1[i] + i;
    }
    
    /* Nested reduction inside parallel region */
    #pragma omp parallel
    {
        int local_sum = 0;
        #pragma omp for reduction(+:local_sum) nowait
        for (int i = 0; i < n; i++) {
            local_sum += arr1[i];
        }
        
        #pragma omp critical
        {
            arr2[0] += local_sum;
        }
    }
    
    __builtin_printf("Combined test iteration %d complete\n", iter_flag);
}

int main(int argc, char *argv[]) {
    /* Use argc for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int flag1 = seed % 7;
    volatile int flag2 = seed % 11;
    volatile int flag3 = seed % 13;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed + 12345) % 1000;
        array2[i] = (i * seed * 2 + 54321) % 1000;
    }
    
    /* Multiple iterations to ensure clause processing */
    volatile int iterations = 3;
    for (int iter = 0; iter < iterations; iter++) {
        volatile int current_flag = flag1 + iter;
        
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(array1, array2, N, current_flag);
        
        /* Test 2: Conditional temporaries */
        test_conditional_temporaries(array1, N, flag2 + iter);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(array2, N, flag3 + iter);
        
        /* Test 4: Combined constructs */
        combined_omp_test(array1, array2, N, current_flag);
        
        /* Test 5: Enter data clause (conditionally based on iteration) */
        if (iter % 2 == 0) {
            test_enter_data_clause(N/2, current_flag);
        }
        
        /* Compute checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += array1[i] + array2[i];
        }
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    /* Final output to prevent dead code elimination */
    int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result ^= array1[i];
        final_result ^= array2[i];
    }
    printf("Final XOR result: %d\n", final_result);
    
    free(array1);
    free(array2);
    
    return 0;
}
