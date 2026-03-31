/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization and constant folding */
volatile int g_flag = 1;
volatile int g_seed = 42;

/* Function attribute to ensure optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temp(int *arr1, int *arr2, int n) {
    volatile int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators on arrays
     * Forces creation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2)
    for (i = 0; i < n; i++) {
        /* Data-dependent operations to inhibit optimization */
        int idx = (i + g_seed) % n;
        sum += arr1[idx] * (i % 7) + arr2[idx];
        prod *= (arr1[idx] % 10 + 1);
        
        /* Conditional max/min with volatile dependency */
        if (g_flag || arr2[idx] > max_val) {
            max_val = arr2[idx];
        }
        if (!g_flag || arr2[idx] < min_val) {
            min_val = arr2[idx];
        }
        
        /* Cross-update arrays to create dependencies */
        arr1[idx] = (arr1[idx] + arr2[idx]) % 100;
        arr2[(idx + 1) % n] += i % 3;
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void test_conditional_temp(int *arr, int n) {
    volatile int cond1 = g_seed % 3;
    volatile int cond2 = g_flag;
    
    /* Multiple if clauses in different OpenMP contexts
     * Forces creation of _condtemp_ temporaries */
    
    /* Parallel region with if clause */
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        /* Task with if clause */
        #pragma omp task if(cond2)
        {
            for (int i = 0; i < n/2; i++) {
                arr[i] *= 2;
            }
        }
        
        #pragma omp task if(!cond2)
        {
            for (int i = n/2; i < n; i++) {
                arr[i] /= 2;
            }
        }
        
        #pragma omp taskwait
        
        /* Nested parallel with if clause */
        #pragma omp parallel for if(cond1 == 2) schedule(dynamic)
        for (int i = 0; i < n; i++) {
            arr[i] += (i % 5);
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond1 == 1) map(tofrom:arr[0:n]) thread_limit(8)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] % 100 + 50;
        }
    }
    
    __builtin_printf("Conditional test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temp(int *arr, int n) {
    int partial_sum = 0;
    int exclusive_prefix = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:partial_sum) \
            private(exclusive_prefix)
    for (int i = 0; i < n; i++) {
        exclusive_prefix = partial_sum;
        #pragma omp scan exclusive(partial_sum)
        arr[i] += exclusive_prefix;
        partial_sum += (i % 7) + 1;
    }
    
    /* Inclusive scan */
    int inclusive_prefix = 0;
    #pragma omp parallel for reduction(inscan, +:inclusive_prefix)
    for (int i = 0; i < n; i++) {
        inclusive_prefix += arr[i] % 5;
        #pragma omp scan inclusive(inclusive_prefix)
        arr[i] = inclusive_prefix;
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", partial_sum);
}

__attribute__((optimize("O2"), noinline))
void test_enter_to_clause(int **dyn_arr, int n) {
    /* Dynamically allocate array for enter data clause */
    *dyn_arr = (int *)malloc(n * sizeof(int));
    if (!*dyn_arr) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = (i * g_seed) % 100;
    }
    
    /* Use enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: (*dyn_arr)[0:n])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for map(always, tofrom: (*dyn_arr)[0:n])
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = (*dyn_arr)[i] * 2 + 1;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: (*dyn_arr)[0:n])
    
    __builtin_printf("Enter data test, dyn_arr[0]=%d\n", (*dyn_arr)[0]);
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Use argv for runtime-dependent seed */
    if (argc > 1) {
        g_seed = atoi(argv[1]);
    }
    
    const int N = 512;
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *dyn_arr = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * 13 + g_seed) % 100;
        arr2[i] = (i * 17 + g_seed) % 100;
    }
    
    volatile int iterations = (g_seed % 3) + 2; /* 2-4 iterations */
    
    for (volatile int iter = 0; iter < iterations; iter++) {
        g_flag = (iter % 2);
        
        /* Call all test functions in sequence */
        test_reduction_temp(arr1, arr2, N);
        test_conditional_temp(arr1, N);
        test_scan_temp(arr2, N);
        test_enter_to_clause(&dyn_arr, N/2);
        
        /* Compute checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + arr1[i]) % 10007;
            checksum = (checksum * 31 + arr2[i]) % 10007;
        }
        if (dyn_arr) {
            for (int i = 0; i < N/2; i++) {
                checksum = (checksum * 31 + dyn_arr[i]) % 10007;
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify seed for next iteration */
        g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output */
    printf("Final arr1[0]=%d, arr2[0]=%d\n", arr1[0], arr2[0]);
    
    free(arr1);
    free(arr2);
    if (dyn_arr) free(dyn_arr);
    
    return 0;
}
