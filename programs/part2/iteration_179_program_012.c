/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int global_array[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Function to create non-trivial condition */
int check_threshold(int val) {
    volatile int limit = 500; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    int N = 1000;
    
    /* Runtime-dependent size to prevent optimization */
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 1000;
    
    /* Initialize arrays */
    for (i = 0; i < 1000; i++) {
        global_array[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: global_array[0:1000])
    
    /* Complex target region with reduction and condition 
       (triggers _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 1) /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        int val = global_array[i % 1000];
        sum += val;
        if (val > max_val) max_val = val;
    }
    
    /* Use results to prevent dead code elimination */
    volatile int sink1 = sum;
    volatile int sink2 = max_val;
    
    /* Scan directive (triggers _scantemp_) */
    int data[100];
    for (i = 0; i < 100; i++) data[i] = i;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < 100; i++) {
        scan_sum += data[i];
        #pragma omp scan inclusive(scan_sum)
        data[i] = scan_sum;
    }
    
    /* Nested reductions with custom reduction */
    #pragma omp parallel for reduction(vec_add:array_sum) \
        reduction(min:min_val)
    for (i = 0; i < N; i++) {
        int idx = i % 100;
        array_sum[idx] += i;
        if (i < min_val) min_val = i;
    }
    
    /* Complex condition with function call (more likely _condtemp_) */
    #pragma omp parallel for if(check_threshold(N)) \
        reduction(+:sum)
    for (i = 0; i < N; i++) {
        sum += i % 10;
    }
    
    /* Nowait clause with uneven work distribution */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < N; i++) {
            sum += (i * 2) % 7;
        }
        
        #pragma omp for reduction(max:max_val)
        for (i = 0; i < N/2; i++) {
            if (i > max_val) max_val = i;
        }
    }
    
    /* Array reduction (OpenMP 5.1) */
    int arr[50] = {0};
    #pragma omp parallel for reduction(+:arr[:50])
    for (i = 0; i < N; i++) {
        arr[i % 50] += 1;
    }
    
    /* Exit target data */
    #pragma omp target exit data map(from: global_array[0:1000])
    
    /* Print results to ensure side effects */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d\n", 
           sum, max_val, min_val, scan_sum);
    printf("Array sums: %d %d %d\n", array_sum[0], array_sum[1], array_sum[2]);
    printf("Arr reduction: %d %d\n", arr[0], arr[1]);
    
    return 0;
}
