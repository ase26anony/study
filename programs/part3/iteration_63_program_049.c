/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Use volatile to prevent optimization of OpenMP conditions */
static volatile int global_seed = 42;
static volatile int dump_trigger = 1;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access
     * Forces creation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access prevents optimization */
        int idx = (i * global_seed) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero product */
        if (arr2[i] > max_val) max_val = arr2[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-update arrays to create dependencies */
        if (i % 3 == 0) {
            arr1[(i + 1) % n] += arr2[i] % 7;
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void test_conditional_temporaries(int *arr, int n, volatile int cond_flag) {
    /* Multiple OpenMP constructs with if clauses
     * Forces creation of _condtemp_ temporaries */
    
    /* Parallel region with runtime-dependent condition */
    #pragma omp parallel if(cond_flag > 0) num_threads(4)
    {
        volatile int local_flag = cond_flag;
        
        /* Task with different condition */
        #pragma omp task if(local_flag < 10) firstprivate(arr, n)
        {
            for (int i = 0; i < n/2; i++) {
                arr[i] += i * global_seed;
            }
        }
        
        /* Another task with complex condition */
        #pragma omp task if(global_seed % 3 == 0) shared(arr)
        {
            for (int i = n/2; i < n; i++) {
                arr[i] -= (i - n/2) * (global_seed % 5);
            }
        }
        
        #pragma omp taskwait
        
        /* Nested parallel for with condition */
        #pragma omp for if(local_flag != 0) schedule(dynamic)
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] % 1000;
        }
    }
    
    /* Target teams with if clause (for offloading contexts) */
    #pragma omp target teams if(cond_flag % 2 == 0) \
            map(tofrom: arr[0:n]) num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for if(cond_flag > 5)
        for (int i = 0; i < n; i++) {
            arr[i] += 1;
        }
    }
    
    __builtin_printf("Conditional test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int n, volatile int scan_flag) {
    int scan_sum = 0;
    
    /* Exclusive scan operation - forces _scantemp_ creation */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            if(scan_flag > 0) schedule(static, 16)
    for (int i = 0; i < n; i++) {
        /* Exclusive scan phase */
        #pragma omp scan exclusive(scan_sum)
        {
            int temp = arr[i];
            arr[i] = scan_sum;
            scan_sum += temp;
        }
    }
    
    /* Reset for inclusive scan test */
    scan_sum = 0;
    int *copy_arr = (int*)malloc(n * sizeof(int));
    memcpy(copy_arr, arr, n * sizeof(int));
    
    /* Inclusive scan with different pattern */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            if(scan_flag < 10)
    for (int i = 0; i < n; i++) {
        scan_sum += copy_arr[i];
        #pragma omp scan inclusive(scan_sum)
        arr[i] += scan_sum;
    }
    
    free(copy_arr);
    __builtin_printf("Scan completed, final sum=%d\n", scan_sum);
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(volatile int size_flag) {
    /* Use enter data with to modifier */
    int data_size = 256 + (size_flag % 128);
    float *device_array = (float*)malloc(data_size * sizeof(float));
    
    /* Initialize array */
    for (int i = 0; i < data_size; i++) {
        device_array[i] = (i * global_seed) / 1000.0f;
    }
    
    /* OMP enter clause with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp enter data to(device_array[0:data_size])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: device_array[0:data_size]) \
            device(0) if(size_flag > 20)
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < data_size; i++) {
            device_array[i] = device_array[i] * 2.0f + 1.0f;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(device_array[0:data_size])
    
    float checksum = 0.0f;
    for (int i = 0; i < data_size; i++) {
        checksum += device_array[i];
    }
    
    __builtin_printf("Enter data test, checksum=%f\n", checksum);
    free(device_array);
}

__attribute__((optimize("O3"), noinline))
void nested_combined_constructs(int *arr1, int *arr2, int n, volatile int nest_flag) {
    /* Complex nested OpenMP structure to stress clause generation */
    
    #pragma omp parallel if(nest_flag > 0) num_threads(2)
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                #pragma omp task if(nest_flag % 3 == 0) \
                        reduction(+:arr1[0:n]) in_reduction(*:arr2[0:n])
                {
                    for (int i = 0; i < n; i++) {
                        arr1[i] += i * 2;
                        arr2[i] *= (i % 7 + 1);
                    }
                }
                
                #pragma omp task if(nest_flag % 2 == 0)
                {
                    #pragma omp parallel for simd reduction(max:arr1[0:n]) \
                            if(nest_flag < 15) simdlen(4)
                    for (int i = 0; i < n; i++) {
                        if (arr1[i] > 1000) arr1[i] = 1000;
                    }
                }
            }
        }
        
        #pragma omp for collapse(2) if(nest_flag != 1)
        for (int i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                arr2[idx] = (arr1[i] + arr2[idx]) % 500;
            }
        }
    }
    
    __builtin_printf("Nested test done, arr1[0]=%d, arr2[0]=%d\n", arr1[0], arr2[0]);
}

int main(int argc, char **argv) {
    /* Use argc to seed variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    global_seed = seed;
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed + 17) % 1000;
        array2[i] = (i * seed * 3 + 23) % 1000;
    }
    
    volatile int loop_counter = (seed % 3) + 2;  /* 2-4 iterations */
    volatile int cond_flag = seed % 20;
    volatile int scan_flag = seed % 15;
    volatile int size_flag = seed % 50;
    volatile int nest_flag = seed % 10;
    
    int total_checksum = 0;
    
    /* Multiple iterations to increase chance of clause generation */
    for (int iter = 0; iter < loop_counter; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, cond_flag + iter);
        
        /* 2. Test conditional temporaries */
        test_conditional_temporaries(array1, N, cond_flag - iter);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, scan_flag + iter);
        
        /* 4. Test enter data clause */
        test_enter_data_clause(size_flag + iter);
        
        /* 5. Test nested combined constructs */
        nested_combined_constructs(array1, array2, N, nest_flag + iter);
        
        /* Update checksum to prevent elimination */
        for (int i = 0; i < N; i++) {
            total_checksum += array1[i] + array2[i];
            total_checksum %= 1000000;
        }
        
        /* Modify volatile flags for next iteration */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
        cond_flag = (cond_flag + 7) % 25;
    }
    
    /* Final output to prevent dead code elimination */
    __builtin_printf("\nFinal checksum: %d\n", total_checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
