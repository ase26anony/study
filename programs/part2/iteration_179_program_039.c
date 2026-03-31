/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test_omp_internal_clauses.c -o test_omp */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int[100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void process_with_scan(int* data, int n, int argc) {
    int scan_sum = 0;
    volatile int sink = 0;
    
    /* OMP_CLAUSE__SCANTEMP_ should be generated here */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
            if(argc > 3)  /* Add condition for _condtemp_ */
    for (int i = 0; i < n; i++) {
        scan_sum += data[i];
        #pragma omp scan inclusive(scan_sum)
        data[i] = scan_sum;
    }
    
    sink = scan_sum;  /* Prevent dead code elimination */
}

int main(int argc, char** argv) {
    const int N = 1000;
    int data[N];
    int sum = 0, max_val = INT_MIN, min_val = INT_MAX;
    volatile int result_sink = 0;
    
    /* Initialize data with non-trivial pattern */
    for (int i = 0; i < N; i++) {
        data[i] = (i * 3) % 7;
    }
    
    /* Trigger OMP_CLAUSE_ENTER */
    #pragma omp target enter data map(to: target_data[0:1000])
    
    /* Complex reduction with multiple clauses - may generate _reductemp_ */
    #pragma omp target teams distribute parallel for simd \
            reduction(+:sum) reduction(max:max_val) \
            if(argc > 1)  /* Condition for _condtemp_ */
    for (int i = 0; i < N; i++) {
        sum += data[i];
        if (data[i] > max_val) max_val = data[i];
    }
    
    result_sink = sum + max_val;
    
    /* Nested reduction with custom operator - may generate more temporaries */
    int arr_reduce[100] = {0};
    #pragma omp declare target enter(arr_reduce)
    
    #pragma omp target teams distribute parallel for simd \
            reduction(vec_add:arr_reduce) \
            if(argc > 2)  /* Another condition */
    for (int i = 0; i < N; i++) {
        arr_reduce[i % 100] += data[i];
    }
    
    /* Array reduction (OpenMP 5.1) */
    int array_sum[10] = {0};
    #pragma omp parallel for reduction(+:array_sum[:10]) \
            if(argc > 4)
    for (int i = 0; i < N; i++) {
        array_sum[i % 10] += data[i];
    }
    
    /* Scan operation - explicit _scantemp_ generation */
    process_with_scan(data, (argc > 1) ? N : N/2, argc);
    
    /* Final clause with volatile condition */
    volatile int final_flag = 1;
    #pragma omp parallel for final(final_flag > 0) \
            reduction(min:min_val)
    for (int i = 0; i < N; i++) {
        if (data[i] < min_val) min_val = data[i];
    }
    
    /* Combined directives */
    #pragma omp target teams distribute parallel for simd \
            collapse(2) reduction(+:sum) \
            if(argc > 5) schedule(dynamic)
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            sum += i * j;
        }
    }
    
    /* Output to prevent optimization */
    printf("Results: sum=%d, max=%d, min=%d\n", sum, max_val, min_val);
    printf("Array sum[0]=%d, scan result=%d\n", array_sum[0], data[N-1]);
    
    return 0;
}
