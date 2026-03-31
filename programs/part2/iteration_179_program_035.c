/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int global_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Function to create non-trivial conditions */
int check_threshold(int val, volatile int* limit) {
    return val > *limit;
}

int main(int argc, char** argv) {
    /* Use argc for runtime-dependent behavior */
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n <= 0) n = 1000;
    
    /* Initialize data */
    for (int i = 0; i < 1000; i++) {
        global_data[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: global_data[0:1000])
    
    int sum = 0;
    int max_val = -1000000;
    int min_val = 1000000;
    volatile int limit = 500;  /* volatile to prevent optimization */
    
    /* Complex reduction with condition - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) reduction(min:min_val) \
        if(argc > 2)  /* Non-trivial condition */
    for (int i = 0; i < n; i++) {
        int val = global_data[i];
        sum += val;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    printf("After target reduction: sum=%d, max=%d, min=%d\n", sum, max_val, min_val);
    
    /* Array reduction - may generate additional temporaries */
    int arr_sum[100] = {0};
    #pragma omp parallel for reduction(vec_add:arr_sum) \
        if(check_threshold(n, &limit))  /* Function call in condition */
    for (int i = 0; i < n; i++) {
        int idx = i % 100;
        arr_sum[idx] += global_data[i];
    }
    
    /* Scan directive - triggers _scantemp_ */
    int scan_sum = 0;
    int scan_values[100];
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10)  /* Uneven scheduling */
    for (int i = 0; i < 100; i++) {
        int val = i * 2;
        scan_values[i] = scan_sum;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
    }
    
    printf("Scan sum: %d\n", scan_sum);
    
    /* Nested parallelism with reductions */
    int outer_sum = 0;
    #pragma omp parallel reduction(+:outer_sum) num_threads(4)
    {
        #pragma omp for nowait  /* nowait creates more complex scheduling */
        for (int i = 0; i < n/2; i++) {
            outer_sum += global_data[i];
        }
        
        #pragma omp for reduction(+:outer_sum)
        for (int i = n/2; i < n; i++) {
            outer_sum += global_data[i] * 2;
        }
    }
    
    printf("Outer sum: %d\n", outer_sum);
    
    /* Final clause with non-trivial condition */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) \
        final(argc > 3)  /* Another condition */
    for (int i = 0; i < n; i++) {
        final_sum += global_data[i] % 10;
    }
    
    printf("Final sum: %d\n", final_sum);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: global_data[0:1000])
    
    return 0;
}
