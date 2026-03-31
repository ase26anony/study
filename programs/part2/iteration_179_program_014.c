/* test_omp_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [100] : \
    for (int i = 0; i < 100; i++) omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Function to prevent optimization */
volatile int sink;

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    int N = 1000;
    
    /* Use argc to prevent compile-time optimization */
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 1000;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to:target_data[0:N])
    
    /* Complex reduction with if clause - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(argc > 2)  /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        int val = target_data[i];
        sum += val;
        if (val > max_val) max_val = val;
    }
    
    sink = sum;  /* Prevent dead code elimination */
    
    /* Scan directive - should generate _scantemp_ */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < N; i++) {
        int val = target_data[i];
        #pragma omp scan exclusive(scan_sum)
        scan_sum += val;
    }
    
    sink = scan_sum;
    
    /* Nested reductions with custom reduction */
    #pragma omp parallel for reduction(vec_add:array_sum) \
        reduction(min:min_val) nowait
    for (i = 0; i < N; i++) {
        int idx = i % 100;
        array_sum[idx] += target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    /* Mixed reductions with volatile to force temporaries */
    volatile int volatile_limit = 500;
    int partial_sum = 0;
    
    #pragma omp parallel for reduction(+:partial_sum) \
        if(volatile_limit > 250)  /* Volatile in condition */
    for (i = 0; i < N; i++) {
        if (i < volatile_limit) {
            partial_sum += target_data[i];
        }
    }
    
    sink = partial_sum + array_sum[0] + min_val;
    
    /* Exit data from target */
    #pragma omp target exit data map(from:target_data[0:N])
    
    printf("Results: sum=%d, scan_sum=%d, min=%d, max=%d, partial=%d\n",
           sum, scan_sum, min_val, max_val, partial_sum);
    
    return 0;
}
