/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal pretty-printing of OpenMP clauses
 * Compile with: gcc -O2 -fopenmp -fdump-tree-all -std=gnu11 test_omp_clause_printing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_bound = 512;
volatile int v_iter = 2;

/* Function to generate runtime-dependent values */
static int runtime_value(int seed) {
    static volatile int counter = 0;
    return (seed * 1103515245 + 12345) ^ counter++;
}

/* 1. Function to trigger _reductemp_ clause generation */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, int seed) {
    volatile int local_flag = v_flag1;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(local_flag)
    for (int i = 0; i < n; i++) {
        /* Data-dependent indexing to prevent optimization */
        int idx = (i + seed) % n;
        if (local_flag && arr1[idx] > 0) {
            sum += arr1[idx] * (i % 7 + 1);
            prod *= (arr1[idx] % 10 + 1);
        }
        
        /* Conditional reduction updates */
        if (arr2[i] % 3 == 0) {
            #pragma omp atomic update
            max_val = (arr2[i] > max_val) ? arr2[i] : max_val;
        } else {
            #pragma omp atomic update
            min_val = (arr2[i] < min_val) ? arr2[i] : min_val;
        }
        
        /* Cross-array dependency */
        arr2[i] += arr1[(i * 17) % n] % 5;
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

/* 2. Function to trigger _condtemp_ clause generation */
__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n, int seed) {
    volatile int cond1 = (seed % 7) > 3;
    volatile int cond2 = v_flag2;
    
    /* Multiple if clauses in different OpenMP constructs */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond2 || (seed % 11 == 0))
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] *= 2;
                }
            }
            
            #pragma omp task if(!cond2 && (seed % 13 != 0))
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] /= 2;
                }
            }
        }
        
        /* Nested parallel region with if clause */
        #pragma omp for if(cond1 && cond2)
        for (int i = 0; i < n; i++) {
            arr[i] += runtime_value(i);
        }
    }
    
    /* Target construct with if clause */
    #pragma omp target teams if(cond1) map(tofrom:arr[0:n]) num_teams(2)
    {
        #pragma omp distribute parallel for if(cond2)
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] % 100;
        }
    }
    
    __builtin_printf("Conditional temporaries processed array[0]=%d\n", arr[0]);
}

/* 3. Function to trigger _scantemp_ clause generation */
__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, int seed) {
    int scan_sum = 0;
    int exclusive_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        scan_sum += arr[i];
        #pragma omp scan exclusive(scan_sum)
        arr[i] = scan_sum - arr[i];  // Exclusive prefix sum
    }
    
    /* Inclusive scan with inscan directive */
    int temp_arr[512];
    memcpy(temp_arr, arr, n * sizeof(int));
    
    #pragma omp parallel for reduction(inscan, +:exclusive_sum) \
            private(scan_sum)
    for (int i = 0; i < n; i++) {
        scan_sum = temp_arr[i];
        #pragma omp scan inclusive(scan_sum)
        exclusive_sum += scan_sum;
        arr[i] = exclusive_sum;
    }
    
    __builtin_printf("Scan results: final_sum=%d, arr[%d]=%d\n", 
                     exclusive_sum, n-1, arr[n-1]);
}

/* 4. Function to trigger OMP_CLAUSE_ENTER with 'to' modifier */
__attribute__((optimize("O2")))
void test_enter_clause(int **ptr_arr, int n, int seed) {
    /* Dynamically allocate array for enter data clause */
    int *dynamic_arr = (int *)malloc(n * sizeof(int));
    if (!dynamic_arr) return;
    
    /* Initialize with runtime-dependent values */
    for (int i = 0; i < n; i++) {
        dynamic_arr[i] = runtime_value(i + seed);
    }
    
    /* Use enter data with to clause */
    #pragma omp target enter data map(to:dynamic_arr[0:n]) depend(out:dynamic_arr)
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for map(tofrom:dynamic_arr[0:n])
    for (int i = 0; i < n; i++) {
        dynamic_arr[i] = dynamic_arr[i] * 3 + i;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from:dynamic_arr[0:n]) depend(in:dynamic_arr)
    
    /* Store result */
    *ptr_arr = dynamic_arr;
    
    __builtin_printf("Enter clause processed: dynamic_arr[0]=%d\n", dynamic_arr[0]);
}

/* Main function with volatile control flow */
int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int n = v_bound;
    
    /* Initialize arrays with pseudo-random values */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    int *dynamic_result = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < n; i++) {
        arr1[i] = runtime_value(i + seed) % 1000;
        arr2[i] = runtime_value(i * 2 + seed) % 1000;
    }
    
    int checksum = 0;
    volatile int iterations = v_iter;
    
    /* Multiple iterations to increase clause instantiation */
    for (int iter = 0; iter < iterations; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(arr1, arr2, n, seed + iter);
        
        /* 2. Test conditional temporaries */
        test_conditional_temporaries(arr1, n, seed + iter * 3);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(arr2, n, seed + iter * 7);
        
        /* 4. Test enter clause with to modifier */
        test_enter_clause(&dynamic_result, n/2, seed + iter * 11);
        
        /* Update checksum with array values */
        for (int i = 0; i < n; i += 16) {
            checksum ^= arr1[i] + arr2[i % n];
        }
        
        if (dynamic_result) {
            for (int i = 0; i < n/2; i += 8) {
                checksum ^= dynamic_result[i];
            }
            free(dynamic_result);
            dynamic_result = NULL;
        }
        
        /* Modify volatile flags for next iteration */
        v_flag1 = !v_flag1;
        v_flag2 = (iter % 3 == 0);
    }
    
    /* Final output to prevent optimization */
    __builtin_printf("\nFinal checksum: 0x%08x\n", checksum);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
