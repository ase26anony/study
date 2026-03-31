/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_bound = 512;
volatile int v_seed = 42;

/* Function with complex reduction operations to generate _reductemp_ */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int iter) {
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1) shared(arr2)
    for (int i = 0; i < n; i++) {
        /* Data-dependent index calculation prevents optimization */
        int idx = (i + v_seed + iter) % n;
        sum += arr1[idx] + arr2[i % n];
        
        /* Avoid multiplication by zero for product reduction */
        if (arr1[idx] != 0 && arr2[i % n] != 0) {
            prod *= (arr1[idx] % 10 + 1) * (arr2[i % n] % 10 + 1);
        }
        
        /* Conditional updates for max/min */
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr2[i % n] < min_val && arr2[i % n] > 0) min_val = arr2[i % n];
        
        /* Modify arrays to create dependencies */
        arr1[idx] = (arr1[idx] + arr2[i % n]) % 1000;
    }
    
    __builtin_printf("Reduction iter %d: sum=%d, prod=%d, max=%d, min=%d\n", 
                     iter, sum, prod, max_val, min_val);
}

/* Function to generate _condtemp_ clauses */
__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, int iter) {
    volatile int cond1 = v_flag1;
    volatile int cond2 = v_flag2;
    
    /* Outer parallel with if clause */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with if clause */
        #pragma omp task if(cond2 || tid % 2 == 0)
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] = (arr[i] * 3 + 7) % 1000;
            }
        }
        
        #pragma omp taskwait
        
        /* Teams with if clause (for target offloading) */
        #pragma omp target teams if(cond1 && iter > 0) num_teams(2) thread_limit(64)
        {
            #pragma omp distribute parallel for
            for (int i = 0; i < (n > 100 ? 100 : n); i++) {
                if (i < n) arr[i] = (arr[i] + tid) % 1000;
            }
        }
    }
    
    __builtin_printf("Conditional temporaries iter %d complete\n", iter);
}

/* Function to generate _scantemp_ clauses (OpenMP 5.0+ scan) */
__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, int iter) {
    int scan_sum = 0;
    int exclusive_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:exclusive_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % 10);
        
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] = exclusive_sum;
        exclusive_sum += val;
    }
    
    /* Inclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        int val = (arr[i] * 2 + 1) % 100;
        
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan temporaries iter %d: final_scan=%d\n", iter, scan_sum);
}

/* Function to test OMP_CLAUSE_ENTER with 'to' modifier */
__attribute__((optimize("O2")))
void test_enter_clause(int **dyn_arr, int n, int iter) {
    /* Allocate and initialize dynamic array */
    *dyn_arr = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = (i * 3 + iter * 7) % 1000;
    }
    
    /* Use enter data with to clause */
    #pragma omp target enter data map(to: (*dyn_arr)[0:n])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = (*dyn_arr)[i] * 2 + 1;
    }
    
    /* Retrieve data */
    #pragma omp target exit data map(from: (*dyn_arr)[0:n])
    
    __builtin_printf("Enter clause iter %d: first=%d, last=%d\n", 
                     iter, (*dyn_arr)[0], (*dyn_arr)[n-1]);
}

/* Main function with runtime variability */
int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    v_seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    const int N = v_bound;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *dyn_array = NULL;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17 + v_seed) % 1000;
        array2[i] = (i * 23 + v_seed * 2) % 1000;
    }
    
    volatile int repeat_count = (v_seed % 3) + 2; /* 2-4 iterations */
    
    for (int iter = 0; iter < repeat_count; iter++) {
        /* Toggle volatile flags for condition variability */
        v_flag1 = (iter % 2 == 0);
        v_flag2 = (iter % 3 == 0);
        
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(array1, array2, N, iter);
        
        /* Test 2: Conditional temporaries */
        test_conditional_temporaries(array1, N, iter);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(array2, N, iter);
        
        /* Test 4: Enter clause with 'to' modifier */
        test_enter_clause(&dyn_array, N/2, iter);
        
        /* Calculate checksum to prevent dead code elimination */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum + array1[i] + array2[i]) % 1000000;
        }
        if (dyn_array) {
            for (int i = 0; i < N/2; i++) {
                checksum = (checksum + dyn_array[i]) % 1000000;
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Free dynamic array if allocated */
        if (dyn_array) {
            free(dyn_array);
            dyn_array = NULL;
        }
    }
    
    /* Final checksum output */
    int final_checksum = 0;
    for (int i = 0; i < N; i++) {
        final_checksum = (final_checksum * 31 + array1[i]) % 1000000;
        final_checksum = (final_checksum * 31 + array2[i]) % 1000000;
    }
    __builtin_printf("\nFinal checksum: %d\n", final_checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
