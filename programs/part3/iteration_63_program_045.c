/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses.
 * Compile with: gcc -O2 -fopenmp -fdump-tree-all -std=gnu11 test_omp_clause_printing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization of OpenMP conditions */
static volatile int global_seed = 42;
static volatile int dump_trigger = 1;

/* Function attribute to ensure optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void complex_reductions(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent indexing to prevent optimization */
        int idx = (i * flag) % n;
        sum += arr1[idx] + arr2[i % 16];
        prod *= (arr1[idx] % 10) + 1;
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Cross-thread data dependency pattern */
        if (i % 32 == 0) {
            #pragma omp atomic
            arr2[i % 16] += arr1[idx] % 7;
        }
    }
    
    /* Force output to prevent dead code elimination */
    __builtin_printf("Reductions: sum=%d prod=%d max=%d min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void conditional_parallelism(volatile int cond1, volatile int cond2, 
                            int *data, int n) {
    /* Nested conditional parallel regions */
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        /* This should generate _condtemp_ clauses */
        #pragma omp single
        {
            #pragma omp task if(cond2 > 0) shared(data)
            {
                for (int i = 0; i < n/2; i++) {
                    data[i] *= 2;
                }
            }
            
            #pragma omp task if(cond1 < cond2) shared(data)
            {
                for (int i = n/2; i < n; i++) {
                    data[i] /= 2;
                }
            }
        }
        
        /* Teams with conditional */
        #pragma omp for
        for (int i = 0; i < n; i++) {
            data[i] += (i % 3) * cond1;
        }
    }
    
    /* Target teams with conditional */
    #pragma omp target teams if(cond1 != cond2) map(tofrom: data[0:n/4]) \
            num_teams(2) thread_limit(8)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n/4; i++) {
            data[i] = data[i] * 3 - 1;
        }
    }
    
    __builtin_printf("Conditional executed: cond1=%d cond2=%d data[0]=%d\n",
                     cond1, cond2, data[0]);
}

__attribute__((optimize("O2"), noinline))
void scan_operations(int *scan_arr, int n, volatile int mode) {
    int sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:sum) \
            if(mode == 1)
    for (int i = 0; i < n; i++) {
        sum += scan_arr[i];
        #pragma omp scan exclusive(sum)
        scan_arr[i] = sum - scan_arr[i];
    }
    
    /* Inclusive scan with different pattern */
    sum = 0;
    #pragma omp parallel for reduction(inscan, +:sum) \
            if(mode == 2)
    for (int i = 0; i < n; i++) {
        sum += scan_arr[i] * 2;
        #pragma omp scan inclusive(sum)
        scan_arr[i] = sum;
    }
    
    /* Combined parallel scan */
    #pragma omp parallel for simd scan(+:sum) \
            if(mode == 3)
    for (int i = 0; i < n; i++) {
        sum += i % 5;
        scan_arr[i] += sum;
    }
    
    __builtin_printf("Scan completed: mode=%d sum=%d arr[10]=%d\n",
                     mode, sum, scan_arr[10]);
}

__attribute__((optimize("O2"), noinline))
void enter_data_operations(volatile int size) {
    int *device_array = NULL;
    int host_size = size > 0 ? size : 512;
    
    /* Allocate and initialize host data */
    int *host_data = (int *)malloc(host_size * sizeof(int));
    for (int i = 0; i < host_size; i++) {
        host_data[i] = i * global_seed;
    }
    
    /* Enter data with to clause - should trigger OMP_CLAUSE_ENTER with to modifier */
    #pragma omp target enter data map(to: host_data[0:host_size/2]) \
            depend(inout: host_data) nowait
    
    /* Another enter with structured block */
    #pragma omp target enter data map(to: host_data[host_size/2:host_size/2]) \
            to(device_array)
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for \
            map(tofrom: host_data[0:host_size]) is_device_ptr(device_array)
    for (int i = 0; i < host_size; i++) {
        host_data[i] = host_data[i] * 2 + 1;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: host_data[0:host_size])
    
    __builtin_printf("Enter data completed: size=%d host_data[0]=%d\n",
                     host_size, host_data[0]);
    
    free(host_data);
}

__attribute__((optimize("O3"), noinline))
void nested_combined_constructs(int *data, int n, volatile int depth) {
    /* Complex nested OpenMP structure */
    #pragma omp parallel num_threads(4) if(depth > 0)
    {
        #pragma omp for reduction(+:global_seed) nowait
        for (int i = 0; i < n; i++) {
            data[i] += global_seed;
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task if(depth > 1) reduction(*:global_seed)
                {
                    for (int j = 0; j < n/4; j++) {
                        data[j] *= 2;
                    }
                }
                
                #pragma omp task if(depth > 2)
                {
                    #pragma omp parallel for simd reduction(max:global_seed) \
                            if(depth > 3)
                    for (int j = n/4; j < n/2; j++) {
                        if (data[j] > global_seed) {
                            global_seed = data[j];
                        }
                    }
                }
            }
        }
        
        #pragma omp for collapse(2) reduction(+:dump_trigger)
        for (int i = 0; i < n/8; i++) {
            for (int j = 0; j < 8; j++) {
                data[i*8 + j] += i * j;
            }
        }
    }
    
    __builtin_printf("Nested constructs: depth=%d data[100]=%d\n",
                     depth, data[100]);
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    global_seed = seed;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *scan_array = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed) % 1000;
        array2[i] = (i * seed * 3) % 500;
        scan_array[i] = (i + seed) % 100;
    }
    
    volatile int iter_count = (seed % 3) + 2;  /* 2-4 iterations */
    volatile int cond_flag1 = seed % 5;
    volatile int cond_flag2 = (seed * 2) % 7;
    volatile int mode_switch = seed % 4;
    
    /* Main loop to execute OpenMP regions multiple times */
    for (int iter = 0; iter < iter_count; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Trigger _reductemp_ clauses */
        complex_reductions(array1, array2, N, iter + 1);
        
        /* 2. Trigger _condtemp_ clauses */
        conditional_parallelism(cond_flag1 + iter, cond_flag2 - iter, 
                              array1, N);
        
        /* 3. Trigger _scantemp_ clauses */
        scan_operations(scan_array, N, mode_switch + iter);
        
        /* 4. Trigger enter with to clause */
        enter_data_operations(N / (iter + 1));
        
        /* 5. Nested combined constructs */
        nested_combined_constructs(array2, N, iter);
        
        /* Modify conditions for next iteration */
        cond_flag1 += array1[10] % 11;
        cond_flag2 += array2[20] % 13;
        mode_switch = (mode_switch * 3) % 4;
    }
    
    /* Final checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += array1[i] + array2[i] + scan_array[i];
        checksum = checksum % 1000000;
    }
    
    __builtin_printf("\nFinal checksum: %d\n", checksum);
    
    free(array1);
    free(array2);
    free(scan_array);
    
    return checksum != 0;
}
