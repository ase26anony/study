/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_bound = 512;
volatile int v_seed = 42;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators on arrays */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2)
    for (i = 1; i < n; i++) {
        /* Data-dependent operations to inhibit optimization */
        int idx = (i * seed) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero multiplication */
        if (arr1[i] > max_val) max_val = arr1[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Additional complexity with branching */
        if (arr2[i % n] % 3 == 0) {
            sum += arr2[i % n];
        }
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_condtemp_temporaries(int *arr, int n, volatile int cond_flag) {
    int i;
    
    /* OMP parallel with volatile condition - likely generates _condtemp_ */
    #pragma omp parallel if(cond_flag > 0) num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Task with another condition */
        #pragma omp task if(thread_id % 2 == 0)
        {
            for (i = 0; i < n/2; i++) {
                arr[i] += thread_id;
            }
        }
        
        #pragma omp task if(thread_id % 2 == 1)
        {
            for (i = n/2; i < n; i++) {
                arr[i] -= thread_id;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* Target teams with condition */
    #pragma omp target teams if(cond_flag < 0) num_teams(2) thread_limit(128)
    {
        #pragma omp distribute parallel for
        for (i = 0; i < n; i++) {
            arr[i] = arr[i] * 2;
        }
    }
    
    __builtin_printf("Condtemp test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2")))
void test_scantemp_temporaries(int *arr, int n, int seed) {
    int i;
    int scan_sum = 0;
    
    /* Exclusive scan operation */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (i = 0; i < n; i++) {
        int val = arr[i] + (i * seed) % 7;
        
        #pragma omp scan exclusive(scan_sum)
        arr[i] = scan_sum;
        scan_sum += val;
    }
    
    /* Another scan variant */
    int prefix_sum = 0;
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (i = n-1; i >= 0; i--) {
        int val = arr[i] % 13;
        
        #pragma omp scan exclusive(prefix_sum)
        arr[i] += prefix_sum;
        prefix_sum += val;
    }
    
    __builtin_printf("Scantemp test completed, final sum=%d\n", scan_sum + prefix_sum);
}

__attribute__((optimize("O2")))
void test_enter_to_clause(int **ptr_arr, int n) {
    /* Dynamically allocate array for enter data clause */
    int *device_array = (int *)malloc(n * sizeof(int));
    
    if (device_array) {
        /* Initialize array */
        for (int i = 0; i < n; i++) {
            device_array[i] = i * 2;
        }
        
        /* Use enter data with to clause - should trigger OMP_CLAUSE_ENTER with to modifier */
        #pragma omp target enter data map(to: device_array[0:n])
        
        /* Use the device array in a target region */
        #pragma omp target teams distribute parallel for is_device_ptr(device_array)
        for (int i = 0; i < n; i++) {
            device_array[i] += 1;
        }
        
        /* Retrieve data */
        #pragma omp target exit data map(from: device_array[0:n])
        
        __builtin_printf("Enter data test, device_array[10]=%d\n", device_array[10]);
        
        *ptr_arr = device_array;
    }
}

/* Combined test with nested regions */
__attribute__((optimize("O3")))
void combined_nested_test(int *arr1, int *arr2, int n, volatile int iter) {
    /* Outer parallel region */
    #pragma omp parallel num_threads(2)
    {
        /* Inner reduction region */
        #pragma omp for reduction(+:arr1[:n/2]) nowait
        for (int i = 0; i < n/2; i++) {
            arr1[i] += iter * (i % 5);
        }
        
        /* Task with condition */
        #pragma omp task if(omp_get_thread_num() == 0)
        {
            #pragma omp parallel for reduction(*:arr2[:n/2])
            for (int i = n/2; i < n; i++) {
                arr2[i] *= (arr1[i % (n/2)] % 7 + 1);
            }
        }
        
        #pragma omp taskwait
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to seed RNG for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    v_seed = seed;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *device_array = NULL;
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed + 17) % 100;
        array2[i] = (i * seed * 3 + 23) % 100;
    }
    
    volatile int loop_counter = 2;  /* Force multiple iterations */
    
    /* Main test loop - executes OpenMP regions multiple times */
    for (int iter = 0; iter < loop_counter + 1; iter++) {
        v_flag1 = (iter % 2 == 0) ? 1 : -1;
        v_flag2 = iter;
        
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, seed + iter);
        
        /* 2. Test condtemp temporaries */
        test_condtemp_temporaries(array1, N, v_flag1);
        
        /* 3. Test scantemp temporaries */
        test_scantemp_temporaries(array2, N, seed + iter * 7);
        
        /* 4. Test enter data with to clause */
        if (iter == 0) {  /* Only once to avoid multiple allocations */
            test_enter_to_clause(&device_array, N/4);
        }
        
        /* 5. Combined nested test */
        combined_nested_test(array1, array2, N, iter);
        
        /* Calculate and print checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += array1[i] + array2[i];
        }
        if (device_array) {
            for (int i = 0; i < N/4; i++) {
                checksum += device_array[i];
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output */
    __builtin_printf("\nFinal values: array1[0]=%d, array2[0]=%d\n", 
                     array1[0], array2[0]);
    
    /* Cleanup */
    free(array1);
    free(array2);
    if (device_array) {
        free(device_array);
    }
    
    return 0;
}
