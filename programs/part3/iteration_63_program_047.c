/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;
static volatile int dump_trigger = 1;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access
       Forces generation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access prevents optimization */
        int idx = (i * 17 + global_seed) % n;
        sum += arr1[idx] + arr2[i % n];
        
        /* Avoid multiplication by zero for product reduction */
        if (arr1[idx] != 0) {
            prod *= (arr1[idx] > 0 ? arr1[idx] : 1);
        }
        
        if (arr2[i % n] > max_val) max_val = arr2[i % n];
        if (arr2[i % n] < min_val) min_val = arr2[i % n];
        
        /* Additional computation to prevent loop simplification */
        arr1[idx] = (arr1[idx] * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Force tree dumping by using builtin printf with runtime values */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_condition_temporaries(int *arr, int n, volatile int cond_flag) {
    volatile int dynamic_condition = cond_flag + global_seed;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(dynamic_condition > 50) num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Nested task with another if clause */
        #pragma omp task if(thread_id % 2 == 0) firstprivate(arr)
        {
            for (int i = 0; i < n/4; i++) {
                arr[(thread_id * n/4 + i) % n] += thread_id;
            }
        }
        
        #pragma omp taskwait
        
        /* Target teams with if clause - different context */
        #pragma omp target teams if(dynamic_condition < 100) \
                map(tofrom: arr[0:n/2]) thread_limit(4)
        {
            #pragma omp distribute parallel for simd
            for (int i = 0; i < n/2; i++) {
                arr[i] = arr[i] * 2 + 1;
            }
        }
    }
    
    __builtin_printf("Condition test completed with flag=%d\n", dynamic_condition);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int scan_flag) {
    int scan_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            if(scan_flag > 0)
    for (int i = 0; i < n; i++) {
        /* Exclusive scan phase */
        #pragma omp scan exclusive(scan_sum)
        {
            int temp = arr[i];
            arr[i] = scan_sum;
            scan_sum += temp;
        }
    }
    
    /* Another scan variant */
    int prefix_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        {
            prefix_sum += arr[i];
            arr[i] = prefix_sum;
        }
    }
    
    __builtin_printf("Scan results: final_sum=%d, prefix_final=%d\n", 
                     scan_sum, prefix_sum);
}

__attribute__((optimize("O2")))
void test_enter_clause(volatile int enter_flag) {
    /* Dynamic allocation for enter data clause */
    int *device_array = (int*)malloc(256 * sizeof(int));
    if (!device_array) return;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        device_array[i] = i * i + global_seed;
    }
    
    /* OMP enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_array[0:256]) \
            if(enter_flag > 0)
    
    /* Use the device data */
    #pragma omp target teams distribute parallel for map(from: device_array[0:256])
    for (int i = 0; i < 256; i++) {
        device_array[i] = device_array[i] * 3 - 2;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_array[0:256])
    
    /* Verify some values */
    int check = 0;
    for (int i = 0; i < 256; i += 16) {
        check ^= device_array[i];
    }
    
    __builtin_printf("Enter clause test: checksum=0x%x\n", check);
    
    free(device_array);
}

/* Main test function with nested OpenMP regions */
__attribute__((optimize("O2")))
void comprehensive_test(int *arr1, int *arr2, int n, volatile int iter) {
    volatile int control = iter + global_seed;
    
    /* Nested parallel regions */
    #pragma omp parallel if(control % 3 == 0) num_threads(2)
    {
        #pragma omp single
        {
            /* Combined construct */
            #pragma omp taskloop reduction(+:global_seed) grainsize(8)
            for (int i = 0; i < n; i++) {
                global_seed += arr1[i] % 7;
            }
        }
        
        /* Inner parallel for with reduction */
        #pragma omp parallel for reduction(*:dump_trigger) \
                if(control % 5 == 0) collapse(2)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < n) {
                    arr2[idx] = arr1[idx] * arr2[idx] + control;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    global_seed = seed;
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0x7fff;
        array2[i] = (i * 1664525 + 1013904223) & 0x7fff;
    }
    
    volatile int iterations = 2;
    volatile int flags[4] = {1, 0, 1, 0};
    
    /* Multiple iterations to ensure processing */
    for (volatile int iter = 0; iter < iterations; iter++) {
        flags[0] = iter % 2;
        flags[1] = (iter + 1) % 2;
        flags[2] = (iter + seed) % 3;
        flags[3] = (iter * 2) % 4;
        
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* Test all clause types */
        test_reduction_temporaries(array1, array2, N, flags[0]);
        test_condition_temporaries(array1, N, flags[1]);
        test_scan_temporaries(array2, N, flags[2]);
        test_enter_clause(flags[3]);
        
        /* Comprehensive nested test */
        comprehensive_test(array1, array2, N, iter);
        
        /* Compute checksum to prevent dead code elimination */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum ^= array1[i] * 31 + array2[i];
            /* Modify arrays for next iteration */
            array1[i] = (array1[i] + checksum) & 0xffff;
            array2[i] = (array2[i] + checksum * 7) & 0xffff;
        }
        
        __builtin_printf("Iteration %d checksum: 0x%08x\n", iter, checksum);
        
        /* Force side effects */
        global_seed = (global_seed * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    /* Final output */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] - array2[i];
    }
    __builtin_printf("\nFinal result: %d (seed=%d)\n", final_sum, seed);
    
    free(array1);
    free(array2);
    
    return 0;
}
