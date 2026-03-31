/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all test_omp_internal_clauses.c -o test_omp */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1000
#define M 100

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target enter(data_array)
int data_array[N];

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [M] : \
    for (int i = 0; i < M; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Function to create non-trivial conditions */
int check_threshold(int val) {
    volatile int limit = 500; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char *argv[]) {
    int i, j;
    int sum = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[M] = {0};
    int reduction_temp = 0;
    
    /* Runtime-dependent iteration count */
    int iterations = (argc > 1) ? atoi(argv[1]) : N;
    if (iterations < 10) iterations = N;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data_array[i] = i % 100;
    }
    
    /* ============================================
       Test 1: Multiple reductions with if clause
       Should generate _reductemp_ and _condtemp_
       ============================================ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum, sum2) reduction(max:max_val) \
        if(target: iterations > 100) map(tofrom: sum, sum2, max_val)
    for (i = 0; i < iterations; i++) {
        int val = data_array[i];
        sum += val;
        sum2 += val * val;
        if (val > max_val) max_val = val;
    }
    
    /* Prevent dead code elimination */
    volatile int sink1 = sum;
    volatile int sink2 = sum2;
    
    /* ============================================
       Test 2: Scan directive (OpenMP 5.0/5.1)
       Should generate _scantemp_
       ============================================ */
    int partial_sums[N];
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        private(j) schedule(dynamic)
    for (i = 0; i < iterations; i++) {
        /* Exclusive scan */
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] = scan_sum;
        
        /* Update for next iteration */
        scan_sum += data_array[i];
    }
    
    /* ============================================
       Test 3: Nested reductions with complex condition
       Should generate additional temporaries
       ============================================ */
    volatile int cond_var = argc;
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        if(parallel: check_threshold(iterations)) \
        schedule(nonmonotonic:dynamic)
    for (i = 0; i < iterations; i++) {
        int val = data_array[i] * (i % 7);
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    /* ============================================
       Test 4: Array reduction with custom reducer
       Complex case that may generate temporaries
       ============================================ */
    #pragma omp parallel for reduction(vec_add: array_sum) \
        collapse(2) ordered
    for (i = 0; i < iterations/10; i++) {
        for (j = 0; j < M; j++) {
            array_sum[j] += data_array[i] * j;
        }
    }
    
    /* ============================================
       Test 5: Combined construct with nowait
       Creates scheduling complexity
       ============================================ */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:reduction_temp)
        for (i = 0; i < iterations/2; i++) {
            reduction_temp += data_array[i];
        }
        
        #pragma omp for reduction(*:reduction_temp)
        for (i = iterations/2; i < iterations; i++) {
            reduction_temp *= (data_array[i] + 1);
        }
    }
    
    /* ============================================
       Test 6: Task reduction (OpenMP 5.0+)
       ============================================ */
    int task_sum = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        for (i = 0; i < iterations/10; i++) {
            #pragma omp task in_reduction(+:task_sum)
            {
                task_sum += data_array[i];
            }
        }
    }
    
    /* Output results to ensure side effects */
    printf("Results:\n");
    printf("  sum = %d\n", sum);
    printf("  sum2 = %d\n", sum2);
    printf("  max_val = %d\n", max_val);
    printf("  min_val = %d\n", min_val);
    printf("  scan_sum = %d\n", scan_sum);
    printf("  reduction_temp = %d\n", reduction_temp);
    printf("  task_sum = %d\n", task_sum);
    printf("  array_sum[0] = %d\n", array_sum[0]);
    printf("  partial_sums[10] = %d\n", partial_sums[10]);
    
    return 0;
}
