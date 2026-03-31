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
volatile int v_iter = 2;
volatile int v_seed = 42;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void complex_reductions(int *arr1, int *arr2, int n, int seed) {
    int i;
    volatile int v_idx;
    
    /* Multiple complex reductions that may generate _reductemp_ */
    #pragma omp parallel for reduction(+:arr1[0:n]) reduction(*:arr2[0:n]) \
            private(v_idx) shared(seed)
    for (i = 0; i < n; i++) {
        v_idx = (i + seed) % n;  /* Volatile-dependent index */
        
        /* Data-dependent operations to inhibit optimization */
        arr1[v_idx] += (i * seed) % 100;
        arr2[v_idx] *= ((i + seed) % 10) + 1;
        
        /* Additional reduction-like operation */
        if (arr1[v_idx] > 1000) {
            arr1[v_idx] = 1000;
        }
    }
    
    /* Another reduction with max operator */
    int max_val = INT_MIN;
    #pragma omp parallel for reduction(max:max_val)
    for (i = 0; i < n; i++) {
        if (arr1[i] > max_val) {
            max_val = arr1[i];
        }
    }
    
    __builtin_printf("Reduction max: %d\n", max_val);
}

__attribute__((optimize("O2")))
void conditional_clauses(int *arr, int n, volatile int cond) {
    int i;
    
    /* OMP_CLAUSE__CONDTEMP_ generation with volatile condition */
    #pragma omp parallel if(cond) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(cond > 0)  /* Another conditional */
            {
                for (i = 0; i < n/2; i++) {
                    arr[i] += i;
                }
            }
            
            #pragma omp task if(cond <= 0)  /* Different condition */
            {
                for (i = n/2; i < n; i++) {
                    arr[i] -= i;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(cond) map(tofrom:arr[0:n]) num_teams(2)
    {
        #pragma omp distribute parallel for
        for (i = 0; i < n; i++) {
            arr[i] = arr[i] * 2;
        }
    }
    
    __builtin_printf("Conditional processed: %d\n", cond);
}

__attribute__((optimize("O2")))
void scan_operations(int *arr, int n, int seed) {
    int i;
    int scan_temp = 0;
    
    /* Exclusive scan - may generate _scantemp_ */
    #pragma omp parallel for private(i) reduction(inscan, +:scan_temp)
    for (i = 0; i < n; i++) {
        /* Exclusive scan phase */
        #pragma omp scan exclusive(scan_temp)
        {
            int temp = arr[i];
            arr[i] = scan_temp;
            scan_temp += temp;
        }
    }
    
    /* Another scan variant */
    int prefix_sum = 0;
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (i = 0; i < n; i++) {
        int val = (arr[i] + seed) % 100;
        #pragma omp scan inclusive(prefix_sum)
        arr[i] = prefix_sum;
        prefix_sum += val;
    }
    
    __builtin_printf("Scan result[0]: %d\n", arr[0]);
}

__attribute__((optimize("O2")))
void enter_data_with_to(int **ptr_arr, int n) {
    /* Allocate and initialize data */
    *ptr_arr = (int *)malloc(n * sizeof(int));
    if (!*ptr_arr) return;
    
    for (int i = 0; i < n; i++) {
        (*ptr_arr)[i] = i * v_seed;
    }
    
    /* OMP_CLAUSE_ENTER with to modifier */
    #pragma omp enter data to(*ptr_arr[0:n])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: (*ptr_arr)[0:n])
    {
        for (int i = 0; i < n; i++) {
            (*ptr_arr)[i] += 1000;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(*ptr_arr[0:n])
    
    __builtin_printf("Enter data processed, first: %d\n", (*ptr_arr)[0]);
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    const int N = 512;
    int *array1, *array2;
    int *dynamic_array = NULL;
    int checksum = 0;
    int i, iter;
    
    /* Use argc for runtime variability */
    v_seed = (argc > 1) ? atoi(argv[1]) : 12345;
    if (v_seed <= 0) v_seed = 42;
    
    /* Initialize arrays */
    array1 = (int *)malloc(N * sizeof(int));
    array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        array1[i] = (i * v_seed) % 1000;
        array2[i] = ((i + 1) * v_seed) % 100 + 1;  /* Avoid zero for multiplication */
    }
    
    /* Multiple iterations based on volatile counter */
    for (iter = 0; iter < v_iter; iter++) {
        v_flag1 = (iter % 2 == 0) ? 1 : 0;
        v_flag2 = (v_seed % 3) + iter;
        
        __builtin_printf("\n=== Iteration %d (seed: %d) ===\n", iter, v_seed);
        
        /* 1. Trigger _reductemp_ generation */
        complex_reductions(array1, array2, N, v_seed);
        
        /* 2. Trigger _condtemp_ generation */
        conditional_clauses(array1, N, v_flag1);
        
        /* 3. Trigger _scantemp_ generation */
        scan_operations(array2, N, v_seed);
        
        /* 4. Trigger enter with to modifier */
        enter_data_with_to(&dynamic_array, N/4);
        
        /* Update checksum to prevent dead code elimination */
        for (i = 0; i < N; i++) {
            checksum += array1[i] + array2[i];
        }
        if (dynamic_array) {
            for (i = 0; i < N/4; i++) {
                checksum += dynamic_array[i];
            }
        }
        
        __builtin_printf("Checksum after iteration %d: %d\n", iter, checksum);
        
        /* Modify seed for next iteration */
        v_seed = (v_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output to ensure all code paths are used */
    __builtin_printf("\nFinal results:\n");
    __builtin_printf("array1[0]=%d, array1[%d]=%d\n", 
                     array1[0], N-1, array1[N-1]);
    __builtin_printf("array2[0]=%d, array2[%d]=%d\n", 
                     array2[0], N-1, array2[N-1]);
    
    if (dynamic_array) {
        __builtin_printf("dynamic_array[0]=%d\n", dynamic_array[0]);
        free(dynamic_array);
    }
    
    free(array1);
    free(array2);
    
    return 0;
}
