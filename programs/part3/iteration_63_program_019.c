/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;
static volatile int dump_counter = 0;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators on arrays */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2) if(flag > 0)
    for (i = 0; i < n; i++) {
        /* Data-dependent operations to inhibit optimization */
        int idx = (i + global_seed) % n;
        sum += arr1[idx] * (i % 7 + 1);
        prod *= (arr2[idx] % 10 + 1);
        
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Cross-update arrays to create dependencies */
        arr2[idx] = arr1[idx] + (i % 3);
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2"), noinline))
void test_condition_temporaries(int *arr, int n, volatile int flag) {
    int i;
    
    /* Parallel region with runtime-dependent condition */
    #pragma omp parallel if(flag > 0) num_threads(4)
    {
        volatile int local_flag = flag + omp_get_thread_num();
        
        /* Task with condition */
        #pragma omp task if(local_flag > 2) firstprivate(local_flag)
        {
            int j;
            for (j = 0; j < n/4; j++) {
                arr[(j + local_flag) % n] += local_flag;
            }
        }
        
        /* Another parallel region inside */
        #pragma omp for
        for (i = 0; i < n; i++) {
            arr[i] = (arr[i] * 3) % 100;
        }
    }
    
    /* Target teams with condition */
    #pragma omp target teams if(flag > 1) map(tofrom:arr[0:n]) \
            num_teams(2) num_threads(4)
    {
        int team_id = omp_get_team_num();
        #pragma omp parallel for
        for (i = 0; i < n; i++) {
            arr[i] += team_id;
        }
    }
    
    __builtin_printf("Condition test completed, flag=%d\n", flag);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int n, volatile int flag) {
    int i;
    int scan_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i) if(flag > 0)
    for (i = 0; i < n; i++) {
        /* Exclusive scan phase */
        #pragma omp scan exclusive(scan_sum)
        {
            int val = arr[i] + (i % 5);
            arr[i] = scan_sum;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan with different pattern */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i) if(flag > 1)
    for (i = n-1; i >= 0; i--) {
        /* Inclusive scan phase */
        #pragma omp scan inclusive(scan_sum)
        {
            scan_sum += arr[i];
            arr[i] = scan_sum;
        }
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(volatile int flag) {
    int *device_array = NULL;
    int n = 512;
    
    if (flag > 0) {
        device_array = (int*)malloc(n * sizeof(int));
        if (!device_array) return;
        
        /* Initialize array */
        for (int i = 0; i < n; i++) {
            device_array[i] = i * 2 + global_seed;
        }
        
        /* Use enter data with to clause - triggers OMP_CLAUSE_ENTER with to modifier */
        #pragma omp target enter data map(to: device_array[0:n])
        
        /* Use the device array in a target region */
        #pragma omp target teams distribute parallel for \
                map(always, tofrom: device_array[0:n])
        for (int i = 0; i < n; i++) {
            device_array[i] = device_array[i] * 3 + omp_get_thread_num();
        }
        
        /* Exit data */
        #pragma omp target exit data map(from: device_array[0:n])
        
        /* Verify some values */
        int check = 0;
        for (int i = 0; i < 10; i++) {
            check += device_array[i];
        }
        __builtin_printf("Enter data test: checksum=%d\n", check);
        
        free(device_array);
    }
}

__attribute__((optimize("O2"), noinline))
void nested_combined_constructs(int *arr1, int *arr2, int n, volatile int flag) {
    /* Nested parallel regions with combined constructs */
    #pragma omp parallel if(flag > 0)
    {
        #pragma omp for reduction(+:global_seed) nowait
        for (int i = 0; i < n; i++) {
            arr1[i] = (arr1[i] + arr2[i]) * 2;
            global_seed += (i % 11);
        }
        
        #pragma omp single
        {
            #pragma omp task if(flag > 1)
            {
                /* Combined target teams distribute parallel for */
                #pragma omp target teams distribute parallel for \
                        map(tofrom: arr2[0:n/2]) if(flag > 2)
                for (int i = 0; i < n/2; i++) {
                    arr2[i] = arr2[i] * arr2[i] % 1000;
                }
            }
        }
    }
    
    __builtin_printf("Nested constructs completed\n");
}

int main(int argc, char **argv) {
    /* Initialize with command line seed for runtime variability */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    global_seed = seed;
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 13 + seed) % 100;
        array2[i] = (i * 17 + seed * 2) % 100;
    }
    
    /* Volatile flags to control execution */
    volatile int flag1 = seed % 3;
    volatile int flag2 = (seed * 2) % 5;
    volatile int flag3 = (seed * 3) % 7;
    volatile int flag4 = (seed * 5) % 11;
    
    /* Execute test functions multiple times with different flags */
    for (dump_counter = 0; dump_counter < 3; dump_counter++) {
        __builtin_printf("\n=== Iteration %d ===\n", dump_counter);
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(array1, array2, N, 
                                 flag1 + dump_counter);
        
        /* Test 2: Condition temporaries */
        test_condition_temporaries(array1, N, 
                                 flag2 + dump_counter);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(array2, N, 
                            flag3 + dump_counter);
        
        /* Test 4: Enter data clause */
        test_enter_data_clause(flag4 + dump_counter);
        
        /* Test 5: Nested and combined constructs */
        nested_combined_constructs(array1, array2, N, 
                                 (flag1 + flag2 + dump_counter) % 4);
        
        /* Calculate checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + array1[i]) % 1000000;
            checksum = (checksum * 17 + array2[i]) % 1000000;
        }
        __builtin_printf("Iteration %d checksum: %d\n", dump_counter, checksum);
        
        /* Modify flags for next iteration */
        flag1 = (flag1 * 3 + 1) % 7;
        flag2 = (flag2 * 5 + 1) % 11;
    }
    
    /* Final output */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    __builtin_printf("\nFinal sum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
