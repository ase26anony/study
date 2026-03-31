/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */

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
    
    /* Use argc to make loop bounds runtime-dependent */
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 1000;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex target region with reduction and if clause 
       (may generate _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2) /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) max_val = target_data[i];
    }
    
    /* Array reduction with custom reduction operator
       (increases chance of internal temporaries) */
    #pragma omp parallel for reduction(vec_add: array_sum) \
        if(check_threshold(N)) /* Function call in condition */
    for (i = 0; i < N; i++) {
        array_sum[i % 100] += target_data[i];
    }
    
    /* Scan directive (triggers OMP_CLAUSE__SCANTEMP_) */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < N; i++) {
        #pragma omp scan inclusive(scan_sum)
        scan_sum += target_data[i];
    }
    
    /* Nested reductions with different operations */
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        schedule(dynamic, 10) /* Dynamic scheduling adds complexity */
    for (i = 0; i < N; i++) {
        if (target_data[i] > max_val) max_val = target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    /* Mixed reduction types in single construct */
    volatile int sink; /* volatile to prevent optimization */
    #pragma omp parallel for reduction(+:sum) reduction(*:scan_sum) \
        if(parallel: N > 100) nowait
    for (i = 0; i < N/2; i++) {
        sum += target_data[i];
        scan_sum *= (target_data[i] + 1);
    }
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    /* Print results to prevent dead code elimination */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d\n", 
           sum, max_val, min_val, scan_sum);
    printf("Array sum[0]=%d, array sum[50]=%d\n", array_sum[0], array_sum[50]);
    
    /* Use sink to ensure computations aren't optimized away */
    sink = sum + max_val + min_val + scan_sum;
    
    return 0;
}
