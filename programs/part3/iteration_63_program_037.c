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
volatile int v_cond = 1;
volatile int v_iter = 2;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators on arrays
     * This should generate _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) shared(n, seed)
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation to inhibit optimization */
        int idx = (i * seed) % n;
        if (idx < 0) idx = -idx;
        
        /* Multiple reduction operations */
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);
        
        if (arr1[i] > max_val) max_val = arr1[i];
        if (arr2[i] < min_val) min_val = arr2[i];
        
        /* Cross-update to force temporary creation */
        arr1[i] = arr2[(i + seed) % n] + i;
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, volatile int cond) {
    /* OMP parallel with volatile condition - should generate _condtemp_ */
    #pragma omp parallel if(cond) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with another condition */
        #pragma omp task if(tid % 2 == 0 && v_cond)
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] += tid;
            }
        }
        
        #pragma omp taskwait
        
        /* Teams with condition */
        #pragma omp target teams if(cond && v_flag1) thread_limit(8)
        {
            #pragma omp distribute parallel for simd
            for (int i = 0; i < n; i++) {
                arr[i] *= 2;
            }
        }
    }
    
    __builtin_printf("Conditional test completed, cond=%d\n", cond);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, int seed) {
    int scan_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(arr) shared(n, seed)
    for (int i = 0; i < n; i++) {
        /* Exclusive scan computation */
        int val = arr[i] + (i * seed) % 7;
        
        #pragma omp scan exclusive(scan_sum)
        arr[i] = scan_sum;
        scan_sum += val;
    }
    
    /* Another scan variant */
    int prefix_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        int temp = arr[i] * 3 - 1;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += temp;
        arr[i] = prefix_sum;
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2")))
void test_enter_data_with_to(int **ptr_arr, int n) {
    /* Dynamically allocate array for enter data clause */
    int *device_arr = (int *)malloc(n * sizeof(int));
    if (!device_arr) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        device_arr[i] = i * 3 + 1;
    }
    
    /* Use enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_arr[0:n]) \
            depend(inout: device_arr) nowait
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for simd \
            is_device_ptr(device_arr)
    for (int i = 0; i < n; i++) {
        device_arr[i] = device_arr[i] * 2 + 1;
    }
    
    /* Retrieve data */
    #pragma omp target exit data map(from: device_arr[0:n])
    
    /* Store result */
    *ptr_arr = device_arr;
    
    __builtin_printf("Enter data test completed, device_arr[0]=%d\n", device_arr[0]);
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Seed from command line for runtime variability */
    int seed = 12345;
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed == 0) seed = 12345;
    }
    
    const int N = 512;
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *result_arr = NULL;
    
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
    
    int checksum = 0;
    
    /* Multiple iterations based on volatile counter */
    for (volatile int iter = 0; iter < v_iter; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        if (v_flag1 || iter % 2 == 0) {
            test_reduction_temporaries(arr1, arr2, N, seed + iter);
        }
        
        /* 2. Test conditional temporaries */
        v_cond = iter % 3;
        test_conditional_temporaries(arr1, N, v_cond);
        
        /* 3. Test scan temporaries */
        if (v_flag2 || iter > 0) {
            test_scan_temporaries(arr2, N, seed - iter);
        }
        
        /* 4. Test enter data with to modifier */
        if (iter == v_iter - 1) {  /* Only on last iteration to avoid leaks */
            test_enter_data_with_to(&result_arr, N / 2);
        }
        
        /* Update checksum to prevent optimization */
        for (int i = 0; i < N; i += 8) {
            checksum += arr1[i] + arr2[i];
        }
        
        __builtin_printf("Checksum after iteration %d: %d\n", iter, checksum);
    }
    
    /* Final output to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += arr1[i] + arr2[i];
    }
    if (result_arr) {
        for (int i = 0; i < N / 2; i++) {
            final_sum += result_arr[i];
        }
        free(result_arr);
    }
    
    __builtin_printf("\nFinal sum: %d, Final checksum: %d\n", final_sum, checksum);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
