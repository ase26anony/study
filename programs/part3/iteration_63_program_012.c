/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_bound = 512;
volatile int v_iter = 2;

/* Function attributes to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int size, int seed) {
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
    for (i = 0; i < size; i++) {
        /* Data-dependent computation to inhibit optimization */
        int idx = (i + seed) % size;
        int val1 = arr1[idx] ^ (i * 3);
        int val2 = arr2[(i * 7) % size] & 0xFF;
        
        sum += val1 + val2;
        prod *= (val1 % 10 + 1);  /* Avoid multiplication by zero */
        if (val1 > max_val) max_val = val1;
        if (val2 < min_val) min_val = val2;
        
        /* Modify arrays to create dependencies */
        arr1[i] = (arr1[i] + val2) % 1000;
        arr2[i] = (arr2[i] ^ val1) % 1000;
    }
    
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n",
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void test_conditional_temporaries(int *arr, int size, volatile int cond) {
    int i;
    
    /* Nested OpenMP with if clauses - should generate _condtemp_ */
    #pragma omp parallel if(cond) num_threads(4)
    {
        #pragma omp master
        {
            /* Task with if clause */
            #pragma omp task if(cond > 0) shared(arr)
            {
                for (i = 0; i < size/2; i++) {
                    arr[i] = arr[i] * 2 + 1;
                }
            }
        }
        
        #pragma omp for
        for (i = size/2; i < size; i++) {
            arr[i] = arr[i] / 2;
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond == 0) map(tofrom:arr[0:size/4]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for
        for (i = 0; i < size/4; i++) {
            arr[i] = arr[i] + i;
        }
    }
    
    __builtin_printf("Conditional test completed with cond=%d\n", cond);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int size) {
    int i;
    int scan_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i)
    for (i = 0; i < size; i++) {
        int val = arr[i] + (i % 7);
        
        #pragma omp scan exclusive(scan_sum)
        {
            arr[i] = scan_sum;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan pattern */
    int prefix_sum = 0;
    #pragma omp parallel for reduction(+:prefix_sum)
    for (i = 0; i < size; i++) {
        prefix_sum += arr[i] % 13;
        arr[i] = prefix_sum;
    }
    
    __builtin_printf("Scan completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(int size) {
    /* Dynamic allocation for enter data with to modifier */
    int *device_arr = (int *)malloc(size * sizeof(int));
    if (!device_arr) return;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        device_arr[i] = i * i % 100;
    }
    
    /* This should trigger OMP_CLAUSE_ENTER with to modifier */
    #pragma omp enter data to(device_arr[0:size])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: device_arr[0:size]) \
            device(0)
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < size; i++) {
            device_arr[i] = device_arr[i] * 3 + 7;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(device_arr[0:size])
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum ^= device_arr[i];
    }
    
    __builtin_printf("Enter data checksum: %d\n", checksum);
    free(device_arr);
}

/* Main test function that combines all patterns */
__attribute__((optimize("O2"), noinline))
void run_omp_tests(int *arr1, int *arr2, int size, int seed) {
    volatile int cond_var = seed % 3;
    
    for (int iter = 0; iter < v_iter; iter++) {
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(arr1, arr2, size, seed + iter);
        
        /* Test 2: Conditional temporaries */
        test_conditional_temporaries(arr1, size, cond_var + iter);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(arr2, size);
        
        /* Test 4: Enter data clause */
        test_enter_data_clause(size / 4);
        
        /* Update seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
}

int main(int argc, char *argv[]) {
    int size = v_bound;
    int seed = 42;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize arrays with pseudo-random values */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < size; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        arr1[i] = seed % 1000;
        arr2[i] = (seed * 3) % 1000;
    }
    
    /* Run tests multiple times with different conditions */
    run_omp_tests(arr1, arr2, size, seed);
    
    /* Final checksum to prevent dead code elimination */
    int final_checksum = 0;
    for (int i = 0; i < size; i++) {
        final_checksum ^= arr1[i];
        final_checksum ^= arr2[i];
    }
    
    __builtin_printf("Final checksum: 0x%08x\n", final_checksum);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
