/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;
static volatile int dump_counter = 0;

/* Function attribute to force optimization and tree dumping */
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
        int idx = (i * global_seed) % n;
        sum += arr1[idx] + (flag & 1);
        prod *= (arr1[idx] % 10 + 1);
        
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Modify arr2 in a way that might require temporaries */
        arr2[i] = arr1[idx] * (i % 7 + 1);
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3")))
void test_condtemp_clauses(int *arr, int n, volatile int cond_flag) {
    volatile int runtime_cond = cond_flag;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(runtime_cond > 0) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with if clause inside parallel region */
        #pragma omp task if(tid % 2 == 0) firstprivate(tid)
        {
            int local_sum = 0;
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                local_sum += arr[i];
            }
            arr[tid % n] = local_sum % 1000;
        }
        
        #pragma omp taskwait
        
        /* Nested parallel with different if condition */
        #pragma omp parallel if(runtime_cond < 10) num_threads(2)
        {
            #pragma omp for
            for (int i = 0; i < n/2; i++) {
                arr[i] = (arr[i] * 3) % 997;
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(runtime_cond != 0) \
            map(tofrom:arr[0:n/4]) num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n/4; i++) {
            arr[i] = arr[i] ^ 0x55;
        }
    }
    
    __builtin_printf("Condtemp test completed with flag=%d\n", runtime_cond);
}

__attribute__((optimize("O2")))
void test_scantemp_clauses(int *arr, int n, volatile int scan_type) {
    int partial_sum = 0;
    int exclusive_prefix = 0;
    
    /* Exclusive scan operation */
    #pragma omp parallel for reduction(inscan, +:partial_sum) \
            if(scan_type == 0)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (global_seed % 5);
        
        #pragma omp scan exclusive(partial_sum)
        {
            arr[i] = partial_sum + val;
            partial_sum += val;
        }
    }
    
    /* Inclusive scan with different pattern */
    partial_sum = 0;
    #pragma omp parallel for reduction(inscan, +:partial_sum) \
            if(scan_type == 1)
    for (int i = n-1; i >= 0; i--) {
        int val = arr[i] * 2;
        
        #pragma omp scan inclusive(partial_sum)
        {
            partial_sum += val;
            arr[i] = partial_sum;
        }
    }
    
    /* Combined scan in nested parallel region */
    #pragma omp parallel if(scan_type == 2)
    {
        int local_arr[64];
        for (int i = 0; i < 64 && i < n; i++) {
            local_arr[i] = arr[i] + omp_get_thread_num();
        }
        
        #pragma omp for reduction(inscan, +:exclusive_prefix)
        for (int i = 0; i < 64 && i < n; i++) {
            #pragma omp scan exclusive(exclusive_prefix)
            {
                local_arr[i] = exclusive_prefix;
                exclusive_prefix += local_arr[i] % 7;
            }
            arr[i] = local_arr[i];
        }
    }
    
    __builtin_printf("Scantemp test completed, final sum=%d\n", partial_sum);
}

__attribute__((optimize("O2")))
void test_enter_clause(volatile int use_device) {
    /* Dynamic allocation for enter data clause */
    int *device_array = (int*)malloc(256 * sizeof(int));
    if (!device_array) return;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        device_array[i] = i * i + global_seed;
    }
    
    /* Use enter data with to clause - should trigger OMP_CLAUSE_ENTER with to modifier */
    if (use_device) {
        #pragma omp target enter data map(to: device_array[0:256])
        
        /* Perform computation on device */
        #pragma omp target teams distribute parallel for \
                map(always, tofrom: device_array[0:256])
        for (int i = 0; i < 256; i++) {
            device_array[i] = device_array[i] * 3 + 1;
        }
        
        /* Exit data */
        #pragma omp target exit data map(from: device_array[0:256])
    }
    
    /* Also test with structured block */
    #pragma omp target data map(tofrom: device_array[0:128]) if(use_device > 1)
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < 128; i++) {
            device_array[i] = device_array[i] / 2;
        }
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum = (checksum + device_array[i]) % 1000000007;
    }
    
    __builtin_printf("Enter clause test, checksum=%d\n", checksum);
    free(device_array);
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Initialize with command line seed if provided */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    }
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17 + global_seed) % 1000;
        array2[i] = (i * 23 + global_seed) % 1000;
    }
    
    volatile int execution_rounds = (argc > 2) ? atoi(argv[2]) : 3;
    volatile int cond_flag = 1;
    volatile int scan_flag = 0;
    volatile int use_device_flag = (argc > 3) ? atoi(argv[3]) : 0;
    
    /* Multiple execution rounds to ensure code paths are taken */
    for (dump_counter = 0; dump_counter < execution_rounds; dump_counter++) {
        __builtin_printf("\n=== Execution Round %d ===\n", dump_counter + 1);
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(array1, array2, N, dump_counter);
        
        /* Test 2: Conditional temporaries */
        cond_flag = (dump_counter % 2 == 0) ? 1 : 5;
        test_condtemp_clauses(array1, N, cond_flag);
        
        /* Test 3: Scan temporaries */
        scan_flag = dump_counter % 3;
        test_scantemp_clauses(array2, N, scan_flag);
        
        /* Test 4: Enter clause with to modifier */
        if (dump_counter > 0 || use_device_flag) {
            test_enter_clause(use_device_flag || dump_counter);
        }
        
        /* Compute and print overall checksum to prevent optimization */
        int total_checksum = 0;
        for (int i = 0; i < N; i++) {
            total_checksum = (total_checksum + array1[i] + array2[i]) % 1000000007;
        }
        __builtin_printf("Round %d total checksum: %d\n", 
                        dump_counter + 1, total_checksum);
        
        /* Modify seed for next round */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    free(array1);
    free(array2);
    
    __builtin_printf("Test completed successfully\n");
    return 0;
}
