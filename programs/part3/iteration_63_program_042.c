/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization of OpenMP conditions */
volatile int g_volatile_flag = 1;
volatile int g_iteration_counter = 0;

/* Function attribute to ensure optimization and tree dumping */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    volatile int local_flag = g_volatile_flag;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators on arrays
     * Forces creation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(local_flag)
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation to inhibit optimization */
        int idx = (i * seed + local_flag) % n;
        arr1[idx] = arr1[idx] + (i % 7);
        arr2[i] = arr2[i] * ((i + seed) % 5 + 1);
        
        sum += arr1[idx] + arr2[i];
        prod *= (arr1[idx] % 10 + 1);
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr2[i] < min_val) min_val = arr2[i];
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, int seed) {
    volatile int cond1 = g_volatile_flag;
    volatile int cond2 = seed % 3;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with if clause in nested context */
        #pragma omp task if(cond2 && (tid % 2))
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] = arr[i] * 2 + tid;
            }
        }
        
        #pragma omp taskwait
        
        /* Teams with if clause (for target offloading context) */
        #pragma omp target teams if(cond1) map(tofrom:arr[0:n/2]) num_teams(2)
        {
            #pragma omp distribute parallel for if(cond2)
            for (int i = 0; i < n/2; i++) {
                arr[i] = arr[i] + omp_get_team_num();
            }
        }
    }
    
    __builtin_printf("Conditional test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, int seed) {
    volatile int scan_flag = g_volatile_flag;
    int partial_sum = 0;
    
    /* Exclusive scan - may generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:partial_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i * seed % 7);
        
        #pragma omp scan exclusive(partial_sum)
        {
            arr[i] = val + partial_sum;
            partial_sum += val;
        }
    }
    
    /* Another scan variant */
    int inclusive_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:inclusive_sum)
    for (int i = 0; i < n; i++) {
        inclusive_sum += arr[i] % 11;
        
        #pragma omp scan inclusive(inclusive_sum)
        arr[i] = arr[i] + inclusive_sum;
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", partial_sum + inclusive_sum);
}

__attribute__((optimize("O2")))
void test_enter_data_with_to(int **dyn_arr_ptr, int n, int seed) {
    /* Dynamic allocation for enter data clause */
    int *device_arr = (int *)malloc(n * sizeof(int));
    if (!device_arr) return;
    
    /* Initialize array with pattern */
    for (int i = 0; i < n; i++) {
        device_arr[i] = (i * seed) % 100;
    }
    
    /* OMP enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_arr[0:n])
    
    /* Use the device array in a target region */
    #pragma omp target teams distribute parallel for map(tofrom: device_arr[0:n])
    for (int i = 0; i < n; i++) {
        device_arr[i] = device_arr[i] * 2 + omp_get_team_num();
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_arr[0:n])
    
    *dyn_arr_ptr = device_arr;
    __builtin_printf("Enter data test completed, device_arr[0]=%d\n", device_arr[0]);
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Use argv for runtime-dependent seed */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int iterations = (seed % 3) + 2;  /* 2-4 iterations */
    
    const int N = 512;
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *dyn_arr = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * 13 + seed) % 1000;
        arr2[i] = (i * 17 + seed) % 1000;
    }
    
    int checksum = 0;
    
    /* Multiple iterations to increase chance of clause generation */
    for (g_iteration_counter = 0; g_iteration_counter < iterations; g_iteration_counter++) {
        volatile int iter_seed = seed + g_iteration_counter * 7;
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(arr1, arr2, N, iter_seed);
        
        /* Test 2: Conditional temporaries */
        test_conditional_temporaries(arr1, N, iter_seed);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(arr2, N, iter_seed);
        
        /* Test 4: Enter data with to modifier */
        test_enter_data_with_to(&dyn_arr, N/4, iter_seed);
        
        /* Update checksum to prevent optimization */
        for (int i = 0; i < N; i += 8) {
            checksum += arr1[i] + arr2[i];
        }
        if (dyn_arr) {
            for (int i = 0; i < N/4; i += 4) {
                checksum += dyn_arr[i];
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", 
                        g_iteration_counter, checksum);
    }
    
    /* Final output to ensure all code paths are used */
    printf("Final checksum: %d\n", checksum);
    printf("Array samples: arr1[0]=%d, arr2[0]=%d\n", arr1[0], arr2[0]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    if (dyn_arr) free(dyn_arr);
    
    return 0;
}
