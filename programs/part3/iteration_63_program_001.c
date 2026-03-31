/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_bound = 512;
volatile int v_iter = 3;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temp(int *arr1, int *arr2, int size, int seed) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2, size, seed)
    for (i = 0; i < size; i++) {
        /* Data-dependent index calculation to inhibit optimization */
        int idx = (i * seed + arr1[i]) % size;
        if (idx < 0) idx = -idx;
        
        sum += arr1[idx] + arr2[i % size];
        prod *= (arr1[i] + 1) % 100 + 1;  /* Avoid zero product */
        
        int temp = arr1[i] ^ arr2[idx % size];
        if (temp > max_val) max_val = temp;
        if (temp < min_val) min_val = temp;
        
        /* Additional computation to create _reductemp_ */
        arr2[i] = (arr1[i] * arr2[i]) % 1000;
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_cond_temp(int *arr, int size, volatile int cond_flag) {
    int i;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond_flag > 0) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond_flag > 1) shared(arr, size)
            {
                for (i = 0; i < size/2; i++) {
                    arr[i] = arr[i] * 2 + 1;
                }
            }
            
            #pragma omp task if(cond_flag > 2) shared(arr, size)
            {
                for (i = size/2; i < size; i++) {
                    arr[i] = arr[i] / 2 + 3;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond_flag > 1) num_teams(2) thread_limit(32) \
            map(tofrom:arr[0:size])
    {
        #pragma omp distribute parallel for
        for (i = 0; i < size; i++) {
            arr[i] = arr[i] + i;
        }
    }
    
    __builtin_printf("Conditional temp test completed, flag=%d\n", cond_flag);
}

__attribute__((optimize("O2")))
void test_scan_temp(int *arr, int size) {
    int i;
    int scan_sum = 0;
    int exclusive_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:exclusive_sum) \
            private(i) shared(arr, size)
    for (i = 0; i < size; i++) {
        exclusive_sum += arr[i];
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] = exclusive_sum - arr[i];  /* Exclusive prefix */
    }
    
    /* Inclusive scan with inscan reduction */
    scan_sum = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i) shared(arr, size)
    for (i = 0; i < size; i++) {
        scan_sum += arr[i];
        #pragma omp scan inclusive(scan_sum)
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2")))
void test_enter_to(int **dyn_arr, int size) {
    /* Dynamic allocation for enter data clause */
    *dyn_arr = (int *)malloc(size * sizeof(int));
    if (!*dyn_arr) return;
    
    for (int i = 0; i < size; i++) {
        (*dyn_arr)[i] = i * i;
    }
    
    /* Use enter data with to modifier */
    #pragma omp target enter data map(to: (*dyn_arr)[0:size])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for \
            map(always, tofrom: (*dyn_arr)[0:size])
    for (int i = 0; i < size; i++) {
        (*dyn_arr)[i] = (*dyn_arr)[i] * 2 + 7;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: (*dyn_arr)[0:size])
    
    __builtin_printf("Enter data test completed, dyn_arr[0]=%d\n", (*dyn_arr)[0]);
}

/* Combined test with nested regions */
__attribute__((optimize("O3")))
void combined_nested_test(int *arr1, int *arr2, int size, int seed) {
    /* Outer parallel region */
    #pragma omp parallel num_threads(2)
    {
        /* Inner reduction loop */
        #pragma omp for reduction(+:seed) nowait
        for (int i = 0; i < size; i++) {
            arr1[i] = (arr1[i] + seed) % 100;
        }
        
        #pragma omp single
        {
            /* Task with if clause inside parallel */
            #pragma omp task if(v_flag1) shared(arr2, size)
            {
                for (int j = 0; j < size; j += 2) {
                    arr2[j] = arr2[j] * 3 - 5;
                }
            }
        }
    }
    
    /* Teams distribute parallel for with reduction */
    #pragma omp target teams distribute parallel for \
            reduction(+:seed) map(tofrom: arr1[0:size], arr2[0:size])
    for (int i = 0; i < size; i++) {
        arr1[i] = arr1[i] + arr2[i];
        arr2[i] = arr1[i] - arr2[i];
    }
}

int main(int argc, char **argv) {
    int size = v_bound;
    int seed = 42;
    
    /* Use argv for runtime variability */
    if (argc > 1) {
        seed = atoi(argv[1]) % 100;
    }
    
    /* Initialize arrays */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *dyn_arr = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < size; i++) {
        arr1[i] = (i * 13 + seed) % 1000;
        arr2[i] = (i * 17 + seed * 2) % 1000;
    }
    
    /* Main test loop with volatile iteration count */
    for (volatile int iter = 0; iter < v_iter; iter++) {
        int checksum = 0;
        
        /* 1. Test reduction temporaries */
        test_reduction_temp(arr1, arr2, size, seed + iter);
        
        /* 2. Test conditional temporaries with volatile flag */
        v_flag1 = (iter % 2 == 0) ? 2 : 1;
        test_cond_temp(arr1, size, v_flag1);
        
        /* 3. Test scan temporaries */
        test_scan_temp(arr2, size);
        
        /* 4. Test enter data with to modifier */
        test_enter_to(&dyn_arr, size/4);
        
        /* 5. Combined nested test */
        combined_nested_test(arr1, arr2, size, seed + iter * 3);
        
        /* Calculate checksum to prevent optimization */
        for (int i = 0; i < size; i++) {
            checksum = (checksum + arr1[i] + arr2[i]) % 1000000;
        }
        if (dyn_arr) {
            for (int i = 0; i < size/4; i++) {
                checksum = (checksum + dyn_arr[i]) % 1000000;
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output */
    printf("Final arr1[0]=%d, arr2[0]=%d\n", arr1[0], arr2[0]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    if (dyn_arr) free(dyn_arr);
    
    return 0;
}
