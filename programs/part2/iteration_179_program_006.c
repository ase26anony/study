/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test.c */

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

/* Volatile variables to prevent optimization */
volatile int vol_cond = 0;
volatile int vol_sum = 0;

int complex_condition(int argc) {
    /* Non-trivial condition to encourage _condtemp_ generation */
    return argc > 3 ? argc * 2 : argc + 1;
}

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    int N = 1000;
    
    /* Use runtime value to prevent constant folding */
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 100;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Combined directive with reduction and if clause 
       Likely triggers _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2) num_teams(4) thread_limit(64)
    for (i = 0; i < N; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) max_val = target_data[i];
    }
    
    vol_sum = sum; /* Force side effect */
    
    /* Scan directive (triggers _scantemp_) */
    int partial_sums[100] = {0};
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        private(i) schedule(static, 16)
    for (i = 0; i < N; i++) {
        int val = target_data[i];
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        partial_sums[i % 100] = scan_sum;
    }
    
    /* Nested reductions with custom reduction */
    #pragma omp parallel for reduction(vec_add:array_sum) \
        reduction(min:min_val) if(vol_cond > 0)
    for (i = 0; i < N; i++) {
        array_sum[i % 100] += target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    /* Complex condition with function call */
    #pragma omp parallel for if(complex_condition(argc) > 5) \
        reduction(+:sum)
    for (i = 0; i < N; i++) {
        sum += i % 10;
    }
    
    /* Array reduction (OpenMP 5.1) */
    int arr_red[10] = {0};
    #pragma omp parallel for reduction(+:arr_red[:10])
    for (i = 0; i < N; i++) {
        arr_red[i % 10] += 1;
    }
    
    /* Nowait clause with uneven work distribution */
    #pragma omp parallel
    {
        #pragma omp for nowait schedule(dynamic)
        for (i = 0; i < N; i++) {
            target_data[i] *= 2;
        }
        
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < N/2; i++) {
            sum += i;
        }
    }
    
    /* Final clause with volatile condition */
    #pragma omp parallel final(vol_cond > 0) reduction(+:sum)
    {
        #pragma omp for
        for (i = 0; i < 100; i++) {
            sum += i;
        }
    }
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    /* Print results to prevent elimination */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d\n", 
           sum, max_val, min_val, scan_sum);
    printf("Array sum[0]=%d, arr_red[0]=%d\n", array_sum[0], arr_red[0]);
    
    return 0;
}
