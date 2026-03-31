/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization of OpenMP conditions */
volatile int g_flag = 1;
volatile int g_iter = 2;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    volatile int vseed = seed;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access
     * Likely to generate _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(vseed)
    for (int i = 0; i < n; i++) {
        /* Data-dependent index calculation to inhibit optimization */
        int idx = (i + vseed) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero product */
        if (arr2[i] > max_val) max_val = arr2[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Additional computation to increase complexity */
        arr1[i] = (arr1[i] + arr2[i]) % 100;
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, int seed) {
    volatile int cond1 = seed % 3;
    volatile int cond2 = (seed * 7) % 5;
    
    /* Multiple if clauses in different OpenMP contexts
     * Likely to generate _condtemp_ temporaries */
    
    /* Parallel region with if clause */
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        /* Task with if clause */
        #pragma omp task if(cond2 == 0)
        {
            for (int i = 0; i < n/2; i++) {
                arr[i] *= 2;
            }
        }
        
        #pragma omp task if(cond1 < 2)
        {
            for (int i = n/2; i < n; i++) {
                arr[i] /= 2;
            }
        }
        
        #pragma omp taskwait
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond1 == 1) map(tofrom:arr[0:n]) thread_limit(4)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] += i;
        }
    }
    
    __builtin_printf("Conditional test completed with cond1=%d, cond2=%d\n", 
                     cond1, cond2);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, int seed) {
    volatile int vseed = seed;
    int prefix_sum = 0;
    
    /* Exclusive scan - likely to generate _scantemp_ temporaries */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (vseed % 10);
        
        #pragma omp scan exclusive(prefix_sum)
        {
            arr[i] = prefix_sum;
            prefix_sum += val;
        }
    }
    
    /* Another scan variant */
    int scan_temp = 0;
    #pragma omp parallel for simd reduction(inscan, +:scan_temp)
    for (int i = 0; i < n; i++) {
        int val = arr[i] * 2;
        
        #pragma omp scan exclusive(scan_temp)
        {
            arr[i] = scan_temp;
            scan_temp += val;
        }
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", prefix_sum + scan_temp);
}

__attribute__((optimize("O2")))
void test_enter_data_to(int **dyn_arr, int n, int seed) {
    /* Dynamic allocation for enter data clause */
    *dyn_arr = (int *)malloc(n * sizeof(int));
    if (!*dyn_arr) return;
    
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = i * seed;
    }
    
    /* OMP enter clause with to modifier */
    #pragma omp enter data to(*dyn_arr[0:n])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: (*dyn_arr)[0:n])
    {
        for (int i = 0; i < n; i++) {
            (*dyn_arr)[i] += 1000;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(*dyn_arr[0:n])
    
    __builtin_printf("Enter data test completed, arr[0]=%d\n", (*dyn_arr)[0]);
}

/* Main test function with nested OpenMP regions */
__attribute__((optimize("O2")))
void comprehensive_omp_test(int *arr1, int *arr2, int n, int seed) {
    volatile int outer_flag = seed % 2;
    
    /* Outer parallel region */
    #pragma omp parallel if(outer_flag) num_threads(2)
    {
        /* Nested reduction region */
        #pragma omp for reduction(+:arr1[0:n]) nowait
        for (int i = 0; i < n; i++) {
            arr1[i] += i * seed;
        }
        
        /* Combined construct */
        #pragma omp target teams distribute parallel for \
                map(tofrom: arr2[0:n]) if(outer_flag)
        for (int i = 0; i < n; i++) {
            arr2[i] = (arr2[i] * 3) % 100;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int repeat_count = g_iter;
    const int N = 512;
    
    /* Initialize arrays with pseudo-random values */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *dyn_arr = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Simple LCG PRNG for initialization */
    unsigned int lcg = seed;
    for (int i = 0; i < N; i++) {
        lcg = lcg * 1103515245 + 12345;
        arr1[i] = (lcg >> 16) % 1000;
        arr2[i] = (lcg >> 8) % 1000;
    }
    
    /* Main test loop - repeats to increase chance of tree dumping */
    for (volatile int iter = 0; iter < repeat_count; iter++) {
        int current_seed = seed + iter * 17;
        
        __builtin_printf("\n=== Iteration %d, seed=%d ===\n", iter, current_seed);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(arr1, arr2, N, current_seed);
        
        /* 2. Test conditional temporaries */
        test_conditional_temporaries(arr1, N, current_seed);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(arr2, N, current_seed);
        
        /* 4. Test enter data with to modifier */
        test_enter_data_to(&dyn_arr, N/4, current_seed);
        
        /* 5. Comprehensive nested test */
        comprehensive_omp_test(arr1, arr2, N, current_seed);
        
        /* Calculate and print checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum + arr1[i] + arr2[i]) % 1000000;
        }
        if (dyn_arr) {
            for (int i = 0; i < N/4; i++) {
                checksum = (checksum + dyn_arr[i]) % 1000000;
            }
        }
        __builtin_printf("Checksum after iteration %d: %d\n", iter, checksum);
    }
    
    /* Cleanup */
    if (dyn_arr) free(dyn_arr);
    free(arr1);
    free(arr2);
    
    return 0;
}
