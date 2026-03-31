/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test_omp test_omp_internal_clauses.c */

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
int check_threshold(volatile int val) {
    return val > 100;
}

int main(int argc, char *argv[]) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    volatile int cond_temp = 0; /* Force memory operations */
    int scan_sum = 0;
    int array_sum[100] = {0};
    
    /* Runtime-dependent iteration count to prevent optimization */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Initialize data */
    for (i = 0; i < n; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:n])
    
    /* Complex reduction with if clause - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(argc > 2) num_teams(4) thread_limit(64)
    for (i = 0; i < n; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) max_val = target_data[i];
    }
    
    /* Store result to volatile to prevent elimination */
    cond_temp = sum;
    
    /* Nested reductions with custom operator - more temporaries */
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        if(check_threshold(n))  /* Non-trivial condition */
    for (i = 0; i < n; i++) {
        int val = target_data[i];
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    /* Array reduction - complex case */
    #pragma omp parallel for reduction(vec_add : array_sum)
    for (i = 0; i < n; i++) {
        int idx = i % 100;
        array_sum[idx] += target_data[i];
    }
    
    /* Scan directive - triggers _scantemp_ */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(dynamic, 16)  /* Dynamic scheduling adds complexity */
    for (i = 0; i < n; i++) {
        #pragma omp scan inclusive(scan_sum)
        scan_sum += target_data[i];
    }
    
    /* Combined directive with multiple clauses */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) if(n > 500) collapse(2) \
        map(tofrom: sum)
    for (i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            sum += target_data[i*2 + j];
        }
    }
    
    /* Nowait clause creates async temporaries */
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            for (i = 0; i < n/4; i++) sum += target_data[i];
        }
        #pragma omp section nowait
        {
            for (i = n/4; i < n/2; i++) sum += target_data[i];
        }
    }
    
    /* Print results to ensure side effects */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d\n", 
           sum, max_val, min_val, scan_sum);
    printf("Array_sum[0]=%d, [99]=%d\n", array_sum[0], array_sum[99]);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:n])
    
    return 0;
}
