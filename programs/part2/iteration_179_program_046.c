/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test_omp test_omp_clauses.c */

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

/* Function to create non-trivial conditions */
int check_threshold(int val) {
    volatile int limit = 100; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    volatile int sink; /* Prevent dead code elimination */
    
    /* Runtime-dependent iteration count */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Initialize data */
    for (i = 0; i < 1000; i++) {
        target_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:1000])
    
    /* Complex target region with reduction and condition 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 1) /* Non-trivial condition */
    for (i = 0; i < n; i++) {
        int val = target_data[i % 1000];
        sum += val;
        if (val > max_val) max_val = val;
    }
    
    sink = sum; /* Use result */
    printf("Target reduction sum: %d, max: %d\n", sum, max_val);
    
    /* Reset for next test */
    sum = 0;
    
    /* SIMD scan directive (triggers _scantemp_) */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 16) /* Uneven scheduling */
    for (i = 0; i < n; i++) {
        int val = i % 50;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        /* Store scan result to prevent optimization */
        if (i % 100 == 0) sink = scan_sum;
    }
    
    printf("Scan sum: %d\n", scan_sum);
    
    /* Nested parallel region with multiple reductions and complex condition */
    #pragma omp parallel sections reduction(+:sum) reduction(min:min_val) \
        if(parallel: check_threshold(n)) /* Function call in condition */
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                sum += i;
                if (i < min_val) min_val = i;
            }
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                sum += i;
                if (i < min_val) min_val = i;
            }
        }
    }
    
    printf("Section reduction sum: %d, min: %d\n", sum, min_val);
    
    /* Array reduction with custom reduction operator */
    #pragma omp parallel for reduction(vec_add: array_sum) \
        if(parallel: n > 500) /* Another condition */
    for (i = 0; i < n; i++) {
        array_sum[i % 100] += i % 10;
    }
    
    /* Use array result */
    int total = 0;
    for (i = 0; i < 100; i++) {
        total += array_sum[i];
    }
    sink = total;
    printf("Array reduction total: %d\n", total);
    
    /* Nowait clause creates additional complexity */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < n; i++) {
            sum += target_data[i % 1000];
        }
        
        #pragma omp for reduction(max:max_val)
        for (i = 0; i < n; i++) {
            int val = target_data[i % 1000] * 2;
            if (val > max_val) max_val = val;
        }
    }
    
    printf("Final sum: %d, final max: %d\n", sum, max_val);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:1000])
    
    return 0;
}
