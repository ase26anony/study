/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all test_omp_internal_clauses.c -o test_omp */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(complex_reduce : int : \
    omp_out = omp_out * 2 + omp_in) \
    initializer(omp_priv = 1)

/* Function to create runtime-dependent conditions */
int get_threshold(int argc) {
    volatile int v = 100; /* volatile to prevent optimization */
    return argc * v;
}

int main(int argc, char *argv[]) {
    int i, sum = 0, max_val = INT_MIN, min_val = INT_MAX;
    int scan_sum = 0, cond_sum = 0;
    int N = 1000;
    
    /* Initialize array with some values */
    for (i = 0; i < N; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex reduction with multiple variables and if clause 
       (triggers _reductemp_ and _condtemp_) */
    int runtime_bound = (argc > 1) ? atoi(argv[1]) : 500;
    volatile int vol_bound = runtime_bound; /* volatile to prevent constant folding */
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) reduction(min:min_val) \
        if(vol_bound > 250) map(tofrom: sum, max_val, min_val)
    for (i = 0; i < N; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) max_val = target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    printf("After target reduction: sum=%d, max=%d, min=%d\n", sum, max_val, min_val);
    
    /* Scan directive (triggers _scantemp_) */
    int partial_sums[1000];
    scan_sum = 0;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        private(i) schedule(dynamic)
    for (i = 0; i < N; i++) {
        /* Exclusive scan */
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] = scan_sum;
        scan_sum += i % 50;
    }
    
    printf("Scan sum: %d, partial_sums[500]=%d\n", scan_sum, partial_sums[500]);
    
    /* Nested reductions with custom reduction operator */
    int custom_result = 1;
    int threshold = get_threshold(argc);
    
    #pragma omp parallel for reduction(complex_reduce:custom_result) \
        if(threshold > 50) /* Another condition for _condtemp_ */ \
        schedule(guided)
    for (i = 0; i < 100; i++) {
        custom_result += i % 10;
    }
    
    printf("Custom reduction result: %d\n", custom_result);
    
    /* Array reduction (OpenMP 5.1) - may generate additional temporaries */
    int arr_sum[10] = {0};
    
    #pragma omp parallel for reduction(+:arr_sum[:10]) \
        if(N > 100) /* Yet another condition */
    for (i = 0; i < N; i++) {
        arr_sum[i % 10] += target_data[i];
    }
    
    printf("Array reduction: arr_sum[0]=%d, arr_sum[5]=%d\n", arr_sum[0], arr_sum[5]);
    
    /* Nowait clause with uneven work distribution */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:cond_sum)
        for (i = 0; i < N/2; i++) {
            cond_sum += target_data[i];
        }
        
        #pragma omp for reduction(+:cond_sum)
        for (i = N/2; i < N; i++) {
            cond_sum += target_data[i] * 2;
        }
    }
    
    printf("Conditional sum: %d\n", cond_sum);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    return 0;
}
