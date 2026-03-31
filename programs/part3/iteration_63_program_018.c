/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter to clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int v_flag1 = 1;
volatile int v_flag2 = 0;
volatile int v_bound = 512;
volatile int v_seed = 42;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void complex_reductions(int *arr1, int *arr2, int n, int iter) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2, n, iter)
    for (i = 0; i < n; i++) {
        /* Data-dependent index calculation to inhibit optimization */
        int idx = (i + iter * v_seed) % n;
        idx = idx < 0 ? 0 : idx;
        idx = idx >= n ? n-1 : idx;
        
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero product */
        if (arr2[i] > max_val) max_val = arr2[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Cross-update to create dependencies */
        if (i % 3 == 0) {
            arr1[(i + 1) % n] += arr2[i] % 7;
        }
    }
    
    /* Use results to prevent dead code elimination */
    __builtin_printf("Reductions[%d]: sum=%d, prod=%d, max=%d, min=%d\n", 
                     iter, sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void conditional_parallelism(int *arr, int n, int iter) {
    int i;
    
    /* OMP_CLAUSE__CONDTEMP_ should be generated here */
    #pragma omp parallel if(v_flag1 || iter % 2) num_threads(4)
    {
        /* Nested conditional task */
        #pragma omp single
        {
            #pragma omp task if(v_flag2 || arr[0] > 100)
            {
                for (i = 0; i < n/2; i++) {
                    arr[i] = arr[i] * 2 + iter;
                }
            }
            
            #pragma omp task if(!v_flag2 || arr[n-1] < 50)
            {
                for (i = n/2; i < n; i++) {
                    arr[i] = arr[i] / 2 + iter;
                }
            }
        }
        
        /* Teams with conditional */
        #pragma omp target teams if(iter > 1) num_teams(2) thread_limit(32)
        {
            #pragma omp distribute parallel for simd
            for (i = 0; i < n; i++) {
                arr[i] = arr[i] + (i % 8);
            }
        }
    }
    
    __builtin_printf("Conditional[%d]: arr[0]=%d, arr[%d]=%d\n", 
                     iter, arr[0], n-1, arr[n-1]);
}

__attribute__((optimize("O3")))
void scan_operations(int *arr, int n, int iter) {
    int i;
    int scan_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            private(i) shared(arr, n)
    for (i = 0; i < n; i++) {
        scan_sum += arr[i];
        #pragma omp scan exclusive(scan_sum)
        arr[i] = scan_sum - arr[i];  /* Exclusive prefix sum */
    }
    
    /* Inclusive scan with inscan clause */
    int total = 0;
    #pragma omp parallel for reduction(inscan, +:total) \
            private(i) shared(arr, n, iter)
    for (i = 0; i < n; i++) {
        total += arr[i] + iter;
        #pragma omp scan inclusive(total)
        arr[i] = total;
    }
    
    __builtin_printf("Scan[%d]: final total=%d, arr[%d]=%d\n", 
                     iter, total, n/2, arr[n/2]);
}

__attribute__((optimize("O2")))
void enter_data_with_to(int **ptr_arr, int n, int iter) {
    /* Allocate and initialize data */
    int *device_data = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        device_data[i] = i * i + iter;
    }
    
    /* OMP_CLAUSE_ENTER with 'to' modifier */
    #pragma omp enter data to(device_data[:n])
    
    /* Use target region to ensure enter data is meaningful */
    #pragma omp target map(tofrom: device_data[:n]) if(iter > 0)
    {
        #pragma omp teams distribute parallel for simd
        for (int i = 0; i < n; i++) {
            device_data[i] = device_data[i] * 2 + 1;
        }
    }
    
    /* Copy back and store */
    #pragma omp exit data from(device_data[:n])
    
    *ptr_arr = device_data;
    __builtin_printf("EnterData[%d]: device_data[0]=%d, [%d]=%d\n", 
                     iter, device_data[0], n-1, device_data[n-1]);
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    v_seed = (argc > 1) ? atoi(argv[1]) : 42;
    if (v_seed < 1) v_seed = 42;
    
    const int N = v_bound;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *device_array = NULL;
    
    /* Initialize with pseudo-random values */
    unsigned int lcg = v_seed;
    for (int i = 0; i < N; i++) {
        lcg = lcg * 1103515245 + 12345;
        array1[i] = (lcg >> 16) % 1000;
        array2[i] = (lcg >> 8) % 1000;
    }
    
    /* Volatile loop counter to force multiple instantiations */
    volatile int repeat;
    int checksum = 0;
    
    for (repeat = 0; repeat < 3; repeat++) {
        v_flag1 = (repeat % 2 == 0);
        v_flag2 = (repeat % 3 == 0);
        
        /* 1. Trigger _reductemp_ clause generation */
        complex_reductions(array1, array2, N, repeat);
        
        /* 2. Trigger _condtemp_ clause generation */
        conditional_parallelism(array1, N, repeat);
        
        /* 3. Trigger _scantemp_ clause generation */
        scan_operations(array2, N, repeat);
        
        /* 4. Trigger OMP_CLAUSE_ENTER with 'to' modifier */
        enter_data_with_to(&device_array, N/2, repeat);
        
        /* Update checksum to prevent elimination */
        for (int i = 0; i < N; i += 8) {
            checksum += array1[i] ^ array2[i % N];
            if (device_array && i < N/2) {
                checksum += device_array[i] * 3;
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", repeat, checksum);
        
        /* Free device array if allocated */
        if (device_array) {
            free(device_array);
            device_array = NULL;
        }
    }
    
    /* Final output to ensure all code paths matter */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    __builtin_printf("Final: sum1=%d, sum2=%d, total=%d, checksum=%d\n",
                     array1[N/4], array2[N/4], final_sum, checksum);
    
    free(array1);
    free(array2);
    
    return 0;
}
