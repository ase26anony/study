/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_ and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Force optimization level to ensure tree transformations */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access
     * Forces generation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access prevents optimization */
        int idx = (i * 17 + flag) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);
        if (arr2[idx] > max_val) max_val = arr2[idx];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-thread data dependency simulation */
        if (i > 0 && arr1[i-1] % 3 == 0) {
            arr2[i] += arr1[i] % 7;
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, volatile int cond1, volatile int cond2) {
    /* Forces generation of _condtemp_ temporaries through multiple if clauses */
    
    /* Parallel region with runtime-dependent condition */
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with another condition - creates more _condtemp_ opportunities */
        #pragma omp task if(cond2 > 0 && tid % 2 == 0) firstprivate(arr)
        {
            for (int i = tid * (n/4); i < (tid + 1) * (n/4) && i < n; i++) {
                arr[i] = arr[i] * 2 + tid;
            }
        }
        
        #pragma omp taskwait
        
        /* Nested parallel region with condition */
        #pragma omp parallel if(cond1 + cond2 > 5) num_threads(2)
        {
            #pragma omp for
            for (int i = 0; i < n/2; i++) {
                arr[i] += omp_get_thread_num();
            }
        }
    }
    
    /* Target teams with condition - different clause context */
    #pragma omp target teams if(cond1 < cond2) num_teams(2) thread_limit(32) \
            map(tofrom:arr[0:n/2])
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n/2; i++) {
            arr[i] = arr[i] % 256;
        }
    }
    
    __builtin_printf("Conditional test completed with cond1=%d, cond2=%d\n", cond1, cond2);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int scan_type) {
    int partial_sum = 0;
    int exclusive_prefix = 0;
    
    /* Forces generation of _scantemp_ temporaries */
    
    /* Exclusive scan pattern */
    #pragma omp parallel for reduction(inscan, +:partial_sum) \
            private(exclusive_prefix) if(scan_type == 0)
    for (int i = 0; i < n; i++) {
        exclusive_prefix = partial_sum;
        #pragma omp scan exclusive(partial_sum)
        arr[i] += exclusive_prefix;
        partial_sum += i + arr[i] % 5;
    }
    
    /* Inclusive scan pattern */
    int temp_arr[512];
    memcpy(temp_arr, arr, n * sizeof(int));
    
    #pragma omp parallel for reduction(inscan, +:partial_sum) if(scan_type == 1)
    for (int i = 0; i < n; i++) {
        partial_sum += temp_arr[i];
        #pragma omp scan inclusive(partial_sum)
        arr[i] = partial_sum % 1000;
    }
    
    /* Combined parallel for scan */
    #pragma omp parallel for scan(+:partial_sum) if(scan_type == 2)
    for (int i = n-1; i >= 0; i--) {
        arr[i] = (arr[i] + partial_sum) % 255;
        partial_sum += arr[i];
    }
    
    __builtin_printf("Scan test type %d completed, final sum=%d\n", scan_type, partial_sum);
}

__attribute__((optimize("O2")))
void test_enter_data_clause(int n, volatile int use_device) {
    /* Forces generation of OMP_CLAUSE_ENTER with 'to' modifier */
    
    int *device_array = (int *)malloc(n * sizeof(int));
    int *host_array = (int *)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        host_array[i] = i * 3 + 1;
        device_array[i] = 0;
    }
    
    /* Use enter data with to clause - triggers OMP_CLAUSE_ENTER with to modifier */
    if (use_device) {
        #pragma omp target enter data map(to: device_array[0:n]) \
                map(to: host_array[0:n/2])
        
        #pragma omp target teams distribute parallel for \
                map(tofrom: device_array[0:n])
        for (int i = 0; i < n; i++) {
            device_array[i] = host_array[i % (n/2)] * 2;
        }
        
        #pragma omp target exit data map(from: device_array[0:n])
    }
    
    /* Another enter data variant */
    struct DataBlock {
        int values[256];
        volatile int count;
    };
    
    struct DataBlock *block = (struct DataBlock *)malloc(sizeof(struct DataBlock));
    block->count = n;
    
    #pragma omp target enter data map(to: block->values[0:256]) \
            map(to: block[0:1])
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: block->values[0:256])
    for (int i = 0; i < 256 && i < n; i++) {
        block->values[i] = i * i % 100;
    }
    
    #pragma omp target exit data map(from: block->values[0:256])
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < n && i < 256; i++) {
        checksum += block->values[i];
    }
    
    __builtin_printf("Enter data test: checksum=%d, use_device=%d\n", checksum, use_device);
    
    free(device_array);
    free(host_array);
    free(block);
}

/* Main test driver with volatile controls */
__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int iter_count = (seed % 3) + 2;  /* 2-4 iterations */
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 13 + seed) % 1000;
        array2[i] = (i * 29 + seed * 7) % 1000;
    }
    
    volatile int flag_reduction = seed % 2;
    volatile int flag_cond1 = seed % 3;
    volatile int flag_cond2 = (seed * 2) % 5;
    volatile int flag_scan = seed % 3;
    volatile int flag_enter = seed % 2;
    
    int total_checksum = 0;
    
    /* Multiple iterations to increase compiler processing */
    for (volatile int iter = 0; iter < iter_count; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, flag_reduction + iter);
        
        /* 2. Test conditional temporaries */
        test_conditional_temporaries(array1, N, flag_cond1 + iter, flag_cond2);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, flag_scan);
        
        /* 4. Test enter data clause */
        test_enter_data_clause(N / 2, flag_enter ^ (iter % 2));
        
        /* Update checksum */
        for (int i = 0; i < N; i++) {
            total_checksum += array1[i] + array2[i];
            total_checksum %= 1000000;
        }
        
        /* Modify flags for next iteration */
        flag_reduction ^= 1;
        flag_cond1 = (flag_cond1 * 3 + 1) % 7;
    }
    
    /* Final output to prevent optimization */
    __builtin_printf("\nFinal checksum: %d\n", total_checksum);
    __builtin_printf("Seed used: %d, Iterations: %d\n", seed, iter_count);
    
    free(array1);
    free(array2);
    
    return total_checksum != 0 ? 0 : 1;
}
