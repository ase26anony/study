/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test_omp test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* Global data for declare target enter clause */
int global_data[N];
#pragma omp declare target enter(global_data)

/* Custom reduction for complex cases */
#pragma omp declare reduction(merge : int : omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create runtime-dependent conditions */
int get_threshold(int argc) {
    volatile int base = 10; /* volatile to prevent optimization */
    return base + argc * 5;
}

int main(int argc, char *argv[]) {
    int i, j;
    int sum = 0, sum1 = 0, sum2 = 0;
    int scan_sum = 0;
    int max_val = -1000000;
    int min_val = 1000000;
    int array_sum[M] = {0};
    volatile int sink; /* Prevent dead code elimination */
    
    /* Runtime-dependent iteration count */
    int iterations = (argc > 1) ? atoi(argv[1]) : N;
    if (iterations < 100) iterations = 100;
    if (iterations > N) iterations = N;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        global_data[i] = i % 100;
    }
    
    /* 1. Target teams with reduction and if clause - may generate _reductemp_ and _condtemp_ */
    int target_sum = 0;
    volatile int cond_var = (argc > 2) ? 1 : 0;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:target_sum) \
        if(target: cond_var > 0) \
        map(tofrom: target_sum)
    for (i = 0; i < iterations; i++) {
        target_sum += global_data[i];
    }
    sink = target_sum;
    
    /* 2. Parallel for with multiple reductions - may generate multiple _reductemp_ */
    #pragma omp parallel for reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        if(parallel: argc > 1)  /* Non-trivial condition */
    for (i = 0; i < iterations; i++) {
        sum1 += i;
        sum2 += (i % 10);
        if (i > max_val) max_val = i;
        if (i < min_val) min_val = i;
    }
    
    /* 3. SIMD with inscan reduction - should generate _scantemp_ */
    int partial_sums[4] = {0};
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        num_threads(4)
    for (i = 0; i < iterations; i++) {
        int val = i % 50;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        partial_sums[omp_get_thread_num()] = scan_sum;
    }
    
    /* 4. Nested parallel regions with custom reduction */
    int custom_red = 0;
    volatile int threshold = get_threshold(argc);
    
    #pragma omp parallel reduction(merge:custom_red) \
        if(threshold > 15)  /* Complex condition with function call */
    {
        #pragma omp for nowait  /* nowait creates more complex scheduling */
        for (i = 0; i < iterations; i++) {
            custom_red += (i % 20);
        }
    }
    
    /* 5. Array reduction (OpenMP 5.1) - may generate additional temporaries */
    #pragma omp parallel for reduction(+:array_sum[:M]) \
        schedule(dynamic, 10)  /* Dynamic scheduling complicates lowering */
    for (i = 0; i < iterations; i++) {
        for (j = 0; j < M && j <= i; j++) {
            array_sum[j] += (i % (j + 1));
        }
    }
    
    /* 6. Combined construct with multiple clause types */
    int combined_sum = 0;
    int combined_max = 0;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:combined_sum) \
        reduction(max:combined_max) \
        if(target: iterations > 500) \
        map(tofrom: combined_sum, combined_max)
    for (i = 0; i < iterations; i++) {
        combined_sum += global_data[i];
        if (global_data[i] > combined_max) {
            combined_max = global_data[i];
        }
    }
    
    /* Ensure all results are used */
    printf("Results: target_sum=%d, sum1=%d, sum2=%d, scan_sum=%d\n", 
           target_sum, sum1, sum2, scan_sum);
    printf("max=%d, min=%d, custom_red=%d, combined_sum=%d, combined_max=%d\n",
           max_val, min_val, custom_red, combined_sum, combined_max);
    
    /* Use array results */
    int array_total = 0;
    for (j = 0; j < M; j++) {
        array_total += array_sum[j];
    }
    printf("array_total=%d\n", array_total);
    
    return 0;
}
