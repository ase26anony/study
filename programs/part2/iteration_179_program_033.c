/* test_omp_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test_omp_clauses.c -o test_omp_clauses */

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
    initializer(omp_priv[i] = 0)

/* Function to create non-trivial conditions */
int check_threshold(int val) {
    volatile int limit = 500; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char *argv[]) {
    int i, j;
    int sum = 0, sum1 = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    volatile int sink; /* Prevent dead code elimination */
    
    /* Runtime-dependent iteration count */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Initialize data */
    for (i = 0; i < n; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to device (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:n])
    
    /* Complex target region with reduction and condition 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        if(argc > 2) /* Non-trivial condition */
    for (i = 0; i < n; i++) {
        sum1 += target_data[i];
        sum2 += target_data[i] * 2;
        if (target_data[i] > max_val) max_val = target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    sink = sum1 + sum2; /* Use results */
    
    /* Scan directive (triggers _scantemp_) */
    int partial_sums[100] = {0};
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        private(j) schedule(dynamic)
    for (i = 0; i < n; i++) {
        int val = target_data[i];
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        partial_sums[i % 100] = scan_sum;
    }
    
    /* Array reduction with custom operator (more complex lowering) */
    int local_arr[100] = {0};
    #pragma omp parallel for reduction(vec_add: array_sum) \
        if(check_threshold(n)) /* Function call in condition */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            array_sum[j] += target_data[i] + j;
        }
    }
    
    /* Nested parallelism with uneven work distribution */
    #pragma omp parallel sections reduction(+:sum)
    {
        #pragma omp section
        {
            #pragma omp parallel for nowait
            for (i = 0; i < n/2; i++) {
                #pragma omp atomic
                sum += target_data[i];
            }
        }
        
        #pragma omp section
        {
            #pragma omp parallel for
            for (i = n/2; i < n; i++) {
                #pragma omp atomic
                sum += target_data[i] * 3;
            }
        }
    }
    
    /* Final reduction with volatile to force temporaries */
    volatile int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) \
        if(n > 1000) /* Always false but not trivially constant */
    for (i = 0; i < n; i++) {
        final_sum += target_data[i] % 10;
    }
    
    /* Exit data from device */
    #pragma omp target exit data map(from: target_data[0:n])
    
    /* Print results to ensure side effects */
    printf("Results: sum1=%d, sum2=%d, max=%d, min=%d\n", 
           sum1, sum2, max_val, min_val);
    printf("Scan sum: %d, Final sum: %d\n", scan_sum, sum);
    printf("Array sum[0]=%d, final_sum=%d\n", array_sum[0], final_sum);
    
    return 0;
}
