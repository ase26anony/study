/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;
static volatile int dump_trigger = 1;

/* Function attribute to force optimization level */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2, flag)
    for (i = 0; i < n; i++) {
        /* Data-dependent indexing to prevent optimization */
        int idx = (i + flag) % n;
        sum += arr1[idx] + arr2[i % 16];
        
        /* Avoid multiplication by zero */
        if (arr1[idx] != 0) {
            prod *= (arr1[idx] > 0 ? arr1[idx] : 1);
        }
        
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Cross-thread dependency simulation */
        if (i % 32 == 0 && flag) {
            #pragma omp atomic
            arr2[i % 16] += 1;
        }
    }
    
    /* Force tree dump with non-optimizable printf */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_condition_temporaries(int *arr, int n, volatile int cond_flag) {
    volatile int dynamic_condition = cond_flag;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(dynamic_condition > 0) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Nested task with if clause */
        #pragma omp task if(tid % 2 == 0 && dynamic_condition)
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] = arr[i] * 2 + tid;
            }
        }
        
        #pragma omp taskwait
        
        /* Teams with if clause */
        #pragma omp target teams if(dynamic_condition > 1) thread_limit(8)
        {
            #pragma omp distribute parallel for simd
            for (int i = 0; i < n/2; i++) {
                arr[i] += i;
            }
        }
    }
    
    __builtin_printf("Condition test completed with flag=%d\n", dynamic_condition);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int scan_type) {
    int partial_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:partial_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % 3);
        
        #pragma omp scan exclusive(partial_sum)
        {
            arr[i] = partial_sum;
            partial_sum += val;
        }
    }
    
    /* Inclusive scan with different array */
    int *arr2 = (int *)malloc(n * sizeof(int));
    memcpy(arr2, arr, n * sizeof(int));
    
    int inclusive_sum = 0;
    #pragma omp parallel for reduction(inscan, +:inclusive_sum)
    for (int i = 0; i < n; i++) {
        inclusive_sum += arr2[i];
        
        #pragma omp scan inclusive(inclusive_sum)
        arr2[i] = inclusive_sum;
    }
    
    __builtin_printf("Scan results: partial_sum=%d, inclusive_sum=%d\n", 
                     partial_sum, inclusive_sum);
    free(arr2);
}

__attribute__((optimize("O2")))
void test_enter_clause(volatile int use_enter) {
    if (!use_enter) return;
    
    int *device_array = (int *)malloc(256 * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        device_array[i] = i * global_seed;
    }
    
    /* OMP enter data with to clause - triggers OMP_CLAUSE_ENTER with to modifier */
    #pragma omp target enter data map(to: device_array[0:256])
    
    /* Use the array in target region */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < 256; i++) {
        device_array[i] = device_array[i] * 2 + 1;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_array[0:256])
    
    __builtin_printf("Enter clause test: device_array[100]=%d\n", device_array[100]);
    free(device_array);
}

__attribute__((optimize("O2")))
void combined_omp_test(int *data, int n, volatile int iter) {
    /* Nested parallel regions with mixed clauses */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                /* Reduction inside nested parallel region */
                #pragma omp taskloop reduction(+:global_seed) grainsize(8)
                for (int i = 0; i < n/2; i++) {
                    global_seed += data[i] % 7;
                }
            }
        }
        
        #pragma omp barrier
        
        /* SIMD with reduction */
        #pragma omp for simd reduction(+:iter)
        for (int i = n/2; i < n; i++) {
            data[i] = data[i] ^ iter;
            iter++;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    volatile int flag1 = seed % 3;
    volatile int flag2 = seed % 5;
    volatile int flag3 = seed % 7;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed + 17) % 1000;
        array2[i] = (i * seed * 3 + 23) % 500;
    }
    
    /* Multiple iterations to increase compiler processing */
    for (volatile int iter = 0; iter < 3; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, flag1 + iter);
        
        /* 2. Test condition temporaries */
        test_condition_temporaries(array1, N, flag2 + iter);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, flag3 + iter);
        
        /* 4. Test enter clause with to modifier */
        test_enter_clause((iter % 2) == 0);
        
        /* 5. Combined test */
        combined_omp_test(array1, N, iter);
        
        /* Calculate checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + array1[i]) % 1000000;
            checksum = (checksum * 17 + array2[i]) % 1000000;
        }
        __builtin_printf("Checksum after iteration %d: %d\n", iter, checksum);
    }
    
    free(array1);
    free(array2);
    
    return 0;
}
