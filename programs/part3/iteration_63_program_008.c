/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int vol_flag1 = 1;
volatile int vol_flag2 = 0;
volatile int vol_bound = 512;
volatile int vol_iter = 3;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temps(int *arr1, int *arr2, int size, int seed) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators on arrays
     * Forces creation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2)
    for (i = 0; i < size; i++) {
        /* Data-dependent computation to inhibit optimization */
        int idx = (i + seed) % size;
        int val1 = arr1[idx];
        int val2 = arr2[(idx * 7) % size];
        
        sum += val1 + val2;
        prod *= (val1 % 10 + 1) * (val2 % 10 + 1);
        max_val = (val1 > max_val) ? val1 : max_val;
        max_val = (val2 > max_val) ? val2 : max_val;
        min_val = (val1 < min_val) ? val1 : min_val;
        min_val = (val2 < min_val) ? val2 : min_val;
        
        /* Cross-update arrays to create dependencies */
        if (i % 3 == 0) {
            arr1[idx] = (arr1[idx] + arr2[idx]) % 1000;
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction temps: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_condition_temps(int *arr, int size, int seed) {
    volatile int cond1 = seed % 2;
    volatile int cond2 = (seed * 3) % 5;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Nested task with if clause */
        #pragma omp task if(cond2 && (tid % 2 == 0))
        {
            for (int i = tid; i < size; i += omp_get_num_threads()) {
                arr[i] = (arr[i] * 3 + 7) % 1000;
            }
        }
        
        #pragma omp taskwait
        
        /* Target teams with if clause */
        #pragma omp target teams if(cond1 || cond2) thread_limit(8)
        {
            #pragma omp distribute parallel for simd
            for (int i = 0; i < size/2; i++) {
                arr[i] = (arr[i] + tid) % 1000;
            }
        }
    }
    
    __builtin_printf("Condition temps processed array[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2")))
void test_scan_temps(int *arr, int size, int seed) {
    int scan_temp = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for simd reduction(inscan, +:scan_temp)
    for (int i = 0; i < size; i++) {
        int val = arr[i] + (seed % 10);
        
        #pragma omp scan exclusive(scan_temp)
        {
            arr[i] = scan_temp;
            scan_temp += val;
        }
    }
    
    /* Inclusive scan variant */
    scan_temp = 0;
    #pragma omp parallel for simd reduction(inscan, +:scan_temp)
    for (int i = 0; i < size; i++) {
        scan_temp += arr[i] + 1;
        
        #pragma omp scan inclusive(scan_temp)
        arr[i] = scan_temp;
    }
    
    __builtin_printf("Scan temps: final scan_temp=%d\n", scan_temp);
}

__attribute__((optimize("O2")))
void test_enter_to_clause(int **dyn_arr, int size) {
    /* Dynamically allocate array for enter data clause */
    *dyn_arr = (int *)malloc(size * sizeof(int));
    if (!*dyn_arr) return;
    
    for (int i = 0; i < size; i++) {
        (*dyn_arr)[i] = i * 3 + 7;
    }
    
    /* OMP enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: (*dyn_arr)[0:size/2])
    
    /* Use the data in target region */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < size/2; i++) {
        (*dyn_arr)[i] *= 2;
    }
    
    #pragma omp target exit data map(from: (*dyn_arr)[0:size/2])
    
    __builtin_printf("Enter to clause: dyn_arr[10]=%d\n", (*dyn_arr)[10]);
}

__attribute__((optimize("O2")))
int main(int argc, char **argv) {
    /* Seed from command line for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    const int base_size = 512;
    int size = vol_bound < 100 ? 100 : (vol_bound > 1000 ? 1000 : vol_bound);
    
    /* Initialize arrays with pseudo-random values */
    int *arr1 = (int *)malloc(size * sizeof(int));
    int *arr2 = (int *)malloc(size * sizeof(int));
    int *dyn_arr = NULL;
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Simple LCG pseudo-random generator */
    unsigned int lcg = seed;
    for (int i = 0; i < size; i++) {
        lcg = lcg * 1103515245 + 12345;
        arr1[i] = (lcg >> 16) % 1000;
        arr2[i] = (lcg >> 8) % 1000;
    }
    
    int checksum = 0;
    
    /* Multiple iterations based on volatile counter */
    for (int iter = 0; iter < vol_iter && iter < 3; iter++) {
        __builtin_printf("\n=== Iteration %d (seed=%d) ===\n", iter, seed + iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temps(arr1, arr2, size, seed + iter);
        
        /* 2. Test condition temporaries */
        test_condition_temps(arr1, size, seed + iter * 7);
        
        /* 3. Test scan temporaries */
        test_scan_temps(arr2, size, seed + iter * 13);
        
        /* 4. Test enter data with to modifier */
        test_enter_to_clause(&dyn_arr, size);
        
        /* Update checksum to prevent elimination */
        for (int i = 0; i < size; i += 32) {
            checksum += arr1[i] + arr2[i];
            if (dyn_arr && i < size/2) {
                checksum += dyn_arr[i];
            }
        }
        
        /* Modify seed for next iteration */
        seed = (seed * 3 + 7) % 100;
    }
    
    /* Final output to prevent dead code elimination */
    __builtin_printf("\nFinal checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    if (dyn_arr) free(dyn_arr);
    
    return 0;
}
