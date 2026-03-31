/* Test program to trigger OpenMP internal clause pretty-printing */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent optimization */
volatile int g_flag = 1;
volatile int g_seed = 42;

/* Function with complex reduction operations to generate _reductemp_ */
__attribute__((optimize("O2")))
void test_reduction_temps(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) if(flag)
    for (int i = 0; i < n; i++) {
        /* Data-dependent index calculation */
        int idx = (i * g_seed) % n;
        if (idx < 0) idx = -idx;
        
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero product */
        if (arr1[i] > max_val) max_val = arr1[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Additional complexity to prevent optimization */
        if (g_flag && (i % 7 == 0)) {
            arr2[i] = (arr2[i] * 3) % 100;
        }
    }
    
    __builtin_printf("Reduction temps: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

/* Function to generate _condtemp_ clauses */
__attribute__((optimize("O2")))
void test_cond_temps(int *arr, int n, volatile int flag1, volatile int flag2) {
    /* Multiple if clauses in different contexts */
    #pragma omp parallel if(flag1) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(flag2)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] = arr[i] * 2 + g_seed;
                }
            }
            
            #pragma omp task if(flag1 || flag2)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] = arr[i] / 2 + g_seed;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(flag1) map(tofrom:arr[0:n/4]) num_teams(2)
    {
        #pragma omp distribute parallel for if(flag2)
        for (int i = 0; i < n/4; i++) {
            arr[i] = arr[i] + i;
        }
    }
    
    __builtin_printf("Cond temps processed array[0]=%d\n", arr[0]);
}

/* Function to generate _scantemp_ clauses (OpenMP 5.0+ scan) */
__attribute__((optimize("O2")))
void test_scan_temps(int *arr, int n, volatile int flag) {
    int scan_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % 3);
        
        #pragma omp scan exclusive(scan_sum)
        {
            arr[i] = scan_sum + val;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan in separate region */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum) if(flag)
    for (int i = 0; i < n/2; i++) {
        scan_sum += arr[i] % 5;
        
        #pragma omp scan inclusive(scan_sum)
        arr[i] += scan_sum;
    }
    
    __builtin_printf("Scan temps: final sum=%d, arr[10]=%d\n", scan_sum, arr[10]);
}

/* Function to test OMP_CLAUSE_ENTER with 'to' modifier */
__attribute__((optimize("O2")))
void test_enter_to_clause(int **ptr, int n, volatile int flag) {
    /* Allocate and initialize data */
    *ptr = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        (*ptr)[i] = i * g_seed + flag;
    }
    
    /* Use enter data with to clause */
    #pragma omp target enter data map(to:(*ptr)[0:n]) if(flag)
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for map(always, tofrom:(*ptr)[0:n])
    for (int i = 0; i < n; i++) {
        (*ptr)[i] = (*ptr)[i] * 2 + 1;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from:(*ptr)[0:n])
    
    __builtin_printf("Enter to clause: ptr[0]=%d, ptr[%d]=%d\n", 
                     (*ptr)[0], n-1, (*ptr)[n-1]);
}

/* Main function with varied execution paths */
int main(int argc, char **argv) {
    /* Use argc to create runtime-dependent behavior */
    volatile int iter_flag = (argc > 1) ? atoi(argv[1]) % 3 : 2;
    volatile int cond_flag1 = (argc > 2) ? atoi(argv[2]) % 2 : 1;
    volatile int cond_flag2 = (argc > 3) ? atoi(argv[3]) % 2 : 0;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *dynamic_array = NULL;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17 + g_seed) % 1000;
        array2[i] = (i * 23 + g_seed) % 1000;
    }
    
    int checksum = 0;
    
    /* Multiple iterations to increase code generation */
    for (int iter = 0; iter < iter_flag + 1; iter++) {
        g_seed = (g_seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Test reduction temporaries */
        test_reduction_temps(array1, array2, N, iter % 2);
        
        /* Test condition temporaries */
        test_cond_temps(array1, N, cond_flag1 ^ (iter % 2), cond_flag2 ^ (iter % 2));
        
        /* Test scan temporaries */
        test_scan_temps(array2, N, (iter + cond_flag1) % 2);
        
        /* Test enter with to clause */
        test_enter_to_clause(&dynamic_array, N/4, iter % 2);
        
        /* Update checksum to prevent dead code elimination */
        for (int i = 0; i < N; i += 16) {
            checksum += array1[i] + array2[i];
            if (dynamic_array && i < N/4) {
                checksum += dynamic_array[i % (N/4)];
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    /* Final output to ensure all code paths are used */
    printf("Final results: array1[0]=%d, array2[0]=%d, checksum=%d\n",
           array1[0], array2[0], checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    if (dynamic_array) free(dynamic_array);
    
    return 0;
}
