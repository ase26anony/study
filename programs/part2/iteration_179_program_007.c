/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test_omp_internal_clauses.c -o test_omp_internal_clauses */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global data for declare target enter clause */
int global_data[1000];
volatile int sink; /* Prevent optimization */

#pragma omp declare target enter(global_data)  /* OMP_CLAUSE_ENTER */

/* Custom reduction for complex cases */
#pragma omp declare reduction(custom_add : int : omp_out = omp_out + omp_in * 2) \
    initializer(omp_priv = 0)

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    volatile int threshold = 500; /* Force runtime evaluation */
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        global_data[i] = i % 100;
    }
    
    /* Complex target region with reduction and if clause - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 1) map(tofrom: sum, max_val)
    for (i = 0; i < N; i++) {
        int val = global_data[i];
        sum += val;
        if (val > max_val) max_val = val;
    }
    
    sink = sum + max_val; /* Use results */
    
    /* SIMD with inscan reduction - should generate _scantemp_ */
    int partial_sums[100];
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        private(i) schedule(static, 10)
    for (i = 0; i < N; i++) {
        int val = global_data[i] % 50;
        
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        
        if (i % 10 == 0) {
            partial_sums[i/10] = scan_sum;
        }
    }
    
    sink = scan_sum; /* Use result */
    
    /* Nested reduction with custom operator and volatile condition */
    int custom_sum = 0;
    volatile int cond = (argc > 2);
    
    #pragma omp parallel for reduction(custom_add:custom_sum) \
        if(parallel: cond)  /* May generate _condtemp_ */
    for (i = 0; i < N; i++) {
        custom_sum += global_data[i];
    }
    
    sink = custom_sum;
    
    /* Array reduction (OpenMP 5.1) - may generate additional temporaries */
    int arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (i = 0; i < N; i++) {
        arr_sum[i % 10] += global_data[i];
    }
    
    /* Use all results to prevent dead code elimination */
    int total = 0;
    for (i = 0; i < 10; i++) {
        total += arr_sum[i];
    }
    
    printf("Results: sum=%d, max=%d, scan=%d, custom=%d, arr_total=%d\n",
           sum, max_val, scan_sum, custom_sum, total);
    
    /* Additional complex case: reduction with nowait */
    int sum1 = 0, sum2 = 0;
    #pragma omp parallel sections reduction(+:sum1, sum2)
    {
        #pragma omp section
        for (i = 0; i < N/2; i++) {
            sum1 += global_data[i];
        }
        
        #pragma omp section
        for (i = N/2; i < N; i++) {
            sum2 += global_data[i];
        }
    }
    
    printf("Section sums: %d + %d = %d\n", sum1, sum2, sum1 + sum2);
    
    return 0;
}
