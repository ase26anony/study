/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
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
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators on arrays
     * This should generate _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2)
    for (i = 0; i < n; i++) {
        /* Data-dependent computation to inhibit optimization */
        int idx = (i + seed) % n;
        int val1 = arr1[idx] ^ (i * 3);
        int val2 = arr2[(i * 7) % n] & 0xFF;
        
        sum += val1 + val2;
        prod *= (val1 % 10 + 1) * (val2 % 5 + 1);
        
        if (val1 > max_val) max_val = val1;
        if (val2 < min_val && val2 > 0) min_val = val2;
        
        /* Modify arrays to create dependencies */
        arr1[idx] = (arr1[idx] + i) % 1000;
        arr2[(i * 7) % n] = (arr2[(i * 7) % n] - i) & 0x3FF;
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void test_conditional_temporaries(int *arr, int n, int seed) {
    volatile int cond1 = (seed % 3) == 0;
    volatile int cond2 = (seed % 5) == 0;
    
    /* Nested parallel regions with if clauses
     * Should generate _condtemp_ temporaries */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond2)
            {
                #pragma omp parallel for
                for (int i = 0; i < n/2; i++) {
                    arr[i] = arr[i] * 2 + 1;
                }
            }
            
            #pragma omp task if(!cond2)
            {
                #pragma omp parallel for
                for (int i = n/2; i < n; i++) {
                    arr[i] = arr[i] / 2 - 1;
                }
            }
        }
        
        /* Another if clause in target teams context */
        #pragma omp target teams if(vol_flag1) map(tofrom:arr[0:n/4]) thread_limit(4)
        {
            #pragma omp distribute parallel for simd
            for (int i = 0; i < n/4; i++) {
                arr[i] = arr[i] ^ 0xAA;
            }
        }
    }
    
    __builtin_printf("Conditional test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int n, int seed) {
    int scan_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i * seed) % 7;
        
        #pragma omp scan exclusive(scan_sum)
        {
            arr[i] = scan_sum;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan pattern */
    int prefix_sum = 0;
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        prefix_sum += arr[i] % 13;
        arr[i] = prefix_sum;
    }
    
    __builtin_printf("Scan results: final_sum=%d, prefix[%d]=%d\n", 
                     scan_sum, n-1, arr[n-1]);
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(int **ptr_arr, int n) {
    /* Dynamically allocate memory for enter data clause */
    int *device_data = (int*)malloc(n * sizeof(int));
    if (!device_data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        device_data[i] = i * i + 1;
    }
    
    /* Use enter data with to modifier - should trigger OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_data[0:n])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for map(tofrom: device_data[0:n])
    for (int i = 0; i < n; i++) {
        device_data[i] = device_data[i] * 3 + 7;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_data[0:n])
    
    *ptr_arr = device_data;
    __builtin_printf("Enter data test: device_data[0]=%d\n", device_data[0]);
}

/* Main test function that combines all patterns */
__attribute__((optimize("O2")))
void run_omp_tests(int argc, char **argv) {
    int seed = vol_seed;
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed == 0) seed = 42;
    }
    
    const int N = vol_bound;
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *scan_arr = (int*)malloc(N * sizeof(int));
    int *enter_data_result = NULL;
    
    if (!arr1 || !arr2 || !scan_arr) {
        free(arr1); free(arr2); free(scan_arr);
        return;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * 13 + seed) % 1000;
        arr2[i] = (i * 17 + seed * 2) % 1000;
        scan_arr[i] = (i * 19 + seed * 3) % 500;
    }
    
    volatile int repeat_count = 2;
    volatile int checksum = 0;
    
    /* Repeat tests to increase chance of clause generation */
    for (volatile int iter = 0; iter < repeat_count; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter + 1);
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(arr1, arr2, N, seed + iter);
        
        /* Test 2: Conditional temporaries */
        test_conditional_temporaries(arr1, N, seed + iter * 3);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(scan_arr, N, seed + iter * 5);
        
        /* Test 4: Enter data with to modifier */
        if (iter == 0) {  /* Only do this once to avoid memory leaks */
            test_enter_data_clause(&enter_data_result, N/2);
        }
        
        /* Calculate checksum to prevent elimination */
        for (int i = 0; i < N; i += 8) {
            checksum ^= arr1[i] ^ arr2[i] ^ scan_arr[i];
        }
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Final output to prevent optimization */
    __builtin_printf("\nFinal checksum: %d\n", checksum);
    if (enter_data_result) {
        __builtin_printf("Enter data sample: %d\n", enter_data_result[0]);
        free(enter_data_result);
    }
    
    free(arr1);
    free(arr2);
    free(scan_arr);
}

int main(int argc, char **argv) {
    /* Force tree dumping by using optimization + OpenMP */
    run_omp_tests(argc, argv);
    
    return 0;
}
