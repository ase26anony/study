/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Force tree dumping with optimization attribute */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int size, int *arr1, int *arr2, volatile int flag) {
    int sum = 0;
    int prod = 1;
    float max_val = -1e9;
    float min_val = 1e9;
    
    /* Complex reduction with array indexing that prevents optimization */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(arr1, arr2) if(flag > 0)
    for (int i = 0; i < size; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i * 17 + flag) % size;
        sum += arr1[idx] + arr2[i % size];
        prod *= (arr1[i] % 10 + 1);  /* Avoid zero product */
        float val = (float)arr1[i] / (arr2[i % size] + 1);
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: %d %d %.2f %.2f\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2")))
void test_condtemp_temporaries(int size, int *arr, volatile int cond_flag) {
    volatile int runtime_cond = cond_flag;
    
    /* OMP_CLAUSE__CONDTEMP_ generation */
    #pragma omp parallel if(runtime_cond > 0) num_threads(4)
    {
        #pragma omp single
        {
            for (int i = 0; i < size; i++) {
                /* Task with conditional execution */
                #pragma omp task if(runtime_cond > 0 && i % 3 == 0) \
                        firstprivate(i) shared(arr)
                {
                    arr[i] = arr[i] * 2 + i;
                }
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(runtime_cond < 0) num_teams(2) thread_limit(32) \
            map(tofrom: arr[0:size])
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < size; i++) {
            arr[i] = arr[i] + 1;
        }
    }
    
    __builtin_printf("Condtemp array[0]: %d\n", arr[0]);
}

__attribute__((optimize("O2")))
void test_scantemp_temporaries(int size, int *arr) {
    int scan_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum)
    for (int i = 0; i < size; i++) {
        /* Exclusive scan computation */
        #pragma omp scan exclusive(scan_sum)
        {
            int temp = arr[i];
            arr[i] = scan_sum;
            scan_sum += temp;
        }
    }
    
    /* Inclusive scan variant */
    int incl_sum = 0;
    #pragma omp parallel for reduction(inscan, +:incl_sum)
    for (int i = 0; i < size; i++) {
        incl_sum += arr[i];
        #pragma omp scan inclusive(incl_sum)
        arr[i] = incl_sum;
    }
    
    __builtin_printf("Scan results: %d %d\n", scan_sum, incl_sum);
}

__attribute__((optimize("O2")))
void test_enter_clause(int size) {
    /* Dynamic allocation for enter data clause */
    int *device_arr = (int*)malloc(size * sizeof(int));
    if (!device_arr) return;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        device_arr[i] = i * 2;
    }
    
    /* OMP_CLAUSE_ENTER with 'to' modifier */
    #pragma omp enter data to(device_arr[0:size])
    
    /* Use the device array in a target region */
    #pragma omp target map(tofrom: device_arr[0:size])
    {
        #pragma omp teams distribute parallel for
        for (int i = 0; i < size; i++) {
            device_arr[i] = device_arr[i] * 3 + 1;
        }
    }
    
    /* Exit data */
    #pragma omp exit data from(device_arr[0:size])
    
    __builtin_printf("Enter clause array[10]: %d\n", device_arr[10]);
    free(device_arr);
}

__attribute__((optimize("O2")))
void nested_combined_constructs(int size, int *arr1, int *arr2, volatile int flag) {
    /* Nested parallel regions with combined constructs */
    #pragma omp parallel num_threads(2) if(flag > 0)
    {
        #pragma omp for reduction(+:arr1[:size]) nowait
        for (int i = 0; i < size; i++) {
            arr1[i] += i;
        }
        
        #pragma omp single
        {
            #pragma omp taskloop reduction(*:arr2[:size]) if(flag < 0)
            for (int i = 0; i < size; i++) {
                arr2[i] *= (i % 7 + 1);
            }
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:size], arr2[0:size]) \
            if(flag != 0) num_teams(2) thread_limit(64)
    for (int i = 0; i < size; i++) {
        arr1[i] = arr1[i] + arr2[i];
        arr2[i] = arr1[i] - arr2[i];
    }
    
    __builtin_printf("Nested constructs checksum: %d\n", arr1[size/2] + arr2[size/2]);
}

int main(int argc, char **argv) {
    /* Use argc for runtime variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    const int SIZE = 512;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100 + 1;
        array2[i] = rand() % 100 + 1;
    }
    
    /* Volatile flags for runtime-dependent control flow */
    volatile int flag1 = (seed % 3) - 1;
    volatile int flag2 = (seed % 5) - 2;
    volatile int flag3 = (seed % 7) - 3;
    volatile int counter = 2 + (seed % 2);
    
    int checksum = 0;
    
    /* Loop to process multiple times */
    for (volatile int iter = 0; iter < counter; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(SIZE, array1, array2, flag1 + iter);
        
        /* 2. Test condtemp temporaries */
        test_condtemp_temporaries(SIZE, array1, flag2 + iter);
        
        /* 3. Test scantemp temporaries */
        test_scantemp_temporaries(SIZE, array2);
        
        /* 4. Test enter clause with to modifier */
        test_enter_clause(SIZE / 4);
        
        /* 5. Nested and combined constructs */
        nested_combined_constructs(SIZE, array1, array2, flag3 + iter);
        
        /* Update checksum to prevent optimization */
        for (int i = 0; i < SIZE; i += 8) {
            checksum += array1[i] ^ array2[i];
        }
    }
    
    /* Final output to prevent dead code elimination */
    __builtin_printf("\nFinal checksum: %d\n", checksum);
    __builtin_printf("Sample values: array1[0]=%d, array2[0]=%d\n", 
                     array1[0], array2[0]);
    
    free(array1);
    free(array2);
    
    return 0;
}
