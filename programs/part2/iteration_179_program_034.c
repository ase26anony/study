/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global data for declare target enter clause */
#define N 1000
int data_array[N];
volatile int sink; /* Prevent optimizations */

#pragma omp declare target enter(data_array)  /* Triggers OMP_CLAUSE_ENTER */

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd: int: omp_out += omp_in) \
    initializer(omp_priv = 0)

int main(int argc, char **argv) {
    int i;
    int sum = 0, sum1 = 0, sum2 = 0;
    int scan_sum = 0;
    int max_val = -1000000, min_val = 1000000;
    int array_sum = 0;
    
    /* Use argc for runtime-dependent behavior */
    int iterations = (argc > 1) ? atoi(argv[1]) : 500;
    if (iterations < 100) iterations = 100;
    if (iterations > N) iterations = N;
    
    volatile int threshold = 10; /* Force memory operations */
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data_array[i] = i % 100;
    }
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) \
        reduction(min:min_val) \
        if(argc > 2)  /* Non-trivial condition for _condtemp_ */
    for (i = 0; i < iterations; i++) {
        sum1 += data_array[i];
        sum2 += data_array[i] * 2;
        if (data_array[i] > max_val) max_val = data_array[i];
        if (data_array[i] < min_val) min_val = data_array[i];
    }
    
    sink = sum1 + sum2; /* Use results */
    
    /* 2. Array reduction - may generate additional temporaries */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma omp parallel for reduction(+:partial_sums[:4])
    for (i = 0; i < iterations; i++) {
        partial_sums[i % 4] += data_array[i];
    }
    
    for (i = 0; i < 4; i++) {
        array_sum += partial_sums[i];
    }
    
    /* 3. Scan directive - should generate _scantemp_ */
    int scan_values[N];
    for (i = 0; i < iterations; i++) {
        scan_values[i] = i % 10;
    }
    
    #pragma omp parallel for simd \
        reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < iterations; i++) {
        int val = scan_values[i];
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        scan_values[i] = scan_sum;
    }
    
    sink = scan_sum;
    
    /* 4. Nested parallelism with custom reduction */
    int custom_sum = 0;
    #pragma omp parallel for reduction(myadd:custom_sum) \
        if(threshold > 5)  /* Another condition */
    for (i = 0; i < iterations; i++) {
        custom_sum += data_array[i] / (threshold + 1);
    }
    
    /* 5. Nowait clause with uneven work distribution */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < iterations; i++) {
            sum += data_array[i];
        }
        
        #pragma omp for reduction(+:sum)
        for (i = iterations/2; i < iterations; i++) {
            sum += data_array[i] * 3;
        }
    }
    
    /* Print results to prevent elimination */
    printf("Results: sum1=%d, sum2=%d, max=%d, min=%d\n", 
           sum1, sum2, max_val, min_val);
    printf("Array sum: %d, Scan sum: %d\n", array_sum, scan_sum);
    printf("Custom sum: %d, Final sum: %d\n", custom_sum, sum);
    
    return 0;
}
