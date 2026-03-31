/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global data for declare target enter clause */
int global_data[1000];
#pragma omp declare target enter(global_data)

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd : int : omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create non-trivial conditions */
int check_threshold(int val) {
    volatile int limit = 500; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char *argv[]) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    volatile int result_sink; /* Prevent dead code elimination */
    
    /* Initialize arrays with runtime-dependent values */
    for (i = 0; i < N; i++) {
        global_data[i] = i + (argc > 1 ? atoi(argv[1]) : 0);
    }
    
    /* ============================================
     * Test 1: Target with reduction and if clause
     * Should generate _reductemp_ and _condtemp_
     * ============================================ */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) reduction(+:sum) \
        if(target: argc > 1) /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        sum += global_data[i];
    }
    result_sink = sum;
    printf("Target reduction sum: %d\n", sum);
    
    /* ============================================
     * Test 2: Nested reductions with custom reduction
     * Multiple reductions increase chance of _reductemp_
     * ============================================ */
    sum = 0;
    #pragma omp parallel for reduction(+:sum) reduction(max:max_val) \
        reduction(min:min_val) reduction(myadd:sum) \
        if(parallel: check_threshold(N)) /* Function call in condition */
    for (i = 0; i < N; i++) {
        int val = global_data[i];
        sum += val;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    printf("Multi-reduction: sum=%d, max=%d, min=%d\n", sum, max_val, min_val);
    
    /* ============================================
     * Test 3: SIMD with inscan reduction (OpenMP 5.0+)
     * Explicitly requires _scantemp_ clauses
     * ============================================ */
    int partial_sums[N];
    scan_sum = 0;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10) /* Uneven scheduling */
    for (i = 0; i < N; i++) {
        int val = global_data[i] % 100;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        partial_sums[i] = scan_sum;
    }
    
    /* Use results to prevent optimization */
    printf("Scan partial sum[%d] = %d\n", N/2, partial_sums[N/2]);
    
    /* ============================================
     * Test 4: Array reduction (OpenMP 5.1)
     * Complex reduction may generate temporaries
     * ============================================ */
    int arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10]) \
        if(omp_get_num_threads() > 1) /* Runtime condition */
    for (i = 0; i < N; i++) {
        arr_sum[i % 10] += global_data[i];
    }
    
    /* ============================================
     * Test 5: Nowait with reduction
     * Creates more complex scheduling
     * ============================================ */
    int nowait_sum = 0;
    #pragma omp parallel sections reduction(+:nowait_sum)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                nowait_sum += global_data[i];
            }
        }
        #pragma omp section nowait
        {
            for (i = N/2; i < N; i++) {
                nowait_sum += global_data[i];
            }
        }
    }
    
    /* Final volatile store to ensure all computations are kept */
    result_sink = sum + scan_sum + nowait_sum + arr_sum[0];
    
    return 0;
}
