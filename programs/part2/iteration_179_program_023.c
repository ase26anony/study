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
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Function to create runtime-dependent conditions */
int get_threshold(int argc) {
    volatile int v = 100; /* volatile to prevent optimization */
    return argc > 1 ? v * atoi(getenv("ITER_MULT") ? : "1") : v;
}

int main(int argc, char *argv[]) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    volatile int sink; /* Prevent dead code elimination */
    
    /* Initialize arrays with non-trivial patterns */
    for (i = 0; i < 1000; i++) {
        target_data[i] = i * (i % 7);
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:1000])
    
    int threshold = get_threshold(argc);
    int n = argc > 1 ? atoi(argv[1]) : 1000;
    
    /* Complex target region with reduction and if clause 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: threshold > 50) map(tofrom: sum, max_val)
    for (i = 0; i < n; i++) {
        sum += target_data[i % 1000];
        if (target_data[i % 1000] > max_val)
            max_val = target_data[i % 1000];
    }
    
    sink = sum; /* Use result */
    
    /* Scan directive (triggers _scantemp_) */
    int partials[100] = {0};
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < n; i++) {
        int val = i * 2;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        partials[i % 100] = scan_sum;
    }
    
    sink = scan_sum;
    
    /* Nested reductions with custom reduction operator */
    #pragma omp parallel for reduction(vec_add:array_sum) \
        reduction(min:min_val) schedule(dynamic, 7)
    for (i = 0; i < n; i++) {
        array_sum[i % 100] += i;
        if (i < min_val) min_val = i;
    }
    
    /* Complex condition with function call */
    #pragma omp parallel for if(parallel: threshold < 200) \
        reduction(+:sum) nowait
    for (i = 0; i < n; i++) {
        sum += i % 17;
    }
    
    /* Finalize target data */
    #pragma omp target exit data map(from: target_data[0:1000])
    
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d\n", 
           sum, max_val, min_val, scan_sum);
    printf("Array sum[0]=%d\n", array_sum[0]);
    
    return 0;
}
