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
#pragma omp declare reduction(vec_add : int [100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Function to create non-trivial conditions */
int check_threshold(volatile int *val) {
    return (*val > 100);
}

int main(int argc, char **argv) {
    int i, sum = 0, scan_sum = 0;
    int max_val = -1000000, min_val = 1000000;
    volatile int threshold = 50; /* volatile to prevent optimization */
    int array_sum[100] = {0};
    int N = 1000;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex reduction with if clause - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) reduction(min:min_val) \
        if(argc > 1) map(tofrom: sum, max_val, min_val)
    for (i = 0; i < N; i++) {
        int val = target_data[i];
        sum += val;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    printf("After target reduction: sum=%d, max=%d, min=%d\n", sum, max_val, min_val);
    
    /* Scan directive - triggers _scantemp_ */
    scan_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10) num_threads(4)
    for (i = 0; i < N; i++) {
        #pragma omp scan inclusive(scan_sum)
        scan_sum += i % 50;
        target_data[i] = scan_sum;
    }
    
    printf("After scan: scan_sum=%d, target_data[999]=%d\n", scan_sum, target_data[999]);
    
    /* Nested reductions with volatile condition */
    sum = 0;
    max_val = -1000000;
    #pragma omp parallel for reduction(+:sum) reduction(max:max_val) \
        if(check_threshold(&threshold)) nowait
    for (i = 0; i < N; i++) {
        int val = i * (argc > 1 ? 2 : 1); /* runtime-dependent */
        sum += val;
        if (val > max_val) max_val = val;
    }
    
    printf("After nested reduction: sum=%d, max=%d\n", sum, max_val);
    
    /* Array reduction with custom reduction operator */
    #pragma omp parallel for reduction(vec_add: array_sum)
    for (i = 0; i < N; i++) {
        int idx = i % 100;
        array_sum[idx] += target_data[i];
    }
    
    /* Use results to prevent dead code elimination */
    volatile int sink = 0;
    sink += sum + scan_sum + max_val + min_val + array_sum[0];
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    return 0;
}
