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
    
    /* Complex reduction with multiple operators and data-dependent access
     * This should generate _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i * 17 + 31) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero product */
        if (arr2[i] > max_val) max_val = arr2[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-update to force temporary creation */
        if (i % 3 == 0) {
            arr1[(i + 1) % n] += arr2[i] % 7;
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d prod=%d max=%d min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_condition_temporaries(int *arr, int n, volatile int cond1, volatile int cond2) {
    /* Multiple if clauses in different contexts to generate _condtemp_ */
    
    /* Parallel region with runtime-dependent condition */
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        /* Task with another condition */
        #pragma omp task if(cond2 == 0) shared(arr)
        {
            for (int i = 0; i < n/2; i++) {
                arr[i] *= 2;
            }
        }
        
        #pragma omp task if(cond1 + cond2 > 5) shared(arr)
        {
            for (int i = n/2; i < n; i++) {
                arr[i] /= (arr[i] % 5 + 1);
            }
        }
        
        #pragma omp taskwait
        
        /* Nested parallel with condition */
        #pragma omp parallel if(cond1 * cond2 < 10) num_threads(2)
        {
            #pragma omp for
            for (int i = 0; i < n; i++) {
                arr[i] += i;
            }
        }
    }
    
    /* Target teams with condition */
    #pragma omp target teams if(cond1 != cond2) map(tofrom:arr[0:n]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] % 100;
        }
    }
    
    __builtin_printf("Condition test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int iter) {
    int scan_sum = 0;
    int exclusive_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:exclusive_sum) \
            private(arr) if(iter > 0)
    for (int i = 0; i < n; i++) {
        exclusive_sum += arr[i];
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] = exclusive_sum - arr[i];  /* Exclusive prefix */
    }
    
    /* Inclusive scan with inscan */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(arr) if(iter < 5)
    for (int i = 0; i < n; i++) {
        scan_sum += arr[i] % 13;
        #pragma omp scan inclusive(scan_sum)
        arr[i] = scan_sum;
    }
    
    /* Combined parallel scan */
    #pragma omp parallel num_threads(4) if(iter % 2 == 0)
    {
        #pragma omp for scan(+:scan_sum)
        for (int i = 0; i < n; i++) {
            scan_sum += i;
            arr[i] += scan_sum;
        }
    }
    
    __builtin_printf("Scan results: exclusive=%d inclusive=%d\n", 
                     exclusive_sum, scan_sum);
}

__attribute__((optimize("O2")))
void test_enter_data_clause(int n, volatile int flag) {
    /* Dynamic allocation for enter data clause */
    int *device_array = (int *)malloc(n * sizeof(int));
    if (!device_array) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        device_array[i] = i * i + 1;
    }
    
    /* OMP enter clause with to modifier - should trigger OMP_CLAUSE_ENTER with to */
    #pragma omp enter data to(device_array[0:n]) if(flag > 0)
    
    /* Use in target region */
    #pragma omp target map(tofrom: device_array[0:n]) if(flag > 0)
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < n; i++) {
            device_array[i] *= 2;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(device_array[0:n]) if(flag > 0)
    
    __builtin_printf("Enter data test: device_array[%d]=%d\n", 
                     n/2, device_array[n/2]);
    
    free(device_array);
}

/* Main test function that combines all patterns */
__attribute__((optimize("O2")))
void run_omp_tests(int seed, volatile int outer_flag) {
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        free(array1);
        free(array2);
        return;
    }
    
    /* Initialize with pseudo-random values based on seed */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed + 17) % 1000;
        array2[i] = (i * seed * 3 + 23) % 1000;
    }
    
    int checksum = 0;
    
    /* Multiple iterations to increase chance of temporary generation */
    for (volatile int iter = 0; iter < 3; iter++) {
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(array1, array2, N, outer_flag + iter);
        
        /* Update checksum */
        for (int i = 0; i < N; i += 8) {
            checksum += array1[i] + array2[i];
        }
        
        /* Test 2: Condition temporaries */
        test_condition_temporaries(array1, N, outer_flag, iter);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(array2, N, iter);
        
        /* Test 4: Enter data clause */
        test_enter_data_clause(256, outer_flag);
        
        /* More checksum updates */
        for (int i = 1; i < N; i += 8) {
            checksum += array1[i] * array2[i];
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    /* Final output to prevent optimization */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) if(outer_flag > -100)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    __builtin_printf("Final result: sum=%d checksum=%d\n", final_sum, checksum);
    
    free(array1);
    free(array2);
}

int main(int argc, char *argv[]) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int flag = seed % 7;  /* Volatile to prevent constant folding */
    
    /* Run tests multiple times with different seeds */
    for (volatile int run = 0; run < 2; run++) {
        run_omp_tests(seed + run, flag + run);
    }
    
    return 0;
}
