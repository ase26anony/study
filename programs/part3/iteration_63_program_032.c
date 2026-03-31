/* test_omp_clause_printing.c
 * Compile with: gcc -std=gnu11 -O2 -fopenmp -fdump-tree-all -o test_omp test_omp_clause_printing.c
 * For target offloading: gcc -std=gnu11 -O2 -fopenmp -foffload=disable -fdump-tree-all -o test_omp test_omp_clause_printing.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define ARRAY_SIZE 512

/* Function to generate pseudo-random values */
static unsigned int lcg_seed = 12345;
static inline unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Force tree dumping with optimization attribute */
__attribute__((optimize("O2")))
void complex_reductions(volatile int flag, int *data, int size) {
    volatile int start = flag ? 1 : 0;
    volatile int end = size - (flag ? 0 : 1);
    
    int sum = 0;
    int product = 1;
    int max_val = INT_MIN;
    int min_val = INT_MAX;
    
    /* Complex reduction with multiple operators - may generate _reductemp_ */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
             reduction(max:max_val) reduction(min:min_val) \
             private(start, end)
    for (int i = start; i < end; i++) {
        /* Data-dependent computation to prevent optimization */
        int idx = (i * 31) % size;
        int val = data[idx] + (i % 7);
        
        sum += val;
        product *= (val != 0 ? val : 1);  /* Avoid multiplication by zero */
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
        
        /* Additional complexity with nested conditions */
        if (val % 3 == 0) {
            sum += (i % 5);
        }
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reductions: sum=%d, product=%d, max=%d, min=%d\n", 
                     sum, product, max_val, min_val);
}

__attribute__((optimize("O2")))
void conditional_parallelism(volatile int cond_flag, int *data, int size) {
    volatile int runtime_cond = cond_flag;
    
    /* OMP_CLAUSE__CONDTEMP_ may be generated for these if clauses */
    #pragma omp parallel if(runtime_cond > 0) num_threads(4)
    {
        volatile int task_cond = runtime_cond % 2;
        
        #pragma omp single
        {
            #pragma omp task if(task_cond > 0)
            {
                int local_sum = 0;
                for (int i = 0; i < size/2; i++) {
                    local_sum += data[i];
                }
                __builtin_printf("Task 1 sum: %d\n", local_sum);
            }
            
            #pragma omp task if(task_cond == 0)
            {
                int local_prod = 1;
                for (int i = size/2; i < size; i++) {
                    local_prod *= (data[i] != 0 ? data[i] : 1);
                }
                __builtin_printf("Task 2 product: %d\n", local_prod);
            }
        }
    }
    
    /* Another conditional construct */
    #pragma omp target teams if(runtime_cond < 0) num_teams(2) thread_limit(32)
    {
        #pragma omp distribute parallel for if(runtime_cond != 0)
        for (int i = 0; i < size; i++) {
            data[i] += (i % 11);
        }
    }
}

__attribute__((optimize("O2")))
void scan_operations(int *data, int size) {
    int scan_array[ARRAY_SIZE];
    
    /* Initialize scan array */
    for (int i = 0; i < size; i++) {
        scan_array[i] = data[i] % 100;
    }
    
    int sum = 0;
    
    /* Exclusive scan - may generate _scantemp_ */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < size; i++) {
        sum += scan_array[i];
        #pragma omp scan exclusive(sum)
        scan_array[i] = sum - scan_array[i];  /* Exclusive scan result */
    }
    
    /* Inclusive scan */
    int prefix_sum = 0;
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < size; i++) {
        prefix_sum += data[i] % 50;
        #pragma omp scan inclusive(prefix_sum)
        data[i] = prefix_sum;
    }
    
    __builtin_printf("Scan operations completed\n");
}

