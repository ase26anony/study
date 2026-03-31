/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_bound = 512;
volatile int v_seed = 42;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int iter) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2)
    for (i = 0; i < n; i++) {
        /* Data-dependent index calculation to inhibit optimization */
        int idx = (i + v_seed + iter) % n;
        int idx2 = (i * 7 + v_seed) % n;
        
        sum += arr1[idx] + arr2[idx2];
        prod *= (arr1[idx] % 10 + 1);  /* Avoid overflow but keep computation */
        
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr2[idx2] < min_val) min_val = arr2[idx2];
        
        /* Cross-update arrays to create dependencies */
        if (i % 3 == 0) {
            arr1[idx] = (arr1[idx] + arr2[idx2]) % 100;
        }
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction iter %d: sum=%d, prod=%d, max=%d, min=%d\n",
                     iter, sum, prod, max_val, min_val);
}

__attribute__((optimize("O2"), noinline))
void test_conditional_temporaries(int *arr, int n, int iter) {
    int i;
    
    /* Parallel region with volatile condition */
    #pragma omp parallel if(v_flag1 || iter > 0) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with another volatile condition */
        #pragma omp task if(v_flag2 || tid % 2 == 0)
        {
            for (i = tid * (n/4); i < (tid + 1) * (n/4) && i < n; i++) {
                arr[i] = (arr[i] * 3 + iter) % 1000;
            }
        }
        
        #pragma omp taskwait
        
        /* Target teams with condition */
        #pragma omp target teams if(iter % 2 == 0) map(tofrom: arr[0:n/2]) thread_limit(4)
        {
            #pragma omp distribute parallel for simd
            for (i = 0; i < n/2; i++) {
                arr[i] = arr[i] + tid;
            }
        }
    }
    
    __builtin_printf("Conditional temporaries iter %d complete\n", iter);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int n, int iter) {
    int i;
    int scan_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i) shared(arr)
    for (i = 0; i < n; i++) {
        int val = arr[i] + (i % 7);
        
        /* Exclusive scan - value before inclusion */
        #pragma omp scan exclusive(scan_sum)
        {
            arr[i] = scan_sum + val;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan with different pattern */
    scan_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
            simdlen(4) shared(arr)
    for (i = n/2; i < n; i++) {
        int val = arr[i] * 2 - iter;
        
        /* Inclusive scan */
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan temporaries iter %d: final sum=%d\n", iter, scan_sum);
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(int **ptr_arr, int n, int iter) {
    /* Dynamically allocate for enter data clause */
    int *device_arr = (int *)malloc(n * sizeof(int));
    
    if (device_arr) {
        /* Initialize array */
        for (int i = 0; i < n; i++) {
            device_arr[i] = i * 3 + iter;
        }
        
        /* Use enter data with to clause - this should trigger OMP_CLAUSE_ENTER with to modifier */
        #pragma omp target enter data map(to: device_arr[0:n]) \
                depend(inout: device_arr) nowait
        
        /* Perform computation on device */
        #pragma omp target teams distribute parallel for \
                map(always, tofrom: device_arr[0:n]) \
                is_device_ptr(device_arr)
        for (int i = 0; i < n; i++) {
            device_arr[i] = device_arr[i] * 2 + 1;
        }
        
        /* Retrieve data */
        #pragma omp target exit data map(from: device_arr[0:n]) \
                depend(inout: device_arr)
        
        /* Store result */
        *ptr_arr = device_arr;
        
        __builtin_printf("Enter data clause iter %d completed\n", iter);
    }
}

__attribute__((optimize("O3"), noinline))
void nested_combined_constructs(int *arr1, int *arr2, int n, int iter) {
    /* Nested parallel regions with combined constructs */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp master
        {
            #pragma omp taskloop reduction(+:arr1[0:n]) \
                    grainsize(64) num_tasks(4)
            for (int i = 0; i < n; i++) {
                arr1[i] = (arr1[i] + iter) % 100;
            }
        }
        
        #pragma omp barrier
        
        /* Combined target teams distribute parallel for */
        #pragma omp target teams distribute parallel for \
                map(tofrom: arr2[0:n/2]) reduction(max: v_seed)
        for (int i = 0; i < n/2; i++) {
            arr2[i] = arr2[i] * arr1[i] + v_seed;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    v_seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    const int N = v_bound;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *device_array = NULL;
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 13 + v_seed) % 1000;
        array2[i] = (i * 17 + v_seed * 2) % 1000;
    }
    
    /* Volatile loop counter to force multiple instantiations */
    volatile int repeat_count = (v_seed % 3) + 2;
    
    for (int iter = 0; iter < repeat_count; iter++) {
        int checksum = 0;
        
        /* Test all clause types in sequence */
        test_reduction_temporaries(array1, array2, N, iter);
        test_conditional_temporaries(array1, N, iter);
        test_scan_temporaries(array2, N, iter);
        test_enter_data_clause(&device_array, N/4, iter);
        nested_combined_constructs(array1, array2, N, iter);
        
        /* Calculate checksum to prevent optimization */
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + array1[i]) % 1000000;
            checksum = (checksum * 31 + array2[i]) % 1000000;
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify volatile flags for next iteration */
        v_flag1 = (iter % 2 == 0);
        v_flag2 = !v_flag1;
        v_seed = (v_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    __builtin_printf("Final sum: %d\n", final_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    if (device_array) free(device_array);
    
    return 0;
}
