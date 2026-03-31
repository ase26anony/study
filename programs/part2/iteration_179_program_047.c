/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int global_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Function to create non-trivial conditions */
int check_threshold(int val) {
    volatile int limit = 500; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char *argv[]) {
    int i, j;
    int sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    
    /* Runtime-dependent iteration count */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Initialize data */
    for (i = 0; i < n; i++) {
        global_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: global_data[0:n])
    
    /* Complex target region with reduction and condition 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2) /* Non-trivial condition */
    for (i = 0; i < n; i++) {
        int val = global_data[i] * (i % 10);
        sum += val;
        if (val > max_val) max_val = val;
    }
    
    /* Array reduction with custom operator 
       (may trigger additional _reductemp_) */
    #pragma omp parallel for reduction(vec_add: array_sum) \
        if(parallel: check_threshold(n)) /* Function call in condition */
    for (i = 0; i < n; i++) {
        int idx = i % 100;
        array_sum[idx] += global_data[i];
    }
    
    /* Scan directive (triggers _scantemp_) */
    volatile int volatile_n = n; /* Prevent optimization */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < volatile_n; i++) {
        int val = global_data[i] + 1;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
    }
    
    /* Nested reductions with different operators */
    int outer_sum = 0;
    #pragma omp parallel for collapse(2) \
        reduction(+:outer_sum) reduction(min:min_val) \
        schedule(dynamic, 4) /* Dynamic scheduling adds complexity */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < n/10; j++) {
            int idx = i * (n/10) + j;
            if (idx < n) {
                outer_sum += global_data[idx];
                if (global_data[idx] < min_val) 
                    min_val = global_data[idx];
            }
        }
    }
    
    /* Nowait clause creates additional complexity */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < n/2; i++) {
            sum += global_data[i] * 2;
        }
        
        #pragma omp for reduction(*:scan_sum) /* Different reduction op */
        for (i = n/2; i < n; i++) {
            if (global_data[i] != 0)
                scan_sum *= (global_data[i] + 1);
        }
    }
    
    /* Exit data from target */
    #pragma omp target exit data map(from: global_data[0:n])
    
    /* Print results to prevent elimination */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d, outer_sum=%d\n",
           sum, max_val, min_val, scan_sum, outer_sum);
    
    /* Print array sum to use the result */
    int total_array = 0;
    for (i = 0; i < 100; i++) {
        total_array += array_sum[i];
    }
    printf("Array reduction total: %d\n", total_array);
    
    return 0;
}
