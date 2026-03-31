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
int check_threshold(int val, volatile int* limit) {
    return val > *limit;
}

int main(int argc, char** argv) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    volatile int sink = 0;  /* Prevent optimizations */
    
    /* Runtime-dependent iteration count */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Initialize data */
    for (i = 0; i < n; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:n])
    
    /* Complex target region with reduction and condition 
       (may generate _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 1) map(tofrom: sum, max_val)
    for (i = 0; i < n; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) {
            max_val = target_data[i];
        }
    }
    
    sink = sum + max_val;  /* Ensure side effect */
    
    /* Scan directive (OMP_CLAUSE__SCANTEMP_) */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < n; i++) {
        int val = i % 50;
        #pragma omp scan exclusive(scan_sum)
        scan_sum += val;
    }
    
    sink += scan_sum;
    
    /* Nested reductions with custom operator and volatile condition */
    volatile int cond_limit = 500;
    #pragma omp parallel for reduction(+:sum) reduction(min:min_val) \
        if(parallel: check_threshold(n, &cond_limit)) \
        schedule(dynamic, 10)
    for (i = 0; i < n; i++) {
        int val = (i * 17) % 100;
        sum += val;
        if (val < min_val) {
            min_val = val;
        }
    }
    
    sink += sum + min_val;
    
    /* Array reduction (OpenMP 5.1) - may generate additional temporaries */
    #pragma omp parallel for reduction(+: array_sum[:100])
    for (i = 0; i < n; i++) {
        int idx = i % 100;
        array_sum[idx] += i;
    }
    
    /* Mixed reductions in teams region */
    int team_sum = 0, team_max = -1;
    #pragma omp target teams distribute parallel for \
        reduction(+:team_sum) reduction(max:team_max) \
        num_teams(4) thread_limit(64)
    for (i = 0; i < n; i++) {
        team_sum += target_data[i];
        team_max = (target_data[i] > team_max) ? target_data[i] : team_max;
    }
    
    sink += team_sum + team_max;
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:n])
    
    /* Print results to prevent dead code elimination */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d, team_sum=%d\n",
           sum, max_val, min_val, scan_sum, team_sum);
    
    return 0;
}
