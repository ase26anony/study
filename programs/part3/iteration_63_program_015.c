/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier.
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
     * to force creation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i * 17 + flag) % n;
        sum += arr1[idx] + arr2[i % n];
        
        /* Avoid multiplication by zero for product reduction */
        if (arr1[idx] != 0 && arr2[i % n] != 0) {
            prod *= (arr1[idx] % 10 + 1) * (arr2[i % n] % 10 + 1);
        }
        
        /* Conditional updates for max/min */
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr2[i % n] < min_val) min_val = arr2[i % n];
        
        /* Cross-update arrays to create dependencies */
        if (i % 3 == 0) {
            arr1[idx] = (arr1[idx] + arr2[i % n]) % 100;
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, volatile int cond_flag) {
    volatile int dynamic_cond = cond_flag;
    
    /* Multiple if clauses in different OpenMP contexts to force _condtemp_ */
    
    /* Parallel region with if clause */
    #pragma omp parallel if(dynamic_cond > 5) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with if clause */
        #pragma omp task if(tid % 2 == 0 && dynamic_cond < 10)
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] = (arr[i] * 3 + 7) % 100;
            }
        }
        
        #pragma omp taskwait
        
        /* Teams with if clause (for target offloading) */
        #pragma omp target teams if(dynamic_cond > 2 && dynamic_cond < 8) \
                num_teams(2) thread_limit(32)
        {
            #pragma omp distribute parallel for
            for (int i = 0; i < n; i++) {
                arr[i] = (arr[i] + tid) % 100;
            }
        }
    }
    
    __builtin_printf("Conditional test completed with flag=%d\n", dynamic_cond);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int scan_type) {
    int prefix_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:prefix_sum) \
            if(scan_type == 0)
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        #pragma omp scan exclusive(prefix_sum)
        {
            arr[i] = prefix_sum;
            prefix_sum += val;
        }
    }
    
    /* Inclusive scan */
    int running_sum = 0;
    #pragma omp parallel for reduction(inscan, +:running_sum) \
            if(scan_type == 1)
    for (int i = 0; i < n; i++) {
        int val = arr[i] % 50;
        
        #pragma omp scan inclusive(running_sum)
        {
            running_sum += val;
            arr[i] = running_sum;
        }
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", prefix_sum + running_sum);
}

__attribute__((optimize("O2")))
void test_enter_data_with_to(int *data, int n, volatile int use_device) {
    /* Allocate device memory with enter data and to clause */
    int *device_data = (int *)malloc(n * sizeof(int));
    memcpy(device_data, data, n * sizeof(int));
    
    /* Use enter data with to modifier - this should trigger OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_data[0:n]) \
            if(use_device > 0)
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for \
            if(use_device > 0) map(tofrom: device_data[0:n])
    for (int i = 0; i < n; i++) {
        device_data[i] = device_data[i] * 2 + 1;
    }
    
    /* Copy back and free */
    #pragma omp target exit data map(from: device_data[0:n]) \
            if(use_device > 0)
    
    memcpy(data, device_data, n * sizeof(int));
    free(device_data);
    
    __builtin_printf("Enter data test completed, use_device=%d\n", use_device);
}

/* Main test function with nested OpenMP regions */
__attribute__((optimize("O2")))
void comprehensive_omp_test(int *arr1, int *arr2, int n, 
                           volatile int iter, volatile int seed) {
    
    /* Outer parallel region */
    #pragma omp parallel if(iter > 1) num_threads(2)
    {
        /* Nested reduction */
        #pragma omp for reduction(+:seed) nowait
        for (int i = 0; i < n; i++) {
            arr1[i] = (arr1[i] + seed) % 1000;
        }
        
        #pragma omp barrier
        
        /* Combined construct */
        #pragma omp target teams distribute parallel for \
                reduction(max:seed) if(iter % 2 == 0)
        for (int i = 0; i < n; i++) {
            arr2[i] = (arr2[i] * 3 - seed) % 1000;
        }
    }
    
    /* Calculate checksum */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i];
    }
    __builtin_printf("Iteration %d checksum: %lld\n", iter, checksum);
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int use_device = (argc > 2) ? atoi(argv[2]) : 0;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(seed);
    for (int i = 0; i < N; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
    }
    
    /* Volatile flags to control execution paths */
    volatile int reduction_flag = seed % 3;
    volatile int cond_flag = seed % 10;
    volatile int scan_flag = seed % 2;
    volatile int enter_flag = use_device;
    
    /* Multiple iterations to increase chance of temporary generation */
    volatile int iterations = 3;
    for (volatile int iter = 0; iter < iterations; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, reduction_flag + iter);
        
        /* 2. Test conditional temporaries */
        test_conditional_temporaries(array1, N, cond_flag + iter);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, scan_flag);
        
        /* 4. Test enter data with to modifier */
        test_enter_data_with_to(array1, N, enter_flag);
        
        /* 5. Comprehensive nested test */
        comprehensive_omp_test(array1, array2, N, iter, seed);
        
        /* Modify flags for next iteration */
        reduction_flag = (reduction_flag * 13 + 7) % 5;
        cond_flag = (cond_flag * 17 + 11) % 12;
        scan_flag ^= 1;
    }
    
    /* Final checksum to prevent optimization */
    long long final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] * array2[i];
    }
    __builtin_printf("\nFinal checksum: %lld\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
