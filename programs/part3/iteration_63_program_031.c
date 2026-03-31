/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;
static volatile int dump_trigger = 1;

/* Function attribute to ensure optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators on arrays */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
             reduction(max:max_val) reduction(min:min_val) \
             if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent operations to prevent optimization */
        int idx = (i + global_seed) % n;
        sum += arr1[idx] * (i % 7 + 1);
        prod *= (arr1[idx] % 10 + 1);
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Cross-array dependency */
        arr2[idx] = arr1[idx] + sum % 100;
    }
    
    /* Force output to prevent dead code elimination */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O3"), noinline))
void test_condtemp_clauses(int *arr, int n, volatile int cond) {
    volatile int runtime_cond = cond + global_seed;
    
    /* Multiple if clauses in different contexts */
    #pragma omp parallel if(runtime_cond > 20) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(runtime_cond % 3 == 0)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] *= 2;
                }
            }
            
            #pragma omp task if(runtime_cond % 5 == 0)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] /= 2;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(runtime_cond < 50) \
            map(tofrom:arr[0:n]) num_teams(2)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] += i;
        }
    }
    
    __builtin_printf("Condtemp processed with cond=%d\n", runtime_cond);
}

__attribute__((optimize("O2"), noinline))
void test_scantemp_clauses(int *arr, int n, volatile int mode) {
    int partial_sum = 0;
    
    /* Exclusive scan pattern */
    #pragma omp parallel for reduction(inscan,+:partial_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + (i % 11);
        
        #pragma omp scan exclusive(partial_sum)
        {
            arr[i] = partial_sum;
            partial_sum += val;
        }
    }
    
    /* Inclusive scan with different array */
    int *arr2 = (int*)malloc(n * sizeof(int));
    memcpy(arr2, arr, n * sizeof(int));
    
    int inclusive_sum = 0;
    #pragma omp parallel for reduction(inscan,+:inclusive_sum)
    for (int i = 0; i < n; i++) {
        inclusive_sum += arr2[i] % 13;
        #pragma omp scan inclusive(inclusive_sum)
        arr2[i] = inclusive_sum;
    }
    
    __builtin_printf("Scan results: partial_sum=%d, inclusive_sum=%d\n",
                     partial_sum, inclusive_sum);
    free(arr2);
}

__attribute__((optimize("O2"), noinline))
void test_enter_clause_with_to(volatile int size) {
    /* Dynamically allocate for enter data clause */
    int *device_array = (int*)malloc(size * sizeof(int));
    int *host_array = (int*)malloc(size * sizeof(int));
    
    /* Initialize host array */
    for (int i = 0; i < size; i++) {
        host_array[i] = i * i + global_seed;
    }
    
    /* Use enter data with to clause */
    #pragma omp target enter data map(to: device_array[0:size]) \
            map(to: host_array[0:size])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for \
            map(tofrom: device_array[0:size])
    for (int i = 0; i < size; i++) {
        device_array[i] = host_array[i] * 3 + i;
    }
    
    /* Retrieve results */
    #pragma omp target exit data map(from: device_array[0:size])
    
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += device_array[i];
    }
    
    __builtin_printf("Enter data checksum: %d\n", checksum);
    
    free(device_array);
    free(host_array);
}

__attribute__((optimize("O3"), noinline))
void nested_combined_constructs(int *arr1, int *arr2, int n, volatile int iter) {
    /* Nested parallel regions with combined constructs */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for reduction(+:global_seed) nowait
        for (int i = 0; i < n; i++) {
            arr1[i] = (arr1[i] + iter) % 1000;
            global_seed += arr1[i] % 7;
        }
        
        #pragma omp single
        {
            #pragma omp taskloop grainsize(16)
            for (int i = 0; i < n; i++) {
                arr2[i] = arr1[(i + iter) % n] * 2;
            }
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:n]) map(to: arr2[0:n]) \
            if(iter % 2 == 0)
    for (int i = 0; i < n; i++) {
        arr1[i] += arr2[i % n] / 3;
    }
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    global_seed = seed;
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 17 + seed) % 1000;
        array2[i] = (i * 23 + seed) % 1000;
    }
    
    volatile int control_flags[4] = {1, 0, 1, 0};
    
    /* Multiple iterations to increase instantiation */
    for (int iter = 0; iter < 3; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* Vary control flags each iteration */
        control_flags[0] = (iter + seed) % 3;
        control_flags[1] = (iter * 2 + seed) % 5;
        control_flags[2] = (iter * 3 + seed) % 7;
        control_flags[3] = (iter * 5 + seed) % 11;
        
        /* Test all clause types */
        test_reduction_temporaries(array1, array2, N, control_flags[0]);
        test_condtemp_clauses(array1, N, control_flags[1]);
        test_scantemp_clauses(array2, N, control_flags[2]);
        
        if (iter % 2 == 0) {
            test_enter_clause_with_to(N/4);
        }
        
        nested_combined_constructs(array1, array2, N, iter);
        
        /* Compute checksum to prevent elimination */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum = (checksum * 31 + array1[i]) % 1000000007;
            checksum = (checksum * 37 + array2[i]) % 1000000007;
        }
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
    }
    
    free(array1);
    free(array2);
    
    return 0;
}
