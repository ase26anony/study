/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;
static volatile int dump_trigger = 1;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    float max_val = arr1[0];
    float min_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            if(flag > 0)  /* Add conditional to create more complex tree */
    for (int i = 0; i < n; i++) {
        /* Data-dependent indexing to prevent optimization */
        int idx = (i + global_seed) % n;
        sum += arr1[idx] + arr2[i % n];
        
        /* Avoid multiplication by zero */
        if (arr1[idx] != 0 && arr2[i % n] != 0) {
            prod *= (arr1[idx] % 10 + 1) * (arr2[i % n] % 10 + 1);
        }
        
        float val = arr1[idx] * 0.5f + arr2[i % n] * 0.3f;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    /* Force tree dump with non-optimizable printf */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%.2f, min=%.2f\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void test_condtemp_clauses(int *arr, int n, volatile int cond) {
    /* Multiple OpenMP constructs with if clauses */
    volatile int dynamic_cond = cond + global_seed;
    
    #pragma omp parallel if(dynamic_cond > 25) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        #pragma omp task if(tid % 2 == 0 && dynamic_cond > 10)
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] += tid;
            }
        }
        
        #pragma omp task if(tid % 3 == 0 && dynamic_cond > 15)
        {
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] * 2 - 1;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* Target region with if clause */
    #pragma omp target teams if(dynamic_cond > 30) map(tofrom:arr[0:n]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for if(dynamic_cond > 20)
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] % 100;
        }
    }
    
    __builtin_printf("Condtemp processed: cond=%d\n", dynamic_cond);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int n, volatile int mode) {
    int scan_sum = 0;
    int exclusive_prefix = 0;
    
    /* OpenMP 5.0/5.1 scan directives */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            if(mode == 1)
    for (int i = 0; i < n; i++) {
        /* Exclusive scan phase */
        #pragma omp scan exclusive(exclusive_prefix)
        {
            arr[i] = exclusive_prefix;
            exclusive_prefix += (arr[i] % 10) + 1;
        }
        
        /* Inscan reduction */
        scan_sum += arr[i];
        
        #pragma omp scan inclusive(scan_sum)
        {
            arr[i] += scan_sum;
        }
    }
    
    /* Another scan pattern */
    int temp_arr[256];
    for (int i = 0; i < 256 && i < n; i++) {
        temp_arr[i] = arr[i];
    }
    
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            if(mode == 2)
    for (int i = 0; i < 256 && i < n; i++) {
        int val = temp_arr[i];
        #pragma omp scan exclusive(scan_sum)
        temp_arr[i] = scan_sum;
        scan_sum += val;
    }
    
    __builtin_printf("Scan results: sum=%d, mode=%d\n", scan_sum, mode);
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(volatile int size) {
    /* Use enter data with to modifier */
    int data_size = size > 0 ? size : 512;
    int *device_array = (int*)malloc(data_size * sizeof(int));
    
    if (!device_array) return;
    
    /* Initialize array */
    for (int i = 0; i < data_size; i++) {
        device_array[i] = i * 2 + global_seed;
    }
    
    /* OMP enter clause with to modifier */
    #pragma omp enter data to(device_array[0:data_size])
    
    /* Use the array in target region */
    #pragma omp target map(tofrom: device_array[0:data_size]) \
            if(dump_trigger > 0)
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < data_size; i++) {
            device_array[i] = device_array[i] * 3 + 7;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(device_array[0:data_size])
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < data_size && i < 100; i++) {
        checksum += device_array[i];
    }
    
    __builtin_printf("Enter data checksum: %d (size=%d)\n", checksum, data_size);
    
    free(device_array);
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argc to seed variability */
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
    srand(seed);
    for (int i = 0; i < N; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
    }
    
    volatile int iteration_control = 2;  /* Force multiple iterations */
    volatile int mode_selector = 1;
    
    /* Multiple iterations to increase compiler processing */
    for (int iter = 0; iter < iteration_control + (seed % 3); iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, iter);
        
        /* 2. Test condtemp clauses */
        test_condtemp_clauses(array1, N, iter + mode_selector);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, (iter % 2) + 1);
        
        /* 4. Test enter data with to modifier */
        test_enter_data_clause(N / (iter + 1) + 100);
        
        /* Modify control variables to prevent dead code elimination */
        mode_selector += iter * 3;
        dump_trigger = (iter % 2 == 0) ? 1 : 0;
    }
    
    /* Final checksum to prevent optimization */
    int final_checksum = 0;
    for (int i = 0; i < N; i++) {
        final_checksum += array1[i] + array2[i];
        final_checksum %= 1000000;
    }
    
    __builtin_printf("\nFinal checksum: %d (seed=%d)\n", final_checksum, seed);
    
    free(array1);
    free(array2);
    
    return 0;
}
