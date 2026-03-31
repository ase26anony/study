/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int[100] : \
    for (int i = 0; i < 100; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void process_results(volatile int *sink, int value) {
    *sink = value; /* Prevent dead code elimination */
}

int main(int argc, char **argv) {
    volatile int sink = 0;
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    
    /* Runtime-dependent iteration count to prevent optimization */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n <= 0) n = 1000;
    
    /* Initialize data */
    for (i = 0; i < n; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:n])
    
    /* Complex target construct with reduction and condition 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2)  /* Non-trivial condition */
    for (i = 0; i < n; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) max_val = target_data[i];
    }
    
    process_results(&sink, sum);
    process_results(&sink, max_val);
    
    /* Scan directive (triggers _scantemp_) */
    int scan_array[1000];
    for (i = 0; i < n; i++) scan_array[i] = i % 10;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < n; i++) {
        scan_sum += scan_array[i];
        #pragma omp scan inclusive(scan_sum)
        scan_array[i] = scan_sum;
    }
    
    process_results(&sink, scan_sum);
    
    /* Nested reductions with custom reduction operator */
    int local_data[100];
    for (i = 0; i < 100; i++) local_data[i] = i;
    
    #pragma omp parallel for reduction(vec_add: array_sum) \
        reduction(min: min_val) schedule(dynamic, 10)
    for (i = 0; i < n; i++) {
        int idx = i % 100;
        array_sum[idx] += local_data[idx];
        if (local_data[idx] < min_val) min_val = local_data[idx];
    }
    
    /* Volatile condition to force temporary generation */
    volatile int volatile_cond = argc;
    #pragma omp parallel for if(parallel: volatile_cond > 1) \
        reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += i % 50;
    }
    
    /* Nowait clause with uneven work distribution */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < n; i++) {
            sum += (i * 3) % 17;
        }
        
        #pragma omp for reduction(max:max_val)
        for (i = 0; i < n/2; i++) {
            if (i > max_val) max_val = i;
        }
    }
    
    /* Finalize target data */
    #pragma omp target exit data map(from: target_data[0:n])
    
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d, array_sum[0]=%d\n",
           sum, max_val, min_val, scan_sum, array_sum[0]);
    
    return 0;
}
