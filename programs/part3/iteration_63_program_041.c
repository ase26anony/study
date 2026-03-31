/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int g_flag1 = 1;
volatile int g_flag2 = 0;
volatile int g_iter = 2;

/* Function with complex reduction operations */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    volatile int v_seed = seed;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(v_seed)
    for (int i = 0; i < n; i++) {
        /* Data-dependent index calculation */
        int idx = (i + v_seed) % n;
        idx = (idx < 0) ? 0 : idx;
        
        /* Multiple reduction operations with array access */
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] != 0) ? arr1[i] : 1;
        max_val = (arr2[idx] > max_val) ? arr2[idx] : max_val;
        min_val = (arr1[i] < min_val) ? arr1[i] : min_val;
        
        /* Cross-update to inhibit optimization */
        if (i % 3 == 0) {
            arr1[idx] = arr2[i % n] + v_seed;
        }
    }
    
    __builtin_printf("Reduction checksum: %d %d %d %d\n", sum, prod, max_val, min_val);
}

/* Function with conditional temporaries */
__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, int seed) {
    volatile int v_cond = seed % 3;
    volatile int v_flag = g_flag1;
    
    /* Multiple if clauses in different OpenMP contexts */
    #pragma omp parallel if(v_cond > 0) num_threads(4)
    {
        #pragma omp task if(v_flag) shared(arr)
        {
            for (int i = 0; i < n/2; i++) {
                arr[i] += i * v_cond;
            }
        }
        
        #pragma omp task if(v_cond < 2) shared(arr)
        {
            for (int i = n/2; i < n; i++) {
                arr[i] -= i * v_cond;
            }
        }
        
        #pragma omp taskwait
        
        /* Nested parallel region with if clause */
        #pragma omp parallel if(v_cond == 1) num_threads(2)
        {
            #pragma omp for
            for (int i = 0; i < n; i++) {
                arr[i] *= (v_flag) ? 2 : 1;
            }
        }
    }
    
    /* Target region with if clause */
    #pragma omp target teams if(v_cond != 2) map(tofrom:arr[0:n]) \
            num_teams(2) thread_limit(64)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] += 1;
        }
    }
    
    __builtin_printf("Conditional temp checksum: %d\n", arr[n/2]);
}

/* Function with scan temporaries */
__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, int seed) {
    volatile int v_seed = seed;
    int scan_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + v_seed;
        
        #pragma omp scan exclusive(scan_sum)
        {
            arr[i] = scan_sum;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan with different array */
    int *arr2 = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        arr2[i] = (i + v_seed) % 10;
    }
    
    int inclusive_sum = 0;
    #pragma omp parallel for reduction(inscan, +:inclusive_sum)
    for (int i = 0; i < n; i++) {
        inclusive_sum += arr2[i];
        
        #pragma omp scan inclusive(inclusive_sum)
        arr2[i] = inclusive_sum;
    }
    
    __builtin_printf("Scan checksum: %d %d\n", scan_sum, inclusive_sum);
    free(arr2);
}

/* Function with enter data and to modifier */
__attribute__((optimize("O2")))
void test_enter_data(int *data, int n, int seed) {
    volatile int v_seed = seed;
    
    /* Allocate and initialize device data */
    int *device_data = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        device_data[i] = data[i] + v_seed;
    }
    
    /* Use enter data with to modifier */
    #pragma omp enter data to(device_data[0:n])
    
    /* Perform computation on device */
    #pragma omp target map(tofrom:device_data[0:n])
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < n; i++) {
            device_data[i] *= 2;
            device_data[i] += i;
        }
    }
    
    /* Copy back and update original */
    #pragma omp exit data from(device_data[0:n])
    
    for (int i = 0; i < n; i++) {
        data[i] = device_data[i];
    }
    
    __builtin_printf("Enter data checksum: %d\n", data[n/3]);
    free(device_data);
}

/* Main driver with volatile control flow */
int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int v_seed = seed;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 13 + v_seed) % 100;
        array2[i] = (i * 17 + v_seed) % 100;
    }
    
    volatile int loop_counter = g_iter;
    int total_checksum = 0;
    
    /* Multiple iterations to ensure clause processing */
    for (int iter = 0; iter < loop_counter; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, v_seed + iter);
        
        /* 2. Test conditional temporaries */
        test_conditional_temporaries(array1, N, v_seed + iter * 2);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, v_seed + iter * 3);
        
        /* 4. Test enter data with to modifier */
        test_enter_data(array1, N, v_seed + iter * 4);
        
        /* Update volatile flags for next iteration */
        g_flag1 = !g_flag1;
        g_flag2 = iter % 2;
        
        /* Calculate checksum to prevent elimination */
        for (int i = 0; i < N; i += 8) {
            total_checksum += array1[i] + array2[i];
        }
    }
    
    __builtin_printf("\nFinal checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
