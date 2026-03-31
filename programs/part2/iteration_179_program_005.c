/* test_omp_internal_clauses.c
 * Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test test_omp_internal_clauses.c
 * For offloading: gcc -O2 -fopenmp -foffload=amd_gcn -fdump-tree-omplower -o test test_omp_internal_clauses.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global data for declare target enter clause */
#define N 1000
int data_array[N];

#pragma omp declare target enter(data_array)  /* Triggers OMP_CLAUSE_ENTER */

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [N] : \
    for (int i = 0; i < N; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

int main(int argc, char *argv[]) {
    int i;
    volatile int runtime_bound;  /* Prevent optimization */
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        data_array[i] = i + 1;
    }
    
    /* Runtime-dependent iteration count */
    runtime_bound = (argc > 1) ? atoi(argv[1]) : 500;
    if (runtime_bound > N) runtime_bound = N;
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    int sum1 = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        if(argc > 2)  /* Conditional - may generate _condtemp_ */
    for (i = 0; i < runtime_bound; i++) {
        int val = data_array[i];
        sum1 += val;
        sum2 += val * val;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    printf("Reduction results: sum1=%d, sum2=%d, max=%d, min=%d\n", 
           sum1, sum2, max_val, min_val);
    
    /* 2. Array reduction with custom operator - likely generates _reductemp_ */
    int arr_sum[N] = {0};
    #pragma omp parallel for reduction(vec_add : arr_sum)
    for (i = 0; i < runtime_bound; i++) {
        arr_sum[i % N] += data_array[i];
    }
    
    /* 3. Scan directive - generates _scantemp_ */
    int scan_sum = 0;
    int scan_results[N] = {0};
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 1)  /* Uneven scheduling for complexity */
    for (i = 0; i < runtime_bound; i++) {
        scan_sum += data_array[i];
        #pragma omp scan inclusive(scan_sum)
        scan_results[i] = scan_sum;
    }
    
    printf("Scan results: final sum=%d, scan_results[%d]=%d\n", 
           scan_sum, runtime_bound-1, scan_results[runtime_bound-1]);
    
    /* 4. Nested parallelism with conditions */
    volatile int outer_cond = (argc > 3);
    
    #pragma omp parallel if(outer_cond)  /* May generate _condtemp_ */
    {
        int local_sum = 0;
        #pragma omp for reduction(+:local_sum) nowait
        for (i = 0; i < runtime_bound; i++) {
            local_sum += data_array[i] % 17;
        }
        /* Use result to prevent elimination */
        #pragma omp atomic
        sum1 += local_sum;
    }
    
    /* 5. Task reduction with condition */
    int task_sum = 0;
    #pragma omp parallel
    #pragma omp single
    {
        for (i = 0; i < runtime_bound; i += 100) {
            #pragma omp task reduction(+:task_sum) \
                if(i < runtime_bound/2)  /* Mixed conditions */
            {
                for (int j = i; j < i + 100 && j < runtime_bound; j++) {
                    task_sum += data_array[j] % 13;
                }
            }
        }
    }
    
    printf("Task reduction sum: %d\n", task_sum);
    
    /* 6. SIMD with multiple clauses */
    double fp_sum = 0.0;
    volatile double *fp_sink = &fp_sum;  /* Force memory operations */
    
    #pragma omp simd reduction(+:fp_sum) aligned(data_array:64) \
        linear(i:1)
    for (i = 0; i < runtime_bound; i++) {
        fp_sum += (double)data_array[i] / 1000.0;
    }
    
    *fp_sink = fp_sum;  /* Ensure side effect */
    printf("FP sum: %f\n", fp_sum);
    
    /* 7. Combined construct with many clauses */
    int combined_sum = 0;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: combined_sum) \
        reduction(+:combined_sum) \
        if(target: argc > 1) \
        num_teams(4) \
        thread_limit(64)
    for (i = 0; i < runtime_bound; i++) {
        combined_sum += data_array[i] % 23;
    }
    
    printf("Combined target sum: %d\n", combined_sum);
    
    return 0;
}
