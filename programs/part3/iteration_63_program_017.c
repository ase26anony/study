/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses.
 * Compile with: gcc -O2 -fopenmp -fdump-tree-all -std=gnu11 test_omp_clause_printing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Force optimization level and prevent early folding */
__attribute__((optimize("O2")))
void omp_reduction_with_temporaries(int *arr1, int *arr2, int size, volatile int seed) {
    int sum = 0;
    int product = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with array dependencies to force _reductemp_ creation */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) shared(size, seed)
    for (int i = 0; i < size; i++) {
        /* Data-dependent indexing to prevent optimization */
        int idx = (i + seed) % size;
        arr1[idx] = arr2[i] * (i + 1);
        sum += arr1[idx];
        
        /* Avoid product becoming 0 */
        product *= (arr1[idx] != 0) ? (arr1[idx] % 10 + 1) : 1;
        
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Cross-iteration dependency through shared array */
        if (i > 0) {
            arr2[i] += arr1[(i-1) % size] % 7;
        }
    }
    
    __builtin_printf("Reduction results: sum=%d, product=%d, max=%d, min=%d\n", 
                     sum, product, max_val, min_val);
}

__attribute__((optimize("O2")))
void omp_conditional_temporaries(int *arr, int size, volatile int cond_seed) {
    volatile int dynamic_condition = cond_seed % 4;
    
    /* Force _condtemp_ creation with runtime-dependent conditions */
    #pragma omp parallel if(dynamic_condition > 0) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(dynamic_condition > 1) 
            {
                for (int i = 0; i < size/2; i++) {
                    arr[i] *= 2;
                }
            }
            
            #pragma omp task if(dynamic_condition > 2)
            {
                for (int i = size/2; i < size; i++) {
                    arr[i] /= 2;
                }
            }
        }
    }
    
    /* Another conditional region with teams */
    #pragma omp target teams if(dynamic_condition == 0) map(tofrom:arr[0:size])
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < size; i++) {
            arr[i] += i;
        }
    }
    
    __builtin_printf("Conditional execution with seed=%d\n", dynamic_condition);
}

__attribute__((optimize("O2")))
void omp_scan_temporaries(int *arr, int size, volatile int scan_seed) {
    int prefix_sum = 0;
    
    /* Exclusive scan to force _scantemp_ creation */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < size; i++) {
        int val = arr[i] + (i * scan_seed) % 5;
        
        #pragma omp scan exclusive(prefix_sum)
        {
            arr[i] = prefix_sum;
            prefix_sum += val;
        }
    }
    
    /* Inclusive scan variant */
    int running_total = 0;
    #pragma omp parallel for reduction(inscan, +:running_total)
    for (int i = 0; i < size; i++) {
        running_total += arr[i] % 10;
        
        #pragma omp scan inclusive(running_total)
        arr[i] = running_total;
    }
    
    __builtin_printf("Scan operations completed with seed=%d\n", scan_seed);
}

__attribute__((optimize("O2")))
void omp_enter_data_with_to(int size, volatile int data_seed) {
    /* Dynamic allocation for enter data clause */
    int *device_array = (int *)malloc(size * sizeof(int));
    if (!device_array) return;
    
    /* Initialize array with pattern */
    for (int i = 0; i < size; i++) {
        device_array[i] = i * data_seed;
    }
    
    /* Trigger OMP_CLAUSE_ENTER with 'to' modifier */
    #pragma omp target enter data map(to: device_array[0:size])
    
    /* Use the data on device */
    #pragma omp target teams distribute parallel for map(tofrom: device_array[0:size])
    for (int i = 0; i < size; i++) {
        device_array[i] *= 2;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_array[0:size])
    
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += device_array[i];
    }
    
    __builtin_printf("Enter data checksum: %d\n", checksum);
    free(device_array);
}

__attribute__((optimize("O2")))
void nested_combined_constructs(int *arr1, int *arr2, int size, volatile int nest_seed) {
    /* Nested parallel regions with combined constructs */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for reduction(+:arr1[:size]) nowait
        for (int i = 0; i < size; i++) {
            arr1[i] += nest_seed;
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task if(nest_seed > 10)
                {
                    #pragma omp parallel for simd
                    for (int i = 0; i < size; i++) {
                        arr2[i] -= nest_seed;
                    }
                }
            }
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:size], arr2[0:size]) \
            if(nest_seed % 3 == 0)
    for (int i = 0; i < size; i++) {
        arr1[i] = arr1[i] * arr2[i] / (size + 1);
    }
    
    __builtin_printf("Nested constructs with seed=%d\n", nest_seed);
}

int main(int argc, char *argv[]) {
    /* Use argc for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int iterations = (seed % 3) + 2;  /* 2-4 iterations */
    
    const int SIZE = 512;
    int *array1 = (int *)malloc(SIZE * sizeof(int));
    int *array2 = (int *)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (i * 13 + seed) % 100;
        array2[i] = (i * 17 + seed) % 100;
    }
    
    volatile int flags[4] = {1, 1, 1, 1};
    
    /* Multiple iterations to ensure clause processing */
    for (int iter = 0; iter < iterations; iter++) {
        volatile int iter_seed = seed + iter * 7;
        
        __builtin_printf("\n=== Iteration %d (seed=%d) ===\n", iter, iter_seed);
        
        if (flags[0]) {
            omp_reduction_with_temporaries(array1, array2, SIZE, iter_seed);
        }
        
        if (flags[1]) {
            omp_conditional_temporaries(array1, SIZE, iter_seed);
        }
        
        if (flags[2]) {
            omp_scan_temporaries(array2, SIZE, iter_seed);
        }
        
        if (flags[3]) {
            omp_enter_data_with_to(SIZE/4, iter_seed);
        }
        
        /* Nested constructs every other iteration */
        if (iter % 2 == 0) {
            nested_combined_constructs(array1, array2, SIZE, iter_seed);
        }
        
        /* Compute checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < SIZE; i++) {
            checksum += array1[i] ^ array2[i];
        }
        __builtin_printf("Iteration %d checksum: %08x\n", iter, checksum);
        
        /* Modify flags for next iteration */
        flags[iter % 4] = !flags[iter % 4];
    }
    
    free(array1);
    free(array2);
    
    return 0;
}
