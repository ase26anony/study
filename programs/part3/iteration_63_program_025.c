/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile int g_loop_counter = 0;
volatile int g_seed = 42;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temp(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i * g_seed) % n;
        if (idx < 0) idx = -idx;
        
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero product */
        if (arr1[i] > max_val) max_val = arr1[i];
        if (arr2[i % n] < min_val) min_val = arr2[i % n];
        
        /* Cross-update to create dependencies */
        if (i % 3 == 0) {
            arr1[(i + 1) % n] += arr2[i % n] % 7;
        }
    }
    
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temp(int *arr, int n, volatile int cond) {
    /* Multiple if clauses in different OpenMP contexts */
    
    /* Parallel region with if clause */
    #pragma omp parallel if(cond > 0) num_threads(4)
    {
        #pragma omp single
        {
            /* Task with if clause */
            #pragma omp task if(cond < 10) shared(arr)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] += g_volatile_flag;
                }
            }
            
            /* Another task with different if condition */
            #pragma omp task if(cond > 5) shared(arr)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] -= g_volatile_flag;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond % 2 == 0) map(tofrom:arr[0:n]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for simd
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 2 - 1;
        }
    }
    
    __builtin_printf("Conditional temp test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2")))
void test_scan_temp(int *arr, int n) {
    int scan_sum = 0;
    int exclusive_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:exclusive_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % 5);
        exclusive_sum += val;
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] = exclusive_sum - val;  /* Exclusive prefix */
    }
    
    /* Inclusive scan with inscan reduction */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] % 100;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2")))
void test_enter_data(int **ptr_arr, int n) {
    /* Dynamic allocation for enter data clause */
    int *dyn_arr = (int *)malloc(n * sizeof(int));
    if (!dyn_arr) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        dyn_arr[i] = i * g_seed % 100;
    }
    
    /* Enter data with to modifier */
    #pragma omp enter data to(dyn_arr[0:n]) depend(out: dyn_arr)
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: dyn_arr[0:n])
    {
        for (int i = 0; i < n; i++) {
            dyn_arr[i] = dyn_arr[i] * 3 + 7;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(dyn_arr[0:n]) depend(in: dyn_arr)
    
    /* Store pointer for checksum */
    *ptr_arr = dyn_arr;
    
    __builtin_printf("Enter data test completed, dyn_arr[0]=%d\n", dyn_arr[0]);
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Use argc to seed variability */
    if (argc > 1) {
        g_seed = atoi(argv[1]);
    }
    
    const int N = 512;
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *dyn_arr = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * g_seed + 17) % 1000;
        arr2[i] = (i * g_seed * 3 + 23) % 1000;
    }
    
    int checksum = 0;
    
    /* Multiple iterations to increase chance of temporary generation */
    for (g_loop_counter = 0; g_loop_counter < 3; g_loop_counter++) {
        volatile int iter_flag = g_loop_counter + g_seed;
        
        /* 1. Test reduction temporaries */
        test_reduction_temp(arr1, arr2, N, iter_flag);
        
        /* 2. Test conditional temporaries */
        test_conditional_temp(arr1, N, iter_flag);
        
        /* 3. Test scan temporaries */
        test_scan_temp(arr2, N);
        
        /* 4. Test enter data with to modifier */
        test_enter_data(&dyn_arr, N/2);
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum ^= arr1[i] + arr2[i];
            checksum = (checksum << 1) | (checksum >> 31); /* rotate */
        }
        
        if (dyn_arr) {
            for (int i = 0; i < N/2; i++) {
                checksum ^= dyn_arr[i];
            }
            free(dyn_arr);
            dyn_arr = NULL;
        }
        
        __builtin_printf("Iteration %d checksum: %08x\n", 
                        g_loop_counter, checksum);
        
        /* Modify seed for next iteration */
        g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output to ensure all code paths are considered */
    int final_result = 0;
    for (int i = 0; i < N; i += 64) {
        final_result += arr1[i] - arr2[i];
    }
    
    __builtin_printf("Final result: %d\n", final_result);
    __builtin_printf("Final checksum: %08x\n", checksum);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
