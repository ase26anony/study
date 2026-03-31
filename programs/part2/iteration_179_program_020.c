/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_internal_clauses.c */

#include <stdlib.h>
#include <stdio.h>

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
    volatile int base = 100; /* volatile to prevent optimization */
    return base + argc * 50;
}

int main(int argc, char *argv[]) {
    int i, j;
    int sum = 0, sum1 = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int reduction_temp = 0;
    volatile int sink; /* volatile sink to prevent dead code elimination */
    
    /* Runtime-dependent iteration count */
    int iterations = (argc > 1) ? atoi(argv[1]) : M;
    if (iterations <= 0) iterations = M;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        global_data[i] = i % 100;
    }
    
    /* 1. Target region with reduction and if clause - may generate _reductemp_ and _condtemp_ */
    int threshold = get_threshold(argc);
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: iterations > 50) map(tofrom: sum, max_val)
    for (i = 0; i < iterations; i++) {
        sum += global_data[i % N];
        if (global_data[i % N] > max_val) {
            max_val = global_data[i % N];
        }
    }
    sink = sum + max_val; /* Use results */
    
    /* 2. Parallel region with multiple reductions - may generate multiple _reductemp_ */
    #pragma omp parallel for reduction(+:sum1, sum2) \
        reduction(min:min_val) if(parallel: argc > 1)
    for (i = 0; i < iterations; i++) {
        sum1 += i;
        sum2 += i * 2;
        if (i < min_val) min_val = i;
    }
    sink = sum1 + sum2 + min_val;
    
    /* 3. SIMD region with inscan reduction - explicitly requires _scantemp_ */
    int scan_array[M];
    for (i = 0; i < M; i++) scan_array[i] = i % 10;
    
    #pragma omp simd reduction(inscan, +:scan_sum)
    for (i = 0; i < M; i++) {
        scan_sum += scan_array[i];
        #pragma omp scan inclusive(scan_sum)
        scan_array[i] = scan_sum;
    }
    sink = scan_sum;
    
    /* 4. Nested parallel region with custom reduction */
    #pragma omp parallel for reduction(merge:reduction_temp) \
        if(iterations > threshold)
    for (i = 0; i < iterations; i++) {
        reduction_temp += (i % 5);
    }
    sink = reduction_temp;
    
    /* 5. Teams distribute with array reduction (OpenMP 5.1) */
    int arr_reduce[10] = {0};
    #pragma omp target teams distribute parallel for simd \
        reduction(+:arr_reduce[:10]) if(target:argc > 2)
    for (i = 0; i < iterations; i++) {
        arr_reduce[i % 10] += 1;
    }
    for (i = 0; i < 10; i++) sink += arr_reduce[i];
    
    /* 6. Combined construct with multiple clauses */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: iterations > 75) collapse(2)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < iterations/10; j++) {
            int idx = i * (iterations/10) + j;
            sum += idx % 100;
            if (idx % 100 > max_val) max_val = idx % 100;
        }
    }
    sink = sum + max_val;
    
    printf("Results: %d %d %d %d %d %d\n", 
           sum, max_val, min_val, scan_sum, reduction_temp, sink);
    
    return 0;
}
