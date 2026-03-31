/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_internal_clauses.c */

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

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    int N = 1000;
    
    /* Use runtime argument to prevent constant folding */
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
    
    /* Complex target region with reduction and condition 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2) /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) 
            max_val = target_data[i];
    }
    
    /* Array reduction with custom reduction operator */
    #pragma omp parallel for reduction(vec_add: array_sum) \
        if(check_threshold(N)) /* Function call in condition */
    for (i = 0; i < N; i++) {
        int idx = i % 100;
        array_sum[idx] += target_data[i];
    }
    
    /* Scan directive (triggers _scantemp_) - requires OpenMP 5.0+ */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < N; i++) {
        #pragma omp scan exclusive(scan_sum)
        scan_sum += target_data[i];
    }
    
    /* Nested reductions with volatile variables */
    volatile int outer_sum = 0;
    #pragma omp parallel for reduction(+:outer_sum) \
        reduction(min:min_val) schedule(dynamic, 10)
    for (i = 0; i < N; i++) {
        outer_sum += target_data[i];
        if (target_data[i] < min_val) 
            min_val = target_data[i];
    }
    
    /* Combined directive with multiple clauses */
    #pragma omp target teams distribute parallel for \
        reduction(+:sum) if(omp_get_num_threads() > 1) \
        nowait
    for (i = 0; i < N/2; i++) {
        sum += target_data[i] * 2;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d\n", 
           sum, max_val, min_val, scan_sum);
    printf("Array sum[0]=%d, outer_sum=%d\n", array_sum[0], outer_sum);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    return 0;
}
