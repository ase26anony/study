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

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2, v_flag1)
    for (i = 0; i < n; i++) {
        /* Data-dependent index calculation to inhibit optimization */
        int idx = (i + seed + v_flag1) % n;
        sum += arr1[idx] + arr2[i % n];
        
        /* Avoid multiplication by zero to prevent trivial optimization */
        if (arr1[idx] != 0 && arr2[i % n] != 0) {
            prod *= (arr1[idx] % 10 + 1) * (arr2[i % n] % 10 + 1);
        }
        
        /* Conditional updates to force temporary creation */
        if (arr1[idx] > max_val) {
            max_val = arr1[idx];
        }
        if (arr2[i % n] < min_val && arr2[i % n] > 0) {
            min_val = arr2[i % n];
        }
        
        /* Cross-update arrays to create dependencies */
        if (v_flag1 && (i % 3 == 0)) {
            arr1[idx] = (arr1[idx] + arr2[i % n]) % 100;
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, int seed) {
    int i;
    
    /* Nested OpenMP with volatile conditions */
    #pragma omp parallel if(v_cond) num_threads(4)
    {
        /* Runtime-dependent condition */
        int local_cond = (seed + omp_get_thread_num()) % 3;
        
        #pragma omp single
        {
            #pragma omp task if(local_cond > 0) shared(arr, v_flag2)
            {
                for (int j = 0; j < n/2; j++) {
                    arr[j] = (arr[j] * 3) % 100;
                }
            }
            
            #pragma omp task if(local_cond > 1) shared(arr, v_flag2)
            {
                for (int j = n/2; j < n; j++) {
                    arr[j] = (arr[j] + 7) % 100;
                }
            }
        }
        
        /* Combined construct with if clause */
        #pragma omp for schedule(dynamic) if(v_flag2)
        for (i = 0; i < n; i++) {
            arr[i] = (arr[i] + i + seed) % 100;
        }
    }
    
    /* Target region with if clause */
    #pragma omp target teams if(v_cond && v_flag1) map(tofrom:arr[0:n/2]) thread_limit(4)
    {
        #pragma omp distribute parallel for simd if(v_flag2)
        for (i = 0; i < n/2; i++) {
            arr[i] = arr[i] * 2;
        }
    }
    
    __builtin_printf("Conditional test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, int seed) {
    int i;
    int scan_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan,+:scan_sum) \
            private(i) shared(arr) schedule(static)
    for (i = 0; i < n; i++) {
        int val = (arr[i] + seed + i) % 50;
        
        #pragma omp scan exclusive(scan_sum)
        {
            arr[i] = scan_sum;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan with different operator */
    int scan_prod = 1;
    #pragma omp parallel for reduction(inscan,*:scan_prod) \
            private(i) shared(arr, v_flag1)
    for (i = 0; i < n/2; i++) {
        int val = (arr[i] % 7) + 2;  /* Ensure val >= 2 for product */
        
        #pragma omp scan inclusive(scan_prod)
        {
            scan_prod *= val;
            arr[i + n/2] = scan_prod % 1000;
        }
    }
    
    __builtin_printf("Scan results: final_sum=%d, final_prod=%d\n", 
                     scan_sum, scan_prod);
}

__attribute__((optimize("O2")))
void test_enter_data_with_to(int **ptr_arr, int n) {
    /* Dynamic allocation for enter data clause */
    int *device_data = (int *)malloc(n * sizeof(int));
    if (!device_data) return;
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        device_data[i] = i * 3 + 7;
    }
    
    /* Use enter data with to modifier - this should trigger OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_data[0:n])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for map(tofrom: device_data[0:n])
    for (int i = 0; i < n; i++) {
        device_data[i] = device_data[i] * 2 + 1;
    }
    
    /* Retrieve data */
    #pragma omp target exit data map(from: device_data[0:n])
    
    /* Store result */
    *ptr_arr = device_data;
    
    __builtin_printf("Enter data test: device_data[0]=%d\n", device_data[0]);
}

/* Main test function with nested OpenMP regions */
__attribute__((optimize("O2")))
void run_omp_tests(int *arr1, int *arr2, int n, int seed) {
    /* Nested parallel region containing reduction */
    #pragma omp parallel if(v_cond) num_threads(2)
    {
        #pragma omp single
        {
            test_reduction_temporaries(arr1, arr2, n, seed);
        }
        
        #pragma omp barrier
        
        #pragma omp for
        for (int i = 0; i < 2; i++) {
            test_conditional_temporaries(arr1, n, seed + i);
        }
    }
    
    /* Scan operations */
    test_scan_temporaries(arr2, n, seed);
    
    /* Enter data with to */
    int *device_arr = NULL;
    test_enter_data_with_to(&device_arr, n/4);
    
    if (device_arr) {
        /* Use device_arr to prevent optimization */
        int check = 0;
        for (int i = 0; i < n/4; i++) {
            check ^= device_arr[i];
        }
        __builtin_printf("Device array checksum: %d\n", check);
        free(device_arr);
    }
}

int main(int argc, char *argv[]) {
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values based on argv */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]) % 1000;
    }
    
    /* Simple LCG for variability */
    unsigned int lcg = seed;
    for (int i = 0; i < N; i++) {
        lcg = lcg * 1103515245 + 12345;
        array1[i] = (lcg >> 16) % 100;
        lcg = lcg * 1103515245 + 12345;
        array2[i] = (lcg >> 16) % 100;
    }
    
    /* Volatile iteration control */
    volatile int iterations = v_iter;
    if (argc > 2) {
        iterations = atoi(argv[2]) % 5 + 1;
    }
    
    /* Run tests multiple times with different parameters */
    int total_checksum = 0;
    for (int iter = 0; iter < iterations; iter++) {
        v_flag1 = (iter % 2);
        v_flag2 = ((iter + 1) % 2);
        v_cond = (iter % 3 != 0);
        
        run_omp_tests(array1, array2, N, seed + iter);
        
        /* Compute checksum to prevent elimination */
        for (int i = 0; i < N; i++) {
            total_checksum ^= array1[i] + array2[i];
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, total_checksum);
    }
    
    /* Final output to ensure all code paths are used */
    printf("Final results: array1[0]=%d, array2[0]=%d, total_checksum=%d\n",
           array1[0], array2[0], total_checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
