/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses.
 * Compile with: gcc -O2 -fopenmp -fdump-tree-all -std=gnu11 test_omp_clause_printing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization of OpenMP conditions */
volatile int global_seed = 42;
volatile int flag_reduction = 1;
volatile int flag_conditional = 1;
volatile int flag_scan = 1;
volatile int flag_enter_data = 1;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void complex_reductions(int *arr1, int *arr2, int n, volatile int iter) {
    int sum = 0;
    int product = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) shared(n, iter)
    for (int i = 0; i < n; i++) {
        /* Volatile index calculation to prevent optimization */
        volatile int idx = (i + iter + global_seed) % n;
        if (idx < 0) idx = 0;
        
        /* Data-dependent operations that inhibit optimization */
        arr1[idx] = arr1[idx] + (i % 7);
        arr2[i] = arr2[i] * ((i % 5) + 1);
        
        /* Multiple reduction operations */
        sum += arr1[idx] + arr2[i];
        product *= (arr1[idx] % 10) + 1;
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr2[i] < min_val) min_val = arr2[i];
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reductions: sum=%d, product=%d, max=%d, min=%d\n", 
                     sum, product, max_val, min_val);
}

__attribute__((optimize("O2")))
void conditional_parallelism(volatile int cond1, volatile int cond2, int *arr, int n) {
    /* Nested conditional parallel regions */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        volatile int local_cond = cond2 + omp_get_thread_num();
        
        #pragma omp single
        {
            #pragma omp task if(local_cond > 2)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] = arr[i] * 2;
                }
            }
            
            #pragma omp task if(local_cond < 5)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] = arr[i] / 2;
                }
            }
        }
        
        /* Target teams with conditional */
        #pragma omp target teams if(cond1 && cond2) map(tofrom: arr[0:n/4]) thread_limit(4)
        {
            #pragma omp distribute parallel for simd
            for (int i = 0; i < n/4; i++) {
                arr[i] = arr[i] + omp_get_team_num();
            }
        }
    }
    
    __builtin_printf("Conditional parallelism completed\n");
}

__attribute__((optimize("O2")))
void scan_operations(int *arr, int n, volatile int offset) {
    int scan_sum = 0;
    int exclusive_scan = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum) private(exclusive_scan)
    for (int i = 0; i < n; i++) {
        exclusive_scan = scan_sum;
        #pragma omp scan exclusive(exclusive_scan)
        arr[i] = arr[i] + exclusive_scan + offset;
        scan_sum += arr[i] % 100;
    }
    
    /* Inclusive scan with different array */
    int *arr2 = (int*)malloc(n * sizeof(int));
    memcpy(arr2, arr, n * sizeof(int));
    
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        arr2[i] = arr2[i] + scan_sum;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += (arr2[i] % 50) + 1;
    }
    
    __builtin_printf("Scan operations: final sum=%d\n", scan_sum);
    free(arr2);
}

__attribute__((optimize("O2")))
void enter_data_with_to(int size, volatile int use_device) {
    /* Dynamic allocation for enter data clause */
    int *device_array = (int*)malloc(size * sizeof(int));
    int *host_array = (int*)malloc(size * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        host_array[i] = i * global_seed;
        device_array[i] = 0;
    }
    
    /* Use enter data with to clause */
    if (use_device) {
        #pragma omp target enter data map(to: device_array[0:size]) \
                map(to: host_array[0:size/2])
        
        /* Perform computation on device */
        #pragma omp target teams distribute parallel for \
                map(tofrom: device_array[0:size])
        for (int i = 0; i < size; i++) {
            device_array[i] = host_array[i % (size/2)] * 3;
        }
        
        #pragma omp target exit data map(from: device_array[0:size])
    }
    
    /* Another enter data with structured block */
    struct DataBlock {
        int values[100];
        volatile int count;
    };
    
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    block->count = size % 100;
    
    #pragma omp target enter data map(to: block[0:1])
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < 100; i++) {
        block->values[i] = i * block->count;
    }
    #pragma omp target exit data map(from: block[0:1])
    
    __builtin_printf("Enter data completed: device_array[0]=%d, block->values[0]=%d\n",
                     device_array[0], block->values[0]);
    
    free(device_array);
    free(host_array);
    free(block);
}

int main(int argc, char **argv) {
    /* Use argc to seed variability */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    }
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 13 + global_seed) % 1000;
        array2[i] = (i * 17 + global_seed) % 1000;
    }
    
    volatile int iterations = 3;
    int checksum = 0;
    
    /* Multiple iterations to increase compiler processing */
    for (volatile int iter = 0; iter < iterations; iter++) {
        if (flag_reduction) {
            complex_reductions(array1, array2, N, iter);
        }
        
        if (flag_conditional) {
            volatile int cond1 = (iter % 2 == 0);
            volatile int cond2 = (global_seed % 3 == 0);
            conditional_parallelism(cond1, cond2, array1, N);
        }
        
        if (flag_scan) {
            volatile int offset = (iter * 7 + global_seed) % 20;
            scan_operations(array2, N, offset);
        }
        
        if (flag_enter_data && iter == 1) {
            volatile int use_dev = (global_seed % 2);
            enter_data_with_to(N/2, use_dev);
        }
        
        /* Calculate checksum to prevent optimization */
        for (int i = 0; i < N; i++) {
            checksum = (checksum + array1[i] + array2[i]) % 1000000;
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    /* Final output to ensure all code paths matter */
    printf("Final result: %d\n", checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
