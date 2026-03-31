/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization and constant folding */
static volatile int global_seed = 42;
static volatile int dump_trigger = 1;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temp(int *arr, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            if(flag > 0)  /* Additional if clause for complexity */
    for (int i = 0; i < n; i++) {
        /* Data-dependent index calculation */
        int idx = (i * 17 + global_seed) % n;
        sum += arr[idx] + (i % 3);
        prod *= (arr[idx] != 0 ? arr[idx] : 1) + (i % 5);
        
        /* Conditional updates to inhibit optimization */
        if (arr[idx] > max_val || (i % 7 == 0)) {
            max_val = arr[idx] + (i % 2);
        }
        if (arr[idx] < min_val || (i % 11 == 0)) {
            min_val = arr[idx] - (i % 3);
        }
        
        /* Modify array element to create dependencies */
        arr[idx] = (arr[idx] + i) % 1000;
    }
    
    /* Force tree dumping with non-optimizable printf */
    __builtin_printf("Reduction results: sum=%d prod=%d max=%d min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3")))
void test_cond_temp(int *arr, int n, volatile int flag) {
    volatile int cond1 = flag % 3;
    volatile int cond2 = flag % 5;
    volatile int cond3 = global_seed % 7;
    
    /* Multiple parallel regions with if clauses */
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        #pragma omp single
        {
            /* Tasks with if clauses */
            #pragma omp task if(cond2 > 1)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] += i * 2;
                }
            }
            
            #pragma omp task if(cond3 > 2)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] -= i / 2;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond1 + cond2 > 2) map(tofrom:arr[0:n/4]) \
            num_teams(2) thread_limit(8)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n/4; i++) {
            arr[i] = arr[i] * 3 % 997;
        }
    }
    
    __builtin_printf("Conditional temp check: cond1=%d cond2=%d\n", cond1, cond2);
}

__attribute__((optimize("O2")))
void test_scan_temp(int *arr, int n, volatile int flag) {
    int scan_sum = 0;
    int excl_scan = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            if(flag > 0)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % 13);
        
        #pragma omp scan exclusive(scan_sum)
        {
            arr[i] = scan_sum + val;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan with different pattern */
    #pragma omp parallel for reduction(inscan, +:excl_scan)
    for (int i = 0; i < n/2; i++) {
        int idx = n - i - 1;
        int val = arr[idx] % 37;
        
        #pragma omp scan inclusive(excl_scan)
        {
            excl_scan += val;
            arr[idx] = excl_scan;
        }
    }
    
    __builtin_printf("Scan results: scan_sum=%d excl_scan=%d\n", scan_sum, excl_scan);
}

__attribute__((optimize("O2")))
void test_enter_to(int *arr, int n, volatile int flag) {
    /* Dynamic allocation for enter data clause */
    int *dyn_arr = (int *)malloc(n * sizeof(int));
    if (!dyn_arr) return;
    
    /* Initialize dynamic array */
    for (int i = 0; i < n; i++) {
        dyn_arr[i] = arr[i] * 2 + i;
    }
    
    /* Use enter data with to modifier */
    #pragma omp enter data to(dyn_arr[0:n]) if(flag > 0)
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: dyn_arr[0:n/2])
    {
        for (int i = 0; i < n/2; i++) {
            dyn_arr[i] = (dyn_arr[i] * 3) % 991;
        }
    }
    
    /* Copy back results */
    for (int i = 0; i < n/2; i++) {
        arr[i] = dyn_arr[i];
    }
    
    /* Exit data */
    #pragma omp exit data from(dyn_arr[0:n])
    
    free(dyn_arr);
    __builtin_printf("Enter data completed for n=%d\n", n);
}

/* Main test function with nested constructs */
__attribute__((optimize("O3")))
void comprehensive_test(int *arr1, int *arr2, int n, volatile int iter) {
    volatile int mode = iter % 4;
    
    /* Outer parallel region */
    #pragma omp parallel if(mode > 0) num_threads(2)
    {
        /* Nested reduction inside parallel */
        test_reduction_temp(arr1, n, mode + global_seed);
        
        #pragma omp barrier
        
        #pragma omp single
        {
            test_cond_temp(arr2, n, mode + 1);
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:n]) if(mode > 1) \
            num_teams(2) num_threads(4)
    for (int i = 0; i < n; i++) {
        arr1[i] = (arr1[i] + arr2[i % n]) % 983;
    }
    
    test_scan_temp(arr1, n, mode + 2);
    test_enter_to(arr2, n, mode + 3);
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    global_seed = seed;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 13 + seed) % 997;
        array2[i] = (i * 17 + seed * 2) % 991;
    }
    
    volatile int iterations = 3;
    volatile int checksum = 0;
    
    /* Multiple iterations to increase compiler processing */
    for (volatile int iter = 0; iter < iterations; iter++) {
        dump_trigger = iter + seed;
        
        comprehensive_test(array1, array2, N, iter);
        
        /* Calculate checksum to prevent dead code elimination */
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + array1[i]) % 1000000007;
            checksum = (checksum * 37 + array2[i]) % 1000000007;
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    /* Final output that can't be optimized away */
    __builtin_printf("Final array1[0]=%d, array2[0]=%d\n", array1[0], array2[0]);
    
    free(array1);
    free(array2);
    
    return 0;
}
