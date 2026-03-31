/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization */
volatile int g_flag = 1;
volatile int g_seed = 42;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i * g_seed) % n;
        sum += arr1[idx] + (arr2[i] % 10);
        
        /* Avoid multiplication by zero for product reduction */
        if (arr1[idx] != 0) {
            prod *= (arr1[idx] % 7 + 1);
        }
        
        /* Conditional updates for max/min */
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Modify arr2 to create side effects */
        arr2[i] = (arr2[i] + i) % 100;
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3")))
void test_condition_temporaries(int *arr, int n, volatile int cond_flag) {
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond_flag > 0) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond_flag > 1) shared(arr)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] += g_seed;
                }
            }
            
            #pragma omp task if(cond_flag > 2) shared(arr)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] -= g_seed;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond_flag > 3) map(tofrom:arr[0:n]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 2;
        }
    }
    
    __builtin_printf("Conditional execution completed with flag=%d\n", cond_flag);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int scan_flag) {
    int partial_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:partial_sum) \
            if(scan_flag > 0)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % 5);
        
        #pragma omp scan exclusive(partial_sum)
        {
            arr[i] = partial_sum;
            partial_sum += val;
        }
    }
    
    /* Inclusive scan in separate region */
    int temp_sum = 0;
    #pragma omp parallel for reduction(+:temp_sum) if(scan_flag > 1)
    for (int i = 0; i < n; i++) {
        temp_sum += arr[i];
        arr[i] = temp_sum;
    }
    
    __builtin_printf("Scan operations completed, final sum=%d\n", temp_sum);
}

__attribute__((optimize("O2")))
void test_enter_data_clause(int **ptr_arr, int n, volatile int data_flag) {
    /* Allocate and initialize data */
    *ptr_arr = (int*)malloc(n * sizeof(int));
    if (*ptr_arr == NULL) return;
    
    for (int i = 0; i < n; i++) {
        (*ptr_arr)[i] = i * g_seed;
    }
    
    /* Use enter data with to clause */
    #pragma omp target enter data map(to: (*ptr_arr)[0:n]) \
            if(data_flag > 0)
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for \
            map(tofrom: (*ptr_arr)[0:n])
    for (int i = 0; i < n; i++) {
        (*ptr_arr)[i] = (*ptr_arr)[i] * 3 + 1;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: (*ptr_arr)[0:n])
    
    __builtin_printf("Enter data completed, first element=%d\n", (*ptr_arr)[0]);
}

__attribute__((optimize("O3")))
void nested_combined_constructs(int *arr1, int *arr2, int n, volatile int nest_flag) {
    /* Nested parallel regions with combined constructs */
    #pragma omp parallel if(nest_flag > 0) num_threads(2)
    {
        #pragma omp for reduction(+:g_seed) nowait
        for (int i = 0; i < n; i++) {
            arr1[i] += omp_get_thread_num();
        }
        
        #pragma omp single
        {
            #pragma omp taskloop if(nest_flag > 1) grainsize(16)
            for (int i = 0; i < n; i++) {
                arr2[i] = arr1[i] * arr2[i];
            }
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:n], arr2[0:n]) \
            if(nest_flag > 2) num_teams(2) thread_limit(64)
    for (int i = 0; i < n; i++) {
        arr1[i] = arr1[i] % 256;
        arr2[i] = arr2[i] % 256;
    }
    
    __builtin_printf("Nested constructs executed with flag=%d\n", nest_flag);
}

int main(int argc, char **argv) {
    /* Initialize with command-line seed if provided */
    if (argc > 1) {
        g_seed = atoi(argv[1]);
    }
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *device_array = NULL;
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * g_seed + 13) % 1000;
        array2[i] = (i * 17 + g_seed) % 1000;
    }
    
    volatile int iteration_counter = 2;  /* Force multiple iterations */
    
    /* Main test loop - executes OpenMP regions multiple times */
    for (int iter = 0; iter < iteration_counter + g_flag; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, g_flag + iter);
        
        /* 2. Test condition temporaries */
        test_condition_temporaries(array1, N, g_flag + iter);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, g_flag + iter);
        
        /* 4. Test enter data clause */
        test_enter_data_clause(&device_array, N/2, g_flag + iter);
        
        /* 5. Test nested and combined constructs */
        nested_combined_constructs(array1, array2, N, g_flag + iter);
        
        /* Calculate and print checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum + array1[i] + array2[i]) % 1000000;
        }
        if (device_array) {
            for (int i = 0; i < N/2; i++) {
                checksum = (checksum + device_array[i]) % 1000000;
            }
        }
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify seed for next iteration */
        g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output */
    __builtin_printf("\nFinal results: array1[0]=%d, array2[0]=%d\n", 
                     array1[0], array2[0]);
    
    /* Cleanup */
    free(array1);
    free(array2);
    if (device_array) free(device_array);
    
    return 0;
}
