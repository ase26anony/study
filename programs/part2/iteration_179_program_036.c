/* test_omp_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details test_omp_clauses.c -o test_omp_clauses */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
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
    int i, sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    int N = 1000;
    
    /* Use argc for runtime-dependent behavior */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex target region with reduction and if clause 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2) /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) {
            max_val = target_data[i];
        }
    }
    
    /* Nested reductions with custom operator 
       (increases chance of _reductemp_) */
    #pragma omp parallel for reduction(vec_add:array_sum) \
        reduction(min:min_val) \
        if(check_threshold(N)) /* Function call in condition */
    for (i = 0; i < N; i++) {
        int idx = i % 100;
        array_sum[idx] += target_data[i];
        if (target_data[i] < min_val) {
            min_val = target_data[i];
        }
    }
    
    /* Scan directive (triggers _scantemp_) */
    int partial_sums[1000];
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < N; i++) {
        int val = target_data[i];
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] = scan_sum;
        scan_sum += val;
    }
    
    /* Combined directive with multiple clauses */
    int sum2 = 0, max2 = -1;
    volatile int use_parallel = 1; /* volatile prevents optimization */
    
    #pragma omp target teams distribute parallel for \
        reduction(+:sum2) reduction(max:max2) \
        if(use_parallel) schedule(dynamic, 16)
    for (i = 0; i < N; i++) {
        sum2 += target_data[i] * 2;
        if (target_data[i] > max2) {
            max2 = target_data[i];
        }
    }
    
    /* Output results to prevent dead code elimination */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d, sum2=%d, max2=%d\n",
           sum, max_val, min_val, scan_sum, sum2, max2);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    return 0;
}