__attribute__((optimize("O2")))
void enter_data_with_to(int *host_data, int size) {
    /* Allocate device memory using enter data with to clause */
    int *device_data = (int *)malloc(size * sizeof(int));
    if (!device_data) return;
    
    /* Initialize host data */
    for (int i = 0; i < size; i++) {
        host_data[i] = i * 3;
    }
    
    /* OMP_CLAUSE_ENTER with to modifier */
    #pragma omp enter data to(device_data[0:size])
    
    /* Copy data to device */
    #pragma omp target teams distribute parallel for \
            map(tofrom: device_data[0:size])
    for (int i = 0; i < size; i++) {
        device_data[i] = host_data[i] * 2;
    }
    
    /* Copy back */
    #pragma omp target teams distribute parallel for \
            map(from: host_data[0:size])
    for (int i = 0; i < size; i++) {
        host_data[i] = device_data[i] / 2;
    }
    
    #pragma omp exit data delete(device_data)
    
    free(device_data);
    __builtin_printf("Enter data with 'to' completed\n");
}

__attribute__((optimize("O2")))
void nested_combined_constructs(volatile int iter, int *data, int size) {
    /* Nested and combined constructs for complex clause generation */
    
    #pragma omp parallel num_threads(2)
    {
        volatile int inner_flag = iter % 3;
        
        #pragma omp for reduction(+:iter) nowait
        for (int i = 0; i < size; i++) {
            data[i] += (i * iter) % 97;
        }
        
        #pragma omp single
        {
            #pragma omp task if(inner_flag > 0)
            {
                int temp = 0;
                for (int j = 0; j < size/4; j++) {
                    temp += data[j];
                }
                __builtin_printf("Nested task temp: %d\n", temp);
            }
        }
    }
    
    /* Combined construct */
    #pragma omp target teams distribute parallel for \
            map(tofrom: data[0:size/2]) \
            reduction(+:iter)
    for (int i = 0; i < size/2; i++) {
        data[i] = (data[i] + iter) % 256;
    }
}

int main(int argc, char **argv) {
    /* Seed from command line for runtime variability */
    if (argc > 1) {
        lcg_seed = atoi(argv[1]);
    }
    
    int size = ARRAY_SIZE;
    int *data1 = (int *)malloc(size * sizeof(int));
    int *data2 = (int *)malloc(size * sizeof(int));
    
    if (!data1 || !data2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < size; i++) {
        data1[i] = (int)(lcg_rand() % 1000);
        data2[i] = (int)(lcg_rand() % 1000);
    }
    
    volatile int flag1 = (argc > 2) ? atoi(argv[2]) : 1;
    volatile int flag2 = (argc > 3) ? atoi(argv[3]) : 0;
    volatile int flag3 = (argc > 4) ? atoi(argv[4]) : 1;
    
    volatile int iterations = 2;  /* Force multiple passes */
    
    for (int iter = 0; iter < iterations; iter++) {
        volatile int dynamic_flag = (iter + lcg_seed) % 3;
        
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Trigger _reductemp_ generation */
        complex_reductions(dynamic_flag, data1, size);
        
        /* 2. Trigger _condtemp_ generation */
        conditional_parallelism(dynamic_flag, data2, size);
        
        /* 3. Trigger _scantemp_ generation */
        scan_operations(data1, size);
        
        /* 4. Trigger OMP_CLAUSE_ENTER with 'to' modifier */
        enter_data_with_to(data2, size);
        
        /* 5. Additional nested constructs for complexity */
        nested_combined_constructs(dynamic_flag, data1, size);
        
        /* Compute checksum to prevent optimization */
        long long checksum = 0;
        for (int i = 0; i < size; i++) {
            checksum += data1[i] + data2[i];
            /* Mix operations to create data dependencies */
            data1[i] = (data1[i] + i) % 1000;
            data2[i] = (data2[i] * 31) % 1000;
        }
        
        __builtin_printf("Iteration %d checksum: %lld\n", iter, checksum);
        
        /* Force side effects */
        if (checksum % 7 == 0) {
            flag1 = !flag1;
        }
    }
    
    /* Final output */
    int final_sum = 0;
    for (int i = 0; i < size; i++) {
        final_sum += data1[i] + data2[i];
    }
    __builtin_printf("Final sum: %d\n", final_sum);
    
    free(data1);
    free(data2);
    
    return 0;
}
