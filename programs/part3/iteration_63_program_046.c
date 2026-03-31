/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int vol_flag1 = 1;
volatile int vol_flag2 = 0;
volatile int vol_bound = 512;
volatile int vol_seed = 42;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    int i;
    volatile int vol_idx;
    
    /* Complex reduction operations that may generate _reductemp_ */
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Use volatile index to prevent loop optimizations */
    vol_idx = seed % n;
    
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
             reduction(max:max_val) reduction(min:min_val) \
             private(i) shared(arr1, arr2)
    for (i = 0; i < n; i++) {
        /* Data-dependent operations to inhibit optimization */
        int idx = (i + vol_idx) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);
        
        if (arr1[i] > max_val) max_val = arr1[i];
        if (arr2[i % n] < min_val) min_val = arr2[i % n];
        
        /* Cross-update arrays to create dependencies */
        if (i % 3 == 0) {
            arr1[(i + 1) % n] += arr2[i % n] % 7;
        }
    }
    
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void test_conditional_temporaries(int *arr, int n, volatile int cond) {
    int i;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(cond) num_threads(4)
    {
        volatile int local_cond = cond;
        
        #pragma omp single
        {
            __builtin_printf("Parallel region entered, cond=%d\n", local_cond);
        }
        
        #pragma omp for
        for (i = 0; i < n; i++) {
            arr[i] = arr[i] * 2 + (i % 3);
        }
        
        /* Task with if clause */
        #pragma omp task if(local_cond > 0) shared(arr)
        {
            arr[0] += 1000;
        }
        
        #pragma omp taskwait
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond > 0) map(tofrom:arr[0:n]) thread_limit(4)
    {
        #pragma omp distribute parallel for
        for (i = 0; i < n; i++) {
            arr[i] = arr[i] / 2;
        }
    }
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int n) {
    int i;
    int scan_sum = 0;
    volatile int vol_offset = 5;
    
    /* Exclusive scan - may generate _scantemp_ */
    #pragma omp parallel for private(i) reduction(inscan, +:scan_sum)
    for (i = 0; i < n; i++) {
        int val = arr[i] + (i % vol_offset);
        
        #pragma omp scan exclusive(scan_sum)
        {
            int temp = scan_sum;
            arr[i] = temp + val;
            scan_sum += val;
        }
    }
    
    __builtin_printf("Scan sum: %d\n", scan_sum);
    
    /* Inclusive scan pattern */
    int prefix_sum = 0;
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (i = 0; i < n; i++) {
        prefix_sum += arr[i] % 13;
        
        #pragma omp scan inclusive(prefix_sum)
        arr[i] = prefix_sum;
    }
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(int **ptr_arr, int n) {
    /* Allocate and initialize array for enter data */
    *ptr_arr = (int *)malloc(n * sizeof(int));
    if (!*ptr_arr) return;
    
    for (int i = 0; i < n; i++) {
        (*ptr_arr)[i] = i * i + 7;
    }
    
    /* OMP enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: (*ptr_arr)[0:n])
    
    /* Use the data on target */
    #pragma omp target teams distribute parallel for map(always, tofrom: (*ptr_arr)[0:n])
    for (int i = 0; i < n; i++) {
        (*ptr_arr)[i] += i % 11;
    }
    
    #pragma omp target exit data map(from: (*ptr_arr)[0:n])
    
    __builtin_printf("Enter data test completed, first element: %d\n", (*ptr_arr)[0]);
}

__attribute__((optimize("O2"), noinline))
int compute_checksum(int *arr1, int *arr2, int n) {
    int checksum = 0;
    volatile int vol_mod = 97; /* Prime to prevent optimization */
    
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < n; i++) {
        checksum = (checksum + arr1[i] + arr2[i % n]) % vol_mod;
    }
    
    return checksum;
}

int main(int argc, char **argv) {
    const int N = 512;
    int *array1, *array2;
    int *dynamic_array = NULL;
    int i, iter;
    
    /* Initialize pseudo-random seed from argv */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed <= 0) seed = 42;
    }
    vol_seed = seed;
    
    /* Allocate and initialize arrays */
    array1 = (int *)malloc(N * sizeof(int));
    array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    srand(seed);
    for (i = 0; i < N; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 500 + 1;
    }
    
    /* Volatile iteration counter */
    volatile int vol_iterations = 2;
    if (argc > 2) vol_iterations = atoi(argv[2]) % 5;
    
    /* Main test loop - multiple iterations to increase coverage */
    for (iter = 0; iter < vol_iterations; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        vol_flag1 = (iter % 2 == 0) ? 1 : 0;
        test_reduction_temporaries(array1, array2, N, seed + iter);
        
        int checksum1 = compute_checksum(array1, array2, N);
        __builtin_printf("Checksum after reduction: %d\n", checksum1);
        
        /* 2. Test conditional temporaries */
        vol_flag2 = (iter % 3 == 0) ? 1 : 0;
        test_conditional_temporaries(array1, N, vol_flag2);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N);
        
        int checksum2 = compute_checksum(array1, array2, N);
        __builtin_printf("Checksum after scan: %d\n", checksum2);
        
        /* 4. Test enter data clause with to modifier */
        test_enter_data_clause(&dynamic_array, N / 2);
        
        if (dynamic_array) {
            int sum = 0;
            for (i = 0; i < N / 2; i++) {
                sum += dynamic_array[i];
            }
            __builtin_printf("Dynamic array sum: %d\n", sum);
            free(dynamic_array);
            dynamic_array = NULL;
        }
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final checksum output */
    int final_checksum = compute_checksum(array1, array2, N);
    __builtin_printf("\nFinal checksum: %d\n", final_checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
