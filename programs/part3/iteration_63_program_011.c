/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier.
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
__attribute__((optimize("O2")))
void omp_reduction_with_temps(int *arr1, int *arr2, int n, int seed) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction operations that may generate _reductemp_ */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2) schedule(dynamic)
    for (i = 0; i < n; i++) {
        /* Data-dependent indexing to prevent optimization */
        int idx = (i + seed) % n;
        int idx2 = (i * seed) % n;
        
        /* Complex reduction expressions */
        sum += arr1[idx] + arr2[idx2];
        prod *= (arr1[idx] % 10 + 1);  /* Avoid zero product */
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr2[idx2] < min_val) min_val = arr2[idx2];
        
        /* Cross-update to create dependencies */
        arr1[idx] = (arr1[idx] + arr2[idx2]) % 1000;
        arr2[idx2] = (arr2[idx2] - arr1[idx]) % 1000;
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void omp_conditional_temps(int *arr, int n, volatile int cond) {
    int i;
    
    /* OMP_CLAUSE__CONDTEMP_ generation with volatile condition */
    #pragma omp parallel if(cond > 0) num_threads(4)
    {
        #pragma omp single
        {
            /* Nested task with condition */
            #pragma omp task if(cond < 10) shared(arr)
            {
                for (i = 0; i < n/2; i++) {
                    arr[i] = arr[i] * 2;
                }
            }
            
            #pragma omp task if(cond > 5) shared(arr)
            {
                for (i = n/2; i < n; i++) {
                    arr[i] = arr[i] / 2;
                }
            }
            
            #pragma omp taskwait
        }
        
        /* Teams with condition */
        #pragma omp target teams if(cond % 2 == 0) num_teams(2) thread_limit(8)
        {
            #pragma omp distribute parallel for simd
            for (i = 0; i < n; i++) {
                arr[i] = arr[i] + (i % 100);
            }
        }
    }
    
    __builtin_printf("Conditional temps processed, cond=%d\n", cond);
}

__attribute__((optimize("O2")))
void omp_scan_temps(int *arr, int n) {
    int i;
    int scan_sum = 0;
    
    /* Exclusive scan - may generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i) shared(arr)
    for (i = 0; i < n; i++) {
        /* Exclusive scan phase */
        #pragma omp scan exclusive(scan_sum)
        {
            int temp = arr[i];
            arr[i] = scan_sum;
            scan_sum += temp;
        }
    }
    
    /* Inclusive scan */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i) shared(arr)
    for (i = 0; i < n; i++) {
        scan_sum += arr[i];
        #pragma omp scan inclusive(scan_sum)
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2")))
void omp_enter_data_with_to(int **dyn_arr, int n) {
    /* Dynamic allocation for enter data clause */
    *dyn_arr = (int *)malloc(n * sizeof(int));
    if (!*dyn_arr) return;
    
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = i * i;
    }
    
    /* OMP_CLAUSE_ENTER with to modifier */
    #pragma omp enter data to(*dyn_arr[:n])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: (*dyn_arr)[:n])
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < n; i++) {
            (*dyn_arr)[i] += i;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(*dyn_arr[:n])
    
    __builtin_printf("Enter data with to completed\n");
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    const int N = 512;
    int *array1, *array2;
    int *dyn_array = NULL;
    int i, iter;
    
    /* Use argv for runtime variability */
    if (argc > 1) {
        v_seed = atoi(argv[1]);
    }
    
    /* Initialize with pseudo-random values */
    array1 = (int *)malloc(N * sizeof(int));
    array2 = (int *)malloc(N * sizeof(int));
    
    srand(v_seed);
    for (i = 0; i < N; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    /* Multiple iterations to increase coverage chances */
    for (iter = 0; iter < 3; iter++) {
        v_flag1 = (iter % 2 == 0);
        v_flag2 = (iter % 3 == 0);
        
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Trigger _reductemp_ generation */
        omp_reduction_with_temps(array1, array2, N, v_seed + iter);
        
        /* 2. Trigger _condtemp_ generation */
        omp_conditional_temps(array1, N, v_flag1 ? 7 : 3);
        
        /* 3. Trigger _scantemp_ generation */
        omp_scan_temps(array2, N);
        
        /* 4. Trigger OMP_CLAUSE_ENTER with to modifier */
        omp_enter_data_with_to(&dyn_array, 100);
        
        /* Compute checksum to prevent optimization */
        int checksum = 0;
        #pragma omp parallel for reduction(+:checksum)
        for (i = 0; i < N; i++) {
            checksum += array1[i] + array2[i];
        }
        if (dyn_array) {
            for (i = 0; i < 100; i++) {
                checksum += dyn_array[i];
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify seed for next iteration */
        v_seed = (v_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output */
    int final_sum = 0;
    for (i = 0; i < N; i++) {
        final_sum += array1[i] - array2[i];
    }
    __builtin_printf("\nFinal difference sum: %d\n", final_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    if (dyn_array) free(dyn_array);
    
    return 0;
}
