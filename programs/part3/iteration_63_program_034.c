/* Test program to trigger uncovered OpenMP clause printing in GCC */
/* Compile with: gcc -O2 -fopenmp -fdump-tree-all -std=gnu11 -o test test.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
static void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
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
        int idx = (i * 17 + flag) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);
        if (arr2[idx] > max_val) max_val = arr2[idx];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-update to force temporary creation */
        arr1[i] = (arr1[i] + arr2[idx]) % 100;
    }
    
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2"), noinline))
static void test_condition_temporaries(int *arr, int n, volatile int cond_flag) {
    volatile int runtime_cond = cond_flag;
    
    /* OMP parallel with if clause using volatile condition */
    #pragma omp parallel if(runtime_cond > 0) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with if clause */
        #pragma omp task if(tid % 2 == 0 && runtime_cond > 0)
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] = (arr[i] * 3 + 7) % 100;
            }
        }
        
        #pragma omp taskwait
        
        /* Nested parallel region with if clause */
        #pragma omp parallel if(runtime_cond < 10) num_threads(2)
        {
            #pragma omp for
            for (int i = 0; i < n/2; i++) {
                arr[i] += tid * 17;
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(runtime_cond != 0) map(tofrom:arr[0:n/4]) thread_limit(4)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n/4; i++) {
            arr[i] = (arr[i] + i) % 256;
        }
    }
    
    __builtin_printf("Conditional execution completed with flag=%d\n", runtime_cond);
}

__attribute__((optimize("O2"), noinline))
static void test_scan_temporaries(int *arr, int n, volatile int scan_type) {
    int partial_sum = 0;
    int exclusive_prefix = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:partial_sum) \
            if(scan_type == 0)
    for (int i = 0; i < n; i++) {
        int val = arr[i] % 50;
        
        #pragma omp scan exclusive(partial_sum)
        {
            arr[i] = exclusive_prefix + val;
            exclusive_prefix = partial_sum;
        }
        partial_sum += val;
    }
    
    /* Inclusive scan with different pattern */
    partial_sum = 0;
    #pragma omp parallel for reduction(inscan, +:partial_sum) \
            if(scan_type == 1)
    for (int i = 0; i < n; i++) {
        int val = (arr[i] + i) % 30;
        partial_sum += val;
        
        #pragma omp scan inclusive(partial_sum)
        {
            arr[i] = partial_sum;
        }
    }
    
    __builtin_printf("Scan operations completed, type=%d\n", scan_type);
}

__attribute__((optimize("O2"), noinline))
static void test_enter_data_clause(int **dyn_arr, int n, volatile int use_gpu) {
    /* Dynamically allocate array for enter data testing */
    *dyn_arr = (int*)malloc(n * sizeof(int));
    if (!*dyn_arr) return;
    
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = (i * 13 + 7) % 100;
    }
    
    /* Use enter data with to clause */
    if (use_gpu) {
        #pragma omp target enter data map(to: (*dyn_arr)[0:n/2])
        
        /* Perform computation on device */
        #pragma omp target teams distribute parallel for map(tofrom: (*dyn_arr)[0:n/2])
        for (int i = 0; i < n/2; i++) {
            (*dyn_arr)[i] = (*dyn_arr)[i] * 2 + 1;
        }
        
        #pragma omp target exit data map(from: (*dyn_arr)[0:n/2])
    } else {
        /* Alternative: enter data with to clause for host fallback */
        #pragma omp target enter data map(to: (*dyn_arr)[0:n/4]) if(use_gpu == 0)
    }
    
    __builtin_printf("Enter data completed, array[0]=%d\n", (*dyn_arr)[0]);
}

__attribute__((optimize("O3"), noinline))
static void nested_combined_constructs(int *arr1, int *arr2, int n, volatile int iter) {
    /* Combined target teams distribute parallel for with reduction */
    #pragma omp target teams distribute parallel for reduction(+:arr1[0:n]) \
            map(tofrom: arr1[0:n], arr2[0:n]) if(iter > 1) thread_limit(8)
    for (int i = 0; i < n; i++) {
        arr1[i] = (arr1[i] + arr2[i] + iter) % 1000;
        arr2[i] = (arr2[i] * 3 - iter) % 500;
    }
    
    /* Nested parallel regions with task reduction */
    #pragma omp parallel num_threads(4) if(iter < 5)
    {
        #pragma omp single
        {
            for (int i = 0; i < 4; i++) {
                #pragma omp task shared(arr1) if(i % 2 == 0)
                {
                    #pragma omp parallel for reduction(+:arr1[0:n/2])
                    for (int j = 0; j < n/2; j++) {
                        arr1[j] += (j + i) % 17;
                    }
                }
            }
        }
    }
    
    __builtin_printf("Nested constructs iteration %d completed\n", iter);
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int flag1 = seed % 3;
    volatile int flag2 = (seed * 17) % 5;
    volatile int flag3 = (seed + 7) % 2;
    volatile int use_gpu_flag = (seed % 7) > 3;
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *dyn_array = NULL;
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 13 + seed) % 100;
        array2[i] = (i * 29 + seed * 3) % 100;
    }
    
    /* Multiple iterations to increase compiler exposure */
    volatile int iterations = 3;
    for (int iter = 0; iter < iterations; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Trigger _reductemp_ clause printing */
        test_reduction_temporaries(array1, array2, N, flag1 + iter);
        
        /* 2. Trigger _condtemp_ clause printing */
        test_condition_temporaries(array1, N, flag2 + iter);
        
        /* 3. Trigger _scantemp_ clause printing */
        test_scan_temporaries(array2, N, flag3);
        
        /* 4. Trigger OMP_CLAUSE_ENTER with to modifier */
        test_enter_data_clause(&dyn_array, N, use_gpu_flag);
        
        /* 5. Combined and nested constructs */
        nested_combined_constructs(array1, array2, N, iter);
        
        /* Calculate checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum + array1[i] * 3 + array2[i]) % 1000000;
            if (dyn_array && i < N/2) {
                checksum = (checksum + dyn_array[i]) % 1000000;
            }
        }
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify flags for next iteration */
        flag1 = (flag1 * 3 + 7) % 11;
        flag2 = (flag2 * 5 + 13) % 7;
    }
    
    /* Final output to ensure all code paths are considered */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    if (dyn_array) {
        for (int i = 0; i < N/2; i++) {
            final_sum += dyn_array[i];
        }
        free(dyn_array);
    }
    
    __builtin_printf("\nFinal sum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
