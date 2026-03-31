/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int global_array[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd : int : omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create non-trivial conditions */
int check_threshold(int val) {
    volatile int limit = 500; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char *argv[]) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    
    /* Use argc for runtime-dependent behavior */
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 100;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: global_array[0:N])
    
    /* Complex target region with reduction and if clause 
       (may generate _reductemp_ and _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2) /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        sum += global_array[i];
        if (global_array[i] > max_val) max_val = global_array[i];
    }
    
    /* Print to prevent elimination */
    printf("Target region: sum = %d, max = %d\n", sum, max_val);
    
    /* Nested reductions with custom operator 
       (may generate multiple _reductemp_) */
    sum = 0;
    min_val = 1000;
    #pragma omp parallel for reduction(myadd:sum) reduction(min:min_val) \
        if(parallel: check_threshold(N)) /* Function call in condition */
    for (i = 0; i < N; i++) {
        int val = i % 50;
        sum += val;
        if (val < min_val) min_val = val;
    }
    
    printf("Nested reductions: sum = %d, min = %d\n", sum, min_val);
    
    /* Scan directive (triggers OMP_CLAUSE__SCANTEMP_) */
    int partial_sums[1000];
    scan_sum = 0;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10) /* Uneven scheduling */
    for (i = 0; i < N; i++) {
        int val = (i * 3) % 47;
        
        /* Exclusive scan */
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] = scan_sum;
        scan_sum += val;
    }
    
    /* Print scan results */
    printf("Scan results: final sum = %d, partial_sums[0]=%d, [999]=%d\n", 
           scan_sum, partial_sums[0], partial_sums[N-1]);
    
    /* Array reduction (OpenMP 5.1) - may generate additional temporaries */
    int arr_sum[5] = {0, 0, 0, 0, 0};
    #pragma omp parallel for reduction(+:arr_sum[:5])
    for (i = 0; i < N; i++) {
        arr_sum[i % 5] += global_array[i];
    }
    
    printf("Array reduction: ");
    for (i = 0; i < 5; i++) printf("%d ", arr_sum[i]);
    printf("\n");
    
    /* Nowait clause with volatile to prevent optimization */
    volatile int sink = 0;
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < N/2; i++) {
            sink += i;
        }
        
        #pragma omp for nowait
        for (i = N/2; i < N; i++) {
            sink -= i;
        }
    }
    
    /* Exit data from target */
    #pragma omp target exit data map(from: global_array[0:N])
    
    return 0;
}
