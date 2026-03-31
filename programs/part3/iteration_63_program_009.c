/* test_omp_clause_printing.c
 * Designed to trigger GCC's internal OpenMP clause printing logic
 * for _reductemp_, _condtemp_, _scantemp_, and enter with to modifier
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_flag = 1;
volatile int g_loop_counter = 3;
volatile int g_seed = 42;

/* Function to generate runtime-dependent values */
static int runtime_value(int base) {
    /* Simple LCG to create runtime-dependent values */
    g_seed = (1103515245 * g_seed + 12345) & 0x7fffffff;
    return base + (g_seed % 100);
}

/* Complex reduction operations to trigger _reductemp_ generation */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(arr1, arr2)
    for (i = 0; i < n; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i + runtime_value(0)) % n;
        sum += arr1[idx] + arr2[i % n];
        prod *= (arr1[i] % 10 + 1);  /* Avoid overflow but keep computation */
        if (arr2[i] > max_val) max_val = arr2[i];
        if (arr1[i] < min_val) min_val = arr1[i];
        
        /* Additional computation to increase complexity */
        arr1[i] = (arr1[i] + arr2[i]) % 1000;
    }
    
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

/* Conditional temporaries with volatile conditions */
__attribute__((optimize("O2")))
void test_conditional_temporaries(int *arr, int n) {
    volatile int cond1 = g_volatile_flag;
    volatile int cond2 = runtime_value(0) > 50;
    
    /* Parallel region with if clause - may generate _condtemp_ */
    #pragma omp parallel if(cond1) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with if clause inside parallel region */
        #pragma omp task if(cond2 && (tid % 2 == 0))
        {
            for (int i = tid; i < n; i += omp_get_num_threads()) {
                arr[i] += tid * runtime_value(i);
            }
        }
        
        #pragma omp taskwait
        
        /* Teams construct with if clause */
        #pragma omp target teams if(cond1) num_teams(2) thread_limit(64)
        {
            #pragma omp distribute parallel for
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] * 2 - runtime_value(i);
            }
        }
    }
    
    __builtin_printf("Conditional test completed, arr[0]=%d\n", arr[0]);
}

/* Scan directives to trigger _scantemp_ generation */
__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n) {
    int prefix_sum = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        int val = arr[i] + runtime_value(i);
        
        #pragma omp scan exclusive(prefix_sum)
        {
            arr[i] = prefix_sum;
            prefix_sum += val;
        }
    }
    
    /* Inclusive scan pattern */
    int running_sum = 0;
    #pragma omp parallel for reduction(+:running_sum)
    for (int i = 0; i < n; i++) {
        running_sum += arr[i] % 100;
        arr[i] = running_sum;
    }
    
    __builtin_printf("Scan test completed, final sum=%d\n", running_sum);
}

/* Enter data with to modifier */
__attribute__((optimize("O2")))
void test_enter_data_to(int *data, int n) {
    /* Allocate device data with enter data to(...) */
    int *device_data = (int *)malloc(n * sizeof(int));
    
    if (device_data) {
        /* Initialize host data */
        for (int i = 0; i < n; i++) {
            device_data[i] = runtime_value(i);
        }
        
        /* This should generate OMP_CLAUSE_ENTER with to modifier */
        #pragma omp target enter data map(to: device_data[0:n])
        
        /* Use the data in target region */
        #pragma omp target teams distribute parallel for \
                map(tofrom: device_data[0:n])
        for (int i = 0; i < n; i++) {
            device_data[i] = device_data[i] * 3 + i;
        }
        
        /* Exit data */
        #pragma omp target exit data map(from: device_data[0:n])
        
        __builtin_printf("Enter data test, device_data[0]=%d\n", device_data[0]);
        
        free(device_data);
    }
}

/* Nested and combined constructs for complex clause generation */
__attribute__((optimize("O3")))
void test_nested_combined(int *arr1, int *arr2, int n) {
    volatile int outer_cond = g_volatile_flag;
    
    /* Outer parallel region */
    #pragma omp parallel if(outer_cond) num_threads(2)
    {
        /* Inner reduction region */
        #pragma omp for reduction(+:arr1[0:n]) nowait
        for (int i = 0; i < n; i++) {
            arr1[i] += runtime_value(i);
        }
        
        /* Task with reduction-like computation */
        #pragma omp task
        {
            int local_sum = 0;
            for (int i = 0; i < n/2; i++) {
                local_sum += arr2[i] * runtime_value(i);
            }
            #pragma omp atomic
            arr2[0] += local_sum;
        }
        
        #pragma omp taskwait
        
        /* Combined target teams distribute parallel for */
        #pragma omp target teams distribute parallel for \
                map(tofrom: arr1[0:n]) if(outer_cond)
        for (int i = 0; i < n; i++) {
            arr1[i] = (arr1[i] * 2) % 1000;
        }
    }
    
    __builtin_printf("Nested test checksum: arr1[0]=%d, arr2[0]=%d\n", 
                     arr1[0], arr2[0]);
}

/* Main function with runtime-dependent execution */
int main(int argc, char **argv) {
    /* Use argv to seed random generation */
    if (argc > 1) {
        g_seed = atoi(argv[1]);
    }
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = runtime_value(i) % 1000;
        array2[i] = runtime_value(i + N) % 1000;
    }
    
    int checksum = 0;
    volatile int repeat_count = g_loop_counter;
    
    /* Repeat tests multiple times to ensure processing */
    for (int iter = 0; iter < repeat_count && iter < 3; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* Test 1: Reduction temporaries */
        test_reduction_temporaries(array1, array2, N);
        
        /* Test 2: Conditional temporaries */
        test_conditional_temporaries(array1, N);
        
        /* Test 3: Scan temporaries */
        test_scan_temporaries(array2, N);
        
        /* Test 4: Enter data with to modifier */
        test_enter_data_to(array1, N);
        
        /* Test 5: Nested and combined constructs */
        test_nested_combined(array1, array2, N);
        
        /* Calculate checksum to prevent dead code elimination */
        for (int i = 0; i < N; i += 16) {
            checksum += array1[i] ^ array2[i % N];
        }
        
        /* Modify volatile flag for next iteration */
        g_volatile_flag = runtime_value(iter) % 2;
    }
    
    __builtin_printf("\nFinal checksum: %d\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
