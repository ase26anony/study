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
#pragma omp declare reduction(myadd: int: omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create runtime-dependent conditions */
int get_threshold(int argc) {
    volatile int v = argc; /* volatile to prevent optimization */
    return v * 100;
}

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    volatile int sink; /* Prevent dead code elimination */
    
    /* Initialize array on host */
    for (i = 0; i < N; i++) {
        target_data[i] = i + 1;
    }
    
    /* Enter data to device - triggers OMP_CLAUSE_ENTER */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex reduction with if clause - may generate _reductemp_ and _condtemp_ */
    int threshold = get_threshold(argc);
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) reduction(min:min_val) \
        if(target: argc > 1) map(tofrom: sum, max_val, min_val)
    for (i = 0; i < N; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) max_val = target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    sink = sum + max_val + min_val; /* Use results */
    
    /* Scan directive - triggers _scantemp_ */
    int arr[100];
    for (i = 0; i < 100; i++) arr[i] = 1;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10) /* Uneven scheduling for complexity */
    for (i = 0; i < 100; i++) {
        scan_sum += arr[i];
        #pragma omp scan inclusive(scan_sum)
        arr[i] = scan_sum;
    }
    
    sink += scan_sum; /* Use result */
    
    /* Nested reductions with custom operator */
    int custom_sum = 0;
    #pragma omp parallel for reduction(myadd:custom_sum) \
        if(parallel: threshold > 50) /* Another condition */
    for (i = 0; i < N; i++) {
        custom_sum += (target_data[i] % 10);
    }
    
    /* Array reduction (OpenMP 5.1) - may generate additional temporaries */
    int partial_sums[4] = {0, 0, 0, 0};
    #pragma omp parallel for reduction(+:partial_sums[:4]) \
        num_threads(4)
    for (i = 0; i < N; i++) {
        partial_sums[i % 4] += target_data[i];
    }
    
    /* Final reduction from array */
    int final_sum = 0;
    for (i = 0; i < 4; i++) {
        final_sum += partial_sums[i];
    }
    
    /* Nowait clause for additional complexity */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < 10; i++) {
            /* Some computation */
        }
        #pragma omp barrier
    }
    
    /* Exit data from device */
    #pragma omp target exit data map(from: target_data[0:N])
    
    /* Print results to ensure side effects */
    printf("Results: sum=%d, max=%d, min=%d, scan=%d, custom=%d, final=%d\n",
           sum, max_val, min_val, scan_sum, custom_sum, final_sum);
    
    return 0;
}
