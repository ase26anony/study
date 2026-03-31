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
    int sum = 0;
    int prod = 1;
    int max_val = arr1[0];
    int min_val = arr1[0];
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr2) if(flag > 0)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access to inhibit optimization */
        int idx = (i * global_seed) % n;
        sum += arr1[idx] + (flag & 1);
        prod *= (arr1[idx] % 10 + 1);
        if (arr1[idx] > max_val) max_val = arr1[idx];
        if (arr1[idx] < min_val) min_val = arr1[idx];
        
        /* Modify arr2 in a way that might create _reductemp_ */
        arr2[i] = arr1[idx] * (i % 7 + 1);
    }
    
    /* Force tree dump by using builtin printf */
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_condtemp_clauses(int *arr, int n, volatile int cond) {
    /* Use runtime-dependent condition for if clause */
    volatile int dynamic_cond = cond + global_seed;
    
    /* Multiple constructs with if clauses to generate _condtemp_ */
    #pragma omp parallel if(dynamic_cond > 25) num_threads(4)
    {
        #pragma omp single
        {
            #pragma omp task if(dynamic_cond % 3 == 0)
            {
                for (int i = 0; i < n/2; i++) {
                    arr[i] += i * 2;
                }
            }
            
            #pragma omp task if(dynamic_cond % 5 == 0)
            {
                for (int i = n/2; i < n; i++) {
                    arr[i] -= i;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(dynamic_cond < 100) map(tofrom:arr[0:n]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 2 + 1;
        }
    }
    
    __builtin_printf("Condtemp test complete, cond=%d\n", dynamic_cond);
}

__attribute__((optimize("O2")))
void test_scantemp_clauses(int *arr, int n) {
    int scan_sum = 0;
    int exclusive_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < n; i++) {
        scan_sum += arr[i];
        #pragma omp scan exclusive(scan_sum)
        arr[i] = scan_sum - arr[i];  /* Exclusive prefix */
    }
    
    /* Another scan with inscan clause */
    int temp_arr[512];
    #pragma omp parallel for reduction(inscan, +:exclusive_sum) \
            if(dump_trigger > 0)
    for (int i = 0; i < n; i++) {
        exclusive_sum += i;
        #pragma omp scan exclusive(exclusive_sum)
        temp_arr[i] = exclusive_sum;
    }
    
    __builtin_printf("Scan results: scan_sum=%d, exclusive_sum=%d\n", 
                     scan_sum, exclusive_sum);
}

__attribute__((optimize("O2")))
void test_enter_clause_with_to(int **ptr, int n) {
    /* Allocate and initialize data */
    *ptr = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        (*ptr)[i] = i * global_seed;
    }
    
    /* Use enter data with to clause - should trigger OMP_CLAUSE_ENTER with to modifier */
    #pragma omp target enter data map(to: (*ptr)[0:n])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for map(tofrom: (*ptr)[0:n])
    for (int i = 0; i < n; i++) {
        (*ptr)[i] = (*ptr)[i] * 3 + 7;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: (*ptr)[0:n])
    
    __builtin_printf("Enter clause test: ptr[0]=%d, ptr[%d]=%d\n", 
                     (*ptr)[0], n-1, (*ptr)[n-1]);
}

__attribute__((optimize("O2")))
void nested_combined_constructs(int *arr1, int *arr2, int n, volatile int iter) {
    /* Nested parallel regions with combined constructs */
    #pragma omp parallel if(iter % 2 == 0)
    {
        #pragma omp for reduction(+:global_seed) nowait
        for (int i = 0; i < n; i++) {
            arr1[i] += arr2[i] + iter;
            global_seed += (arr1[i] % 5);
        }
        
        #pragma omp single
        {
            #pragma omp taskloop if(iter > 1) grainsize(16)
            for (int i = 0; i < n; i += 2) {
                arr2[i] = arr1[i] * arr1[i+1];
            }
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:n]) if(iter < 5) \
            num_teams(2) num_threads(4)
    for (int i = 0; i < n; i++) {
        arr1[i] = (arr1[i] * 2) % 1000;
    }
    
    __builtin_printf("Nested test iteration %d complete\n", iter);
}

int main(int argc, char **argv) {
    /* Use argc to seed variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    global_seed = seed;
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *dynamic_array = NULL;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i * seed + 17) % 100;
        array2[i] = (i * seed * 3 + 23) % 100;
    }
    
    volatile int control_flag = seed % 10;
    volatile int iterations = 3;
    
    /* Multiple iterations to increase compiler processing */
    for (int iter = 0; iter < iterations; iter++) {
        dump_trigger = iter + 1;
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(array1, array2, N, control_flag + iter);
        
        /* Test 2: Conditional temporaries */
        test_condtemp_clauses(array1, N, control_flag * (iter + 1));
        
        /* Test 3: Scan temporaries */
        test_scantemp_clauses(array2, N);
        
        /* Test 4: Enter clause with to modifier */
        test_enter_clause_with_to(&dynamic_array, N/2);
        
        /* Test 5: Nested and combined constructs */
        nested_combined_constructs(array1, array2, N, iter);
        
        /* Compute checksum to prevent dead code elimination */
        int checksum = 0;
        for (int i = 0; i < N; i++) {
            checksum += array1[i] + array2[i];
        }
        if (dynamic_array) {
            for (int i = 0; i < N/2; i++) {
                checksum += dynamic_array[i];
            }
        }
        
        __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
        
        /* Modify control flow for next iteration */
        control_flag += checksum % 7;
    }
    
    /* Final output */
    printf("Final values: array1[0]=%d, array2[0]=%d\n", array1[0], array2[0]);
    
    /* Cleanup */
    free(array1);
    free(array2);
    if (dynamic_array) {
        free(dynamic_array);
    }
    
    return 0;
}
