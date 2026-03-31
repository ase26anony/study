/* Test program to trigger printing of internal OpenMP temporary clauses
   and the 'enter' clause with 'to' modifier */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization of OpenMP conditions */
static volatile int global_flag = 1;
static volatile int seed_value = 42;

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access
       Forces generation of _reductemp_ clauses */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) reduction(max:max_val) \
            if(flag > 0)  /* Add conditional to create more complex clause tree */
    for (int i = 0; i < n; i++) {
        /* Data-dependent indexing to prevent optimization */
        int idx = (i * seed_value) % n;
        if (idx < 0) idx = -idx;
        
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero multiplication */
        if (arr2[i] > max_val) {
            max_val = arr2[i];
        }
        
        /* Cross-update to create dependencies */
        if (i % 3 == 0) {
            arr1[(i + 1) % n] += sum % 100;
        }
    }
    
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d\n", 
                     sum, prod, max_val);
}

__attribute__((optimize("O3"), noinline))
void test_conditional_temporaries(int *arr, int n, volatile int cond) {
    /* Force generation of _condtemp_ clauses with volatile conditions */
    volatile int dynamic_cond = cond;
    
    /* Multiple parallel regions with if clauses */
    #pragma omp parallel if(dynamic_cond > 0) num_threads(4)
    {
        #pragma omp single
        {
            /* Nested task with if clause */
            #pragma omp task if(dynamic_cond < 100)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] *= 2;
                }
            }
            
            #pragma omp task if(dynamic_cond > 50)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] /= 2;
                }
            }
        }
    }
    
    /* Target region with if clause - different context */
    #pragma omp target teams if(dynamic_cond != 0) map(tofrom:arr[0:n/4]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for simd
        for (int i = 0; i < n/4; i++) {
            arr[i] += i;
        }
    }
    
    __builtin_printf("Conditional test completed, arr[0]=%d\n", arr[0]);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *arr, int n, volatile int iter) {
    int partial_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ clauses */
    #pragma omp parallel for reduction(inscan, +:partial_sum)
    for (int i = 0; i < n; i++) {
        /* Exclusive scan computation */
        int val = arr[i] + (i * iter);
        #pragma omp scan exclusive(partial_sum)
        arr[i] = partial_sum;
        partial_sum += val;
    }
    
    /* Another scan variant */
    int scan_temp = 0;
    #pragma omp parallel for simd reduction(inscan, +:scan_temp)
    for (int i = 0; i < n/2; i++) {
        int offset = (i + iter) % n;
        #pragma omp scan exclusive(scan_temp)
        arr[offset] += scan_temp;
        scan_temp += offset + 1;
    }
    
    __builtin_printf("Scan completed, final sum=%d, scan_temp=%d\n", 
                     partial_sum, scan_temp);
}

__attribute__((optimize("O2"), noinline))
void test_enter_data_clause(int size, volatile int flag) {
    /* Dynamic allocation for enter data clause */
    int *device_array = (int*)malloc(size * sizeof(int));
    if (!device_array) return;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        device_array[i] = i * seed_value;
    }
    
    /* Use enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_array[0:size]) \
            depend(inout: device_array) if(flag > 0)
    
    /* Use the data in a target region */
    #pragma omp target teams distribute parallel for map(tofrom: device_array[0:size])
    for (int i = 0; i < size; i++) {
        device_array[i] += 1;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_array[0:size])
    
    /* Verify and print */
    int checksum = 0;
    for (int i = 0; i < (size > 10 ? 10 : size); i++) {
        checksum += device_array[i];
    }
    __builtin_printf("Enter data checksum: %d\n", checksum);
    
    free(device_array);
}

__attribute__((optimize("O3"), noinline))
void nested_combined_constructs(int *arr1, int *arr2, int n, volatile int mode) {
    /* Complex nested OpenMP to stress clause generation */
    
    /* Outer parallel with if clause */
    #pragma omp parallel if(mode % 2 == 0) default(none) \
            shared(arr1, arr2, n, mode) firstprivate(seed_value)
    {
        /* Inner reduction with scan */
        #pragma omp for reduction(+:seed_value) schedule(dynamic, 4)
        for (int i = 0; i < n; i++) {
            arr1[i] = (arr1[i] + i) % 1000;
            seed_value += arr1[i] % 17;
        }
        
        /* Barrier with task generation */
        #pragma omp barrier
        
        #pragma omp single
        {
            #pragma omp task if(mode > 1) untied
            {
                /* Another reduction inside task */
                int local_max = arr2[0];
                #pragma omp simd reduction(max:local_max)
                for (int i = 0; i < n; i++) {
                    if (arr2[i] > local_max) local_max = arr2[i];
                    arr2[i] = (arr2[i] * 3) % 999;
                }
                __builtin_printf("Task max: %d\n", local_max);
            }
        }
    }
    
    /* Combined construct */
    #pragma omp target teams distribute parallel for simd \
            map(tofrom: arr1[0:n/2]) if(mode < 5) reduction(+:seed_value)
    for (int i = 0; i < n/2; i++) {
        arr1[i] = arr1[i] * 2 + seed_value;
    }
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    seed_value = (argc > 1) ? atoi(argv[1]) : 42;
    if (seed_value <= 0) seed_value = 42;
    
    const int N = 512;
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed_value + 17) % 1000;
        array2[i] = (i * seed_value * 3 + 23) % 1000;
    }
    
    volatile int control = 1;
    volatile int iter_count = (seed_value % 3) + 2;  /* 2-4 iterations */
    
    /* Multiple iterations to increase chance of clause generation */
    for (int iter = 0; iter < iter_count; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* Vary conditions each iteration */
        global_flag = (iter % 2) ? 1 : 0;
        control = (seed_value + iter) % 100;
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(array1, array2, N, control);
        
        /* 2. Test conditional temporaries */
        test_conditional_temporaries(array1, N, control + iter);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(array2, N, iter);
        
        /* 4. Test enter data clause */
        if (iter % 2 == 0) {
            test_enter_data_clause(N/4, control);
        }
        
        /* 5. Nested and combined constructs */
        nested_combined_constructs(array1, array2, N, iter);
        
        /* Compute checksum to prevent elimination */
        int checksum = 0;
        for (int i = 0; i < N; i += 16) {
            checksum += array1[i] + array2[i];
        }
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify seed for next iteration */
        seed_value = (seed_value * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Final output */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    __builtin_printf("\nFinal result: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
