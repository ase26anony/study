/* test-omp-temp-clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test test-omp-temp-clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target enter(data_array)
static int data_array[1000];

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd : int : omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create non-trivial conditions */
int check_threshold(volatile int *val) {
    return (*val > 100);
}

int main(int argc, char **argv) {
    int i;
    volatile int threshold = 100; /* volatile to prevent optimization */
    int sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int reductemp_trigger = 0;
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        data_array[i] = i + 1;
    }
    
    /* Runtime-dependent iteration count */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* 1. Target region with reduction and if clause - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) reduction(+:sum) \
        if(target: argc > 1) /* Non-trivial condition */
    for (i = 0; i < n; i++) {
        sum += data_array[i % 1000];
    }
    
    printf("Target reduction sum: %d\n", sum);
    
    /* 2. Parallel for with multiple reductions - may generate multiple _reductemp_ */
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        reduction(+:reductemp_trigger) \
        if(parallel: check_threshold(&threshold)) /* Function call in condition */
    for (i = 0; i < n; i++) {
        int val = data_array[i % 1000];
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
        reductemp_trigger += val % 7;
    }
    
    printf("Max: %d, Min: %d, Trigger: %d\n", max_val, min_val, reductemp_trigger);
    
    /* 3. SIMD with inscan reduction - should generate _scantemp_ */
    int partial_sums[100];
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10) /* Uneven scheduling */
    for (i = 0; i < 100; i++) {
        int chunk_sum = 0;
        #pragma omp simd reduction(+:chunk_sum) /* Nested SIMD */
        for (int j = 0; j < 10; j++) {
            chunk_sum += data_array[i * 10 + j];
        }
        
        #pragma omp scan inclusive(scan_sum)
        scan_sum += chunk_sum;
        partial_sums[i] = scan_sum;
    }
    
    printf("Scan sum: %d, partial_sums[99]: %d\n", scan_sum, partial_sums[99]);
    
    /* 4. Array reduction (OpenMP 5.1) - may generate complex temporaries */
    int arr_reduce[10] = {0};
    #pragma omp parallel for reduction(+:arr_reduce[:10]) \
        if(check_threshold(&threshold)) /* Another condition */
    for (i = 0; i < n; i++) {
        arr_reduce[i % 10] += data_array[i % 1000];
    }
    
    printf("Array reduction[0]: %d, [9]: %d\n", arr_reduce[0], arr_reduce[9]);
    
    /* 5. Custom reduction with nowait - creates scheduling complexity */
    int custom_sum = 0;
    #pragma omp parallel sections reduction(myadd:custom_sum) nowait
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                custom_sum += data_array[i % 1000] % 13;
            }
        }
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                custom_sum += data_array[i % 1000] % 17;
            }
        }
    }
    
    printf("Custom reduction sum: %d\n", custom_sum);
    
    /* 6. Taskloop with reduction - different lowering path */
    int task_sum = 0;
    #pragma omp taskloop reduction(+:task_sum) grainsize(50) \
        if(taskloop: threshold > 50) /* Task-specific condition */
    for (i = 0; i < n; i++) {
        task_sum += data_array[i % 1000] % 11;
    }
    
    printf("Taskloop sum: %d\n", task_sum);
    
    /* Force use of all results to prevent optimization */
    volatile int sink = sum + max_val + min_val + scan_sum + arr_reduce[0] + 
                       custom_sum + task_sum + reductemp_trigger;
    (void)sink; /* Suppress unused warning */
    
    return 0;
}
