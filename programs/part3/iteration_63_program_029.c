/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
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

/* Function with complex reduction operations to generate _reductemp_ */
__attribute__((optimize("O2")))
void test_reduction_temps(int *arr1, int *arr2, int n, int seed) {
    volatile int v_n = n;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) shared(v_n)
    for (int i = 0; i < v_n; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i + seed) % v_n;
        sum += arr1[idx] + arr2[i % v_n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero for product */
        if (arr2[i] > max_val) max_val = arr2[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-update to create dependencies */
        if (i % 3 == 0) {
            arr1[(i + 1) % v_n] += arr2[i] % 7;
        }
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction temps: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

/* Function to generate _condtemp_ clauses */
__attribute__((optimize("O2")))
void test_condition_temps(int *arr, int n, volatile int cond) {
    volatile int v_n = n;
    volatile int v_cond_local = cond;
    
    /* Multiple if clauses in different OpenMP contexts */
    if (v_flag1) {
        #pragma omp parallel if(v_cond_local) num_threads(4)
        {
            #pragma omp single
            {
                #pragma omp task if(v_cond_local && v_flag2)
                {
                    for (int i = 0; i < v_n/2; i++) {
                        arr[i] *= 2;
                    }
                }
                
                #pragma omp task if(!v_cond_local || v_flag1)
                {
                    for (int i = v_n/2; i < v_n; i++) {
                        arr[i] /= 2;
                    }
                }
            }
        }
    }
    
    /* Target region with if clause */
    #pragma omp target teams if(v_cond_local) map(tofrom:arr[0:n]) thread_limit(8)
    {
        #pragma omp distribute parallel for if(v_cond_local)
        for (int i = 0; i < v_n; i++) {
            arr[i] += i % 5;
        }
    }
    
    __builtin_printf("Condition temps processed array[0]=%d\n", arr[0]);
}

/* Function to generate _scantemp_ clauses (OpenMP 5.0+ scan) */
__attribute__((optimize("O2")))
void test_scan_temps(int *arr, int n) {
    volatile int v_n = n;
    int scan_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < v_n; i++) {
        scan_sum += arr[i];
        #pragma omp scan exclusive(scan_sum)
        arr[i] = scan_sum - arr[i];  /* Exclusive scan result */
    }
    
    int inclusive_sum = 0;
    
    /* Inclusive scan with inscan reduction */
    #pragma omp parallel for reduction(inscan, +:inclusive_sum) \
            private(arr) shared(v_n)
    for (int i = 0; i < v_n; i++) {
        inclusive_sum += arr[i];
        #pragma omp scan inclusive(inclusive_sum)
        arr[i] = inclusive_sum;
    }
    
    __builtin_printf("Scan temps: final sum=%d, arr[last]=%d\n", 
                     inclusive_sum, arr[v_n-1]);
}

/* Function to test OMP_CLAUSE_ENTER with to modifier */
__attribute__((optimize("O2")))
void test_enter_to_clause(int **dyn_arr, int n) {
    volatile int v_n = n;
    
    /* Allocate and initialize dynamic array */
    *dyn_arr = (int*)malloc(v_n * sizeof(int));
    for (int i = 0; i < v_n; i++) {
        (*dyn_arr)[i] = i * i % 100;
    }
    
    /* Use enter data with to clause */
    #pragma omp enter data to(*dyn_arr[0:v_n])
    
    /* Perform computation on device */
    #pragma omp target map(tofrom: (*dyn_arr)[0:v_n])
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < v_n; i++) {
            (*dyn_arr)[i] += 42;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(*dyn_arr[0:v_n])
    
    __builtin_printf("Enter to clause: dyn_arr[0]=%d\n", (*dyn_arr)[0]);
}

/* Main function with runtime-dependent execution */
int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int v_main_cond = (seed % 3 == 0);
    
    const int N = 512;
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *dyn_arr = NULL;
    
    /* Initialize with pseudo-random values */
    srand(seed);
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    int checksum = 0;
    
    /* Execute multiple times with volatile control */
    for (volatile int iter = 0; iter < v_iter; iter++) {
        v_flag1 = (iter % 2 == 0);
        v_flag2 = (seed % 2 == 0);
        v_cond = (iter != 0);
        
        /* 1. Test reduction temporaries */
        test_reduction_temps(arr1, arr2, N, seed + iter);
        
        /* 2. Test condition temporaries */
        test_condition_temps(arr1, N, v_cond);
        
        /* 3. Test scan temporaries */
        test_scan_temps(arr2, N);
        
        /* 4. Test enter data with to clause */
        test_enter_to_clause(&dyn_arr, N/4);
        
        /* Update checksum */
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + arr1[i]) % 1000000007;
            checksum = (checksum * 31 + arr2[i]) % 1000000007;
        }
        if (dyn_arr) {
            for (int i = 0; i < N/4; i++) {
                checksum = (checksum * 31 + dyn_arr[i]) % 1000000007;
            }
            free(dyn_arr);
            dyn_arr = NULL;
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    /* Final output to prevent optimization */
    int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result ^= arr1[i];
        final_result ^= arr2[i];
    }
    __builtin_printf("Final XOR result: %d\n", final_result);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
