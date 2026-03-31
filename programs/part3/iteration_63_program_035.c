/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause pretty-printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses.
 * Compile with: gcc -O2 -fopenmp -fdump-tree-all -std=gnu11 test_omp_clause_printing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization and constant folding */
static volatile int seed = 42;
static volatile int dump_flag = 1;

/* Function attribute to ensure optimization and tree dumping */
__attribute__((optimize("O2")))
void complex_reductions(int *arr1, int *arr2, int n, volatile int iter) {
    int i;
    int sum = 0;
    int product = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2, n, iter)
    for (i = 0; i < n; i++) {
        /* Data-dependent index calculation to prevent optimization */
        int idx = (i + iter + seed) % n;
        
        /* Multiple reduction operations on different arrays */
        sum += arr1[idx] + arr2[i % 32];
        product *= (arr1[idx] % 10 + 1);
        
        /* Conditional reductions based on runtime values */
        if (arr1[idx] > max_val) {
            max_val = arr1[idx];
        }
        if (arr2[i % 32] < min_val && arr2[i % 32] > 0) {
            min_val = arr2[i % 32];
        }
        
        /* Cross-update between reduction variables */
        arr1[idx] = (arr1[idx] + max_val) % 100;
    }
    
    /* Force output to prevent dead code elimination */
    if (dump_flag) {
        __builtin_printf("Reductions: sum=%d, product=%d, max=%d, min=%d\n", 
                        sum, product, max_val, min_val);
    }
}

__attribute__((optimize("O2")))
void conditional_parallelism(volatile int cond1, volatile int cond2, 
                           int *data, int n) {
    int i;
    
    /* Parallel region with volatile condition - may generate _condtemp_ */
    #pragma omp parallel if(cond1 > 100) num_threads(4)
    {
        /* Nested task with another condition */
        #pragma omp single
        {
            #pragma omp task if(cond2 < 50)
            {
                for (i = 0; i < n/2; i++) {
                    data[i] *= 2;
                }
            }
            
            #pragma omp task if(cond2 >= 50)
            {
                for (i = n/2; i < n; i++) {
                    data[i] /= 2;
                }
            }
        }
        
        /* Teams construct with condition */
        #pragma omp target teams if(cond1 + cond2 > 150) num_teams(2) thread_limit(32)
        {
            #pragma omp distribute parallel for simd
            for (i = 0; i < n; i++) {
                data[i] += i;
            }
        }
    }
    
    if (dump_flag) {
        int checksum = 0;
        for (i = 0; i < n; i++) checksum ^= data[i];
        __builtin_printf("Conditional checksum: %d\n", checksum);
    }
}

__attribute__((optimize("O2")))
void scan_operations(int *scan_data, int n, volatile int offset) {
    int i;
    int prefix_sum = 0;
    
    /* Exclusive scan operation */
    #pragma omp parallel for simd reduction(inscan, +:prefix_sum) \
            private(i) shared(scan_data, n, offset)
    for (i = 0; i < n; i++) {
        int val = scan_data[i] + offset;
        
        #pragma omp scan exclusive(prefix_sum)
        {
            int temp = prefix_sum;
            scan_data[i] = temp;
            prefix_sum += val;
        }
    }
    
    /* Inclusive scan with different array */
    int *temp_arr = (int *)malloc(n * sizeof(int));
    memcpy(temp_arr, scan_data, n * sizeof(int));
    
    prefix_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:prefix_sum)
    for (i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        {
            prefix_sum += temp_arr[i];
            temp_arr[i] = prefix_sum;
        }
    }
    
    if (dump_flag) {
        __builtin_printf("Scan complete, first/last: %d/%d\n", 
                        scan_data[0], scan_data[n-1]);
    }
    
    free(temp_arr);
}

__attribute__((optimize("O2")))
void enter_data_with_to(int size, volatile int init_val) {
    /* Dynamic allocation for enter data clause */
    int *device_data = (int *)malloc(size * sizeof(int));
    struct DataStruct {
        int *ptr;
        int len;
    } data_struct;
    
    /* Initialize data */
    for (int i = 0; i < size; i++) {
        device_data[i] = i + init_val;
    }
    data_struct.ptr = device_data;
    data_struct.len = size;
    
    /* Use enter data with to clause - should trigger OMP_CLAUSE_ENTER with to modifier */
    #pragma omp target enter data map(to: device_data[0:size]) \
            map(to: data_struct)
    
    /* Perform some operations on device */
    #pragma omp target teams distribute parallel for \
            map(always, tofrom: device_data[0:size])
    for (int i = 0; i < size; i++) {
        device_data[i] *= 2;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_data[0:size]) \
            map(from: data_struct)
    
    if (dump_flag) {
        int sum = 0;
        for (int i = 0; i < size && i < 10; i++) {
            sum += device_data[i];
        }
        __builtin_printf("Enter data result (partial sum): %d\n", sum);
    }
    
    free(device_data);
}

__attribute__((optimize("O3")))
void nested_combined_constructs(int *arr, int n, volatile int depth) {
    /* Complex nested OpenMP structure */
    #pragma omp parallel num_threads(2) if(depth > 0)
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                #pragma omp taskloop grainsize(16) reduction(+:arr[0:n/2])
                for (int i = 0; i < n/2; i++) {
                    arr[i] += depth;
                }
            }
        }
        
        #pragma omp for collapse(2) reduction(*:arr[n/2:n/2])
        for (int i = n/2; i < n; i++) {
            for (int j = 0; j < 2; j++) {
                arr[i] *= (i + j + depth);
            }
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr[0:n]) if(depth % 2 == 0)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] % 1000;
    }
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    seed = (argc > 1) ? atoi(argv[1]) : 42;
    const int N = 512;
    volatile int iterations = 3;
    volatile int cond_var1, cond_var2;
    
    /* Initialize arrays with pseudo-random values */
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *scan_array = (int *)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17 + seed) % 100;
        array2[i] = (i * 23 + seed * 2) % 100;
        scan_array[i] = (i * 29 + seed * 3) % 100;
    }
    
    /* Runtime-dependent conditions */
    cond_var1 = seed % 200;
    cond_var2 = (seed * 3) % 100;
    
    /* Multiple iterations to increase chance of clause generation */
    for (volatile int iter = 0; iter < iterations; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Trigger _reductemp_ clauses */
        complex_reductions(array1, array2, N, iter);
        
        /* 2. Trigger _condtemp_ clauses */
        conditional_parallelism(cond_var1 + iter, cond_var2 - iter, array1, N);
        
        /* 3. Trigger _scantemp_ clauses */
        scan_operations(scan_array, N, iter * 10);
        
        /* 4. Trigger enter data with to clause */
        enter_data_with_to(256, seed + iter);
        
        /* 5. Nested and combined constructs */
        nested_combined_constructs(array2, N, iter);
        
        /* Calculate checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum ^= array1[i] ^ array2[i] ^ scan_array[i];
        }
        __builtin_printf("Iteration %d checksum: %08x\n", iter, checksum);
        
        /* Modify conditions for next iteration */
        cond_var1 += 50;
        cond_var2 += 25;
    }
    
    /* Final output to ensure all code paths are used */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + scan_array[i];
    }
    __builtin_printf("\nFinal sum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    free(scan_array);
    
    return 0;
}
