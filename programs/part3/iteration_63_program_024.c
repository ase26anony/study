/* test_omp_clause_printing.c
 * Compile with: gcc -std=gnu11 -O2 -fopenmp -fdump-tree-all -o test_omp test_omp_clause_printing.c
 * Additional flags for target offloading: -foffload=nvptx-none (optional)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define ARRAY_SIZE 512

/* Function attribute to force optimization level */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int *data, int *data2, volatile int seed) {
    int sum = 0;
    int prod = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators and data-dependent indexing
     * Forces creation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            schedule(static, 16)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Data-dependent array access prevents optimization */
        int idx = (i * seed) % ARRAY_SIZE;
        sum += data[idx] + data2[i % 32];
        
        /* Avoid multiplication by zero to prevent optimization */
        if (data[idx] != 0) {
            prod *= (data[idx] > 0 ? data[idx] : 1);
        }
        
        /* Conditional updates to force temporary creation */
        if (data[idx] > max_val) {
            max_val = data[idx];
        }
        if (data[idx] < min_val) {
            min_val = data[idx];
        }
        
        /* Additional computation to inhibit optimization */
        data2[i % 32] += (i * seed) % 7;
    }
    
    __builtin_printf("Reduction results: sum=%d, prod=%d, max=%d, min=%d\n", 
                     sum, prod, max_val, min_val);
}

__attribute__((optimize("O2"), noinline))
void test_condition_temporaries(int *data, volatile int flag1, volatile int flag2) {
    /* Force creation of _condtemp_ temporaries with volatile conditions */
    #pragma omp parallel if(flag1 > 0) num_threads(4)
    {
        #pragma omp single
        {
            /* Nested task with condition */
            #pragma omp task if(flag2 == 0) shared(data)
            {
                for (int i = 0; i < ARRAY_SIZE/4; i++) {
                    data[i] += i * flag1;
                }
            }
            
            /* Another task with different condition */
            #pragma omp task if(flag1 < 10) shared(data)
            {
                for (int i = ARRAY_SIZE/4; i < ARRAY_SIZE/2; i++) {
                    data[i] -= i * flag2;
                }
            }
        }
    }
    
    /* Target teams with condition */
    #pragma omp target teams if(flag1 != flag2) map(tofrom: data[0:ARRAY_SIZE/2]) \
            num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for simd
        for (int i = 0; i < ARRAY_SIZE/2; i++) {
            data[i] = data[i] * 2 - 1;
        }
    }
    
    __builtin_printf("Condition test completed with flags: %d, %d\n", flag1, flag2);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int *data, volatile int seed) {
    int scan_sum = 0;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
            schedule(static, 8)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        /* Pre-scan computation */
        int val = data[i] + (i * seed) % 5;
        
        #pragma omp scan exclusive(scan_sum)
        {
            data[i] = scan_sum;
            scan_sum += val;
        }
    }
    
    /* Inclusive scan with different chunking */
    int scan_sum2 = 0;
    #pragma omp parallel for reduction(inscan, +:scan_sum2) \
            schedule(dynamic, 4)
    for (int i = ARRAY_SIZE - 1; i >= 0; i--) {
        int val = data[i] - (i * seed) % 3;
        
        #pragma omp scan inclusive(scan_sum2)
        {
            scan_sum2 += val;
            data[i] += scan_sum2;
        }
    }
    
    __builtin_printf("Scan test completed, final sums: %d, %d\n", scan_sum, scan_sum2);
}

__attribute__((optimize("O2"), noinline))
void test_enter_clause(volatile int size) {
    /* Dynamically allocate to force proper enter data mapping */
    int *device_array = (int *)malloc(size * sizeof(int));
    if (!device_array) return;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        device_array[i] = i * 2 + 1;
    }
    
    /* Use enter data with to clause - triggers OMP_CLAUSE_ENTER with to modifier */
    #pragma omp target enter data map(to: device_array[0:size])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for map(alloc: device_array[0:size])
    for (int i = 0; i < size; i++) {
        device_array[i] = device_array[i] * 3 - 2;
    }
    
    /* Retrieve results */
    #pragma omp target exit data map(from: device_array[0:size])
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum ^= device_array[i];
    }
    
    __builtin_printf("Enter clause test, checksum: %d\n", checksum);
    
    free(device_array);
}

__attribute__((optimize("O2"), noinline))
void nested_combined_constructs(int *data, volatile int seed) {
    /* Nested and combined constructs to increase clause tree complexity */
    
    /* Outer parallel region */
    #pragma omp parallel num_threads(2)
    {
        /* Inner reduction loop */
        #pragma omp for reduction(+:seed) nowait
        for (int i = 0; i < ARRAY_SIZE/2; i++) {
            data[i] += seed * i;
        }
        
        /* Task with reduction-like computation */
        #pragma omp task shared(data)
        {
            int local_sum = 0;
            for (int j = ARRAY_SIZE/2; j < ARRAY_SIZE; j++) {
                local_sum += data[j];
                data[j] = (data[j] * 3) % 100;
            }
            __builtin_printf("Task local sum: %d\n", local_sum);
        }
    }
    
    /* Combined target teams distribute parallel for */
    #pragma omp target teams distribute parallel for \
            map(tofrom: data[0:ARRAY_SIZE]) \
            num_teams(2) num_threads(32)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (data[i] + i) % 255;
    }
}

int main(int argc, char *argv[]) {
    /* Use argc for runtime variability */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    volatile int flag1 = 1;
    volatile int flag2 = 0;
    
    /* Initialize arrays with pseudo-random values */
    int *data = (int *)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int *)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!data || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with seed-dependent values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i * seed + 13) % 1000;
        data2[i] = (i * seed * 3 + 7) % 500;
    }
    
    /* Volatile counter to force multiple executions */
    volatile int iterations = 3;
    
    for (int iter = 0; iter < iterations; iter++) {
        __builtin_printf("\n=== Iteration %d (seed: %d) ===\n", iter, seed);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(data, data2, seed + iter);
        
        /* 2. Test condition temporaries */
        flag1 = (seed + iter) % 5;
        flag2 = (seed * iter) % 3;
        test_condition_temporaries(data, flag1, flag2);
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(data, seed - iter);
        
        /* 4. Test nested/combined constructs */
        nested_combined_constructs(data, seed + iter * 2);
        
        /* 5. Test enter clause with to modifier */
        if (iter % 2 == 0) {
            test_enter_clause(ARRAY_SIZE / 4);
        }
        
        /* Update seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Compute and print checksum to prevent optimization */
        int checksum = 0;
        for (int i = 0; i < ARRAY_SIZE; i++) {
            checksum = (checksum * 31 + data[i]) & 0xffff;
        }
        __builtin_printf("Iteration %d checksum: 0x%04x\n", iter, checksum);
    }
    
    /* Final checksum */
    int final_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum ^= data[i];
    }
    __builtin_printf("\nFinal checksum: %d\n", final_checksum);
    
    free(data);
    free(data2);
    
    return 0;
}
