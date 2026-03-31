/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int volatile_flag = 1;
volatile int volatile_bound = 100;
volatile int volatile_seed = 42;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2")))
void complex_reductions(int *arr1, int *arr2, int n, int iter) {
    int sum = 0;
    int product = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(volatile_flag)
    for (int i = 0; i < n; i++) {
        /* Data-dependent index calculation to inhibit optimization */
        int idx = (i * iter + volatile_seed) % n;
        if (idx < 0) idx = 0;
        if (idx >= n) idx = n - 1;
        
        sum += arr1[idx] + arr2[i % n];
        product *= (arr1[i] % 10 + 1);  /* Avoid multiplication by zero */
        if (arr1[i] > max_val) max_val = arr1[i];
        if (arr2[i] < min_val) min_val = arr2[i];
        
        /* Cross-update to create dependencies */
        if (i % 3 == 0) {
            arr1[(i + 1) % n] += arr2[i] % 7;
        }
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction iter %d: sum=%d, product=%d, max=%d, min=%d\n",
                     iter, sum, product, max_val, min_val);
}

__attribute__((optimize("O2")))
void conditional_parallelism(int *arr, int n, int iter) {
    volatile int dynamic_flag = iter % 2;
    
    /* Multiple if clauses in different OpenMP contexts */
    #pragma omp parallel if(dynamic_flag) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(volatile_flag && (iter > 0))
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] += i * iter;
                }
            }
            
            #pragma omp task if(!volatile_flag || (iter < 3))
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] -= i * iter;
                }
            }
        }
        
        /* Teams with if clause */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] % 1000;
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(iter == 1) map(tofrom:arr[0:n]) \
            num_teams(2) thread_limit(32)
    #pragma omp distribute parallel for
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
    
    __builtin_printf("Conditional iter %d complete\n", iter);
}

__attribute__((optimize("O2")))
void scan_operations(int *arr, int n, int iter) {
    int scan_temp = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_temp)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % (iter + 1));
        
        #pragma omp scan exclusive(scan_temp)
        {
            arr[i] = scan_temp;
            scan_temp += val;
        }
    }
    
    /* Inclusive scan with different pattern */
    scan_temp = 0;
    #pragma omp parallel for reduction(inscan, +:scan_temp) \
            if(volatile_flag)
    for (int i = 0; i < n; i++) {
        scan_temp += arr[i] % 17;
        
        #pragma omp scan inclusive(scan_temp)
        arr[i] = scan_temp;
    }
    
    __builtin_printf("Scan iter %d: final temp=%d\n", iter, scan_temp);
}

__attribute__((optimize("O2")))
void enter_data_with_to(int **ptr_arr, int n, int iter) {
    /* Dynamic allocation for enter data clause */
    int *device_data = (int *)malloc(n * sizeof(int));
    if (!device_data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        device_data[i] = i * iter + volatile_seed;
    }
    
    /* Use enter data with to modifier */
    #pragma omp enter data to(device_data[0:n])
    
    /* Simulate target region */
    #pragma omp target map(tofrom:device_data[0:n]) if(iter > 0)
    #pragma omp teams distribute parallel for
    for (int i = 0; i < n; i++) {
        device_data[i] *= 2;
        device_data[i] += i;
    }
    
    /* Copy back and use */
    #pragma omp exit data from(device_data[0:n])
    
    /* Store pointer for checksum */
    ptr_arr[iter % 2] = device_data;
    
    __builtin_printf("Enter data iter %d: first=%d, last=%d\n",
                     iter, device_data[0], device_data[n-1]);
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *ptr_array[2] = {NULL, NULL};
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    unsigned int lcg = volatile_seed;
    for (int i = 0; i < N; i++) {
        lcg = lcg * 1103515245 + 12345;
        array1[i] = (lcg >> 16) % 1000;
        array2[i] = (lcg >> 8) % 1000;
    }
    
    volatile int repeat_count = (volatile_seed % 3) + 2;
    
    /* Main test loop - triggers multiple OpenMP clause generations */
    for (int iter = 0; iter < repeat_count; iter++) {
        volatile_flag = iter % 2;
        volatile_bound = 100 + iter * 50;
        
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Trigger _reductemp_ clauses */
        complex_reductions(array1, array2, N, iter);
        
        /* 2. Trigger _condtemp_ clauses */
        conditional_parallelism(array1, N, iter);
        
        /* 3. Trigger _scantemp_ clauses */
        scan_operations(array2, N, iter);
        
        /* 4. Trigger enter with to modifier */
        enter_data_with_to(ptr_array, N/4, iter);
        
        /* Compute checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += array1[i] + array2[i];
            checksum = checksum % 1000000;
        }
        __builtin_printf("Checksum after iter %d: %d\n", iter, checksum);
    }
    
    /* Final output and cleanup */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] * 3 + array2[i] * 7;
    }
    __builtin_printf("\nFinal result: %d\n", final_sum);
    
    free(array1);
    free(array2);
    for (int i = 0; i < 2; i++) {
        if (ptr_array[i]) free(ptr_array[i]);
    }
    
    return 0;
}
