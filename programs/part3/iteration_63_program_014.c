/* test_omp_clause_printing.c
 * Compile with: gcc -std=gnu11 -O2 -fopenmp -fdump-tree-all -o test_omp test_omp_clause_printing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int g_flag1 = 1;
volatile int g_flag2 = 0;
volatile int g_iter = 2;

/* Function attribute to force optimization level */
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
        int idx = (i + vseed) % n;
        idx = (idx < 0) ? 0 : idx;
        
        /* Multiple reduction operations */
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);
        
        if (arr2[i] > max_val) max_val = arr2[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-update to inhibit optimization */
        arr1[(i + 1) % n] += arr2[i] % 7;
    }
    
    __builtin_printf("Reductions: sum=%d prod=%d max=%d min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void conditional_parallelism(int *arr, int n, int seed) {
    volatile int cond1 = g_flag1;
    volatile int cond2 = seed % 3;
    
    /* OMP_CLAUSE__CONDTEMP_ should appear here */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond2 > 0)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] += i * seed;
                }
            }
            
            #pragma omp task if(cond2 <= 0)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] -= i * seed;
                }
            }
        }
    }
    
    /* Another conditional construct */
    #pragma omp target teams if(cond1 && cond2) num_teams(2)
    {
        #pragma omp distribute parallel for if(cond1)
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] % 100;
        }
    }
    
    __builtin_printf("Conditional executed with cond1=%d cond2=%d\n", cond1, cond2);
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
        arr[i] = exclusive_sum - arr[i];
    }
    
    /* Inclusive scan with different operator */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        scan_sum += arr[i] * 2;
        #pragma omp scan inclusive(scan_sum)
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan operations completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2")))
void enter_data_with_to(int **ptr_arr, int n, int seed) {
    /* Dynamic allocation for enter data clause */
    int *device_arr = (int *)malloc(n * sizeof(int));
    if (!device_arr) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        device_arr[i] = i * seed;
    }
    
    /* OMP_CLAUSE_ENTER with 'to' modifier */
    #pragma omp enter data to(device_arr[:n])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: device_arr[:n])
    {
        for (int i = 0; i < n; i++) {
            device_arr[i] *= 2;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(device_arr[:n])
    
    *ptr_arr = device_arr;
    __builtin_printf("Enter data with 'to' completed, first element=%d\n", device_arr[0]);
}

__attribute__((optimize("O2")))
int main(int argc, char *argv[]) {
    /* Seed from command line or use default */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int vseed = seed;
    
    const int N = 512;
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *dynamic_arr = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(seed);
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    /* Main test loop with volatile iteration control */
    for (volatile int iter = 0; iter < g_iter; iter++) {
        int checksum = 0;
        
        /* 1. Complex reductions - should generate _reductemp_ */
        complex_reductions(arr1, arr2, N, vseed + iter);
        
        /* 2. Conditional parallelism - should generate _condtemp_ */
        conditional_parallelism(arr1, N, vseed + iter);
        
        /* 3. Scan operations - should generate _scantemp_ */
        scan_operations(arr2, N, vseed + iter);
        
        /* 4. Enter data with 'to' modifier */
        enter_data_with_to(&dynamic_arr, N/4, vseed + iter);
        
        /* Calculate checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum += arr1[i] + arr2[i];
        }
        if (dynamic_arr) {
            for (int i = 0; i < N/4; i++) {
                checksum += dynamic_arr[i];
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify volatile flags to change behavior */
        g_flag1 = !g_flag1;
        g_flag2 = (iter % 2 == 0);
    }
    
    /* Final output */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += arr1[i] * arr2[i];
    }
    __builtin_printf("Final result: %d\n", final_sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    if (dynamic_arr) free(dynamic_arr);
    
    return 0;
}
