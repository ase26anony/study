/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int volatile_flag = 1;
volatile int volatile_seed = 42;
volatile int volatile_iter = 2;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2")))
void complex_reductions(int *arr1, int *arr2, int n, int seed) {
    volatile int vseed = seed;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(vseed)
    for (int i = 0; i < n; i++) {
        /* Data-dependent index calculation */
        int idx = (i * vseed) % n;
        if (idx < 0) idx = -idx;
        
        /* Multiple reduction operations */
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);
        if (arr2[idx] > max_val) max_val = arr2[idx];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-update to prevent optimization */
        arr1[i] = (arr1[i] + arr2[idx]) % 1000;
    }
    
    __builtin_printf("Reductions: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void conditional_constructs(int *arr, int n, int seed) {
    volatile int cond_flag = seed % 3;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond_flag > 0) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond_flag > 1) shared(arr)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] *= 2;
                }
            }
            
            #pragma omp task if(cond_flag < 2) shared(arr)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] /= 2;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(volatile_flag) map(tofrom:arr[0:n/4]) \
            num_teams(2) thread_limit(8)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n/4; i++) {
            arr[i] = arr[i] + i;
        }
    }
    
    __builtin_printf("Conditional constructs completed\n");
}

__attribute__((optimize("O2")))
void scan_operations(int *arr, int n, int seed) {
    int scan_sum = 0;
    int exclusive_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:exclusive_sum)
    for (int i = 0; i < n; i++) {
        exclusive_sum += arr[i];
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] = exclusive_sum - arr[i];  /* Exclusive prefix */
    }
    
    /* Inclusive scan with inscan reduction */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        scan_sum += arr[i];
        #pragma omp scan inclusive(scan_sum)
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan operations: final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2")))
void enter_data_with_to(int **ptr_arr, int n) {
    /* Allocate and initialize data */
    int *device_data = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        device_data[i] = i * volatile_flag;
    }
    
    /* Use enter data with to modifier */
    #pragma omp enter data to(device_data[0:n])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: device_data[0:n/2]) \
            map(to: device_data[n/2:n/2])
    {
        #pragma omp parallel for
        for (int i = 0; i < n/2; i++) {
            device_data[i] += device_data[n - i - 1];
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(device_data[0:n])
    
    *ptr_arr = device_data;
    __builtin_printf("Enter data with to completed\n");
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Initialize with command-line seed for runtime variability */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    volatile_seed = seed;
    
    const int N = 512;
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *scan_arr = (int *)malloc(N * sizeof(int));
    int *enter_data_arr = NULL;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * seed + 13) % 1000;
        arr2[i] = (i * seed * 7 + 29) % 1000;
        scan_arr[i] = (i + seed) % 100;
    }
    
    int checksum = 0;
    
    /* Multiple iterations based on volatile counter */
    for (int iter = 0; iter < volatile_iter; iter++) {
        volatile_flag = (iter % 2) + 1;
        
        /* 1. Complex reductions to trigger _reductemp_ */
        complex_reductions(arr1, arr2, N, seed + iter);
        
        /* 2. Conditional constructs to trigger _condtemp_ */
        conditional_constructs(arr1, N, seed + iter * 3);
        
        /* 3. Scan operations to trigger _scantemp_ */
        scan_operations(scan_arr, N, seed + iter * 5);
        
        /* 4. Enter data with to modifier */
        enter_data_with_to(&enter_data_arr, N/2);
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + arr1[i]) % 1000000;
            checksum = (checksum * 31 + arr2[i]) % 1000000;
            checksum = (checksum * 31 + scan_arr[i]) % 1000000;
        }
        
        if (enter_data_arr) {
            for (int i = 0; i < N/2; i++) {
                checksum = (checksum * 31 + enter_data_arr[i]) % 1000000;
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    /* Final output to ensure all code paths are used */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(scan_arr);
    if (enter_data_arr) free(enter_data_arr);
    
    return 0;
}
