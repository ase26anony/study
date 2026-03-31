/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int global_data[1000];
#pragma omp end declare target

/* Initialize global data on target device */
#pragma omp declare target enter(global_data)

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

void process_results(volatile int *sink, int value) {
    *sink = value;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n <= 0) n = 1000;
    
    volatile int sink1 = 0, sink2 = 0, sink3 = 0;
    int sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum[100] = {0};
    
    /* Initialize data with non-trivial pattern */
    for (int i = 0; i < n; i++) {
        global_data[i] = i * (i % 7) - (i % 13);
    }
    
    /* 1. Complex target construct with reduction and if clause 
       Likely generates _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum, max_val) \
        reduction(+:sum) reduction(max:max_val) \
        if(argc > 2)  /* Non-trivial condition */
    for (int i = 0; i < n; i++) {
        sum += global_data[i];
        if (global_data[i] > max_val)
            max_val = global_data[i];
    }
    
    process_results(&sink1, sum);
    process_results(&sink2, max_val);
    
    /* 2. SIMD with inscan reduction - should generate _scantemp_ */
    #pragma omp parallel for simd \
        reduction(inscan, +:scan_sum) \
        num_threads(4)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(scan_sum)
        scan_sum += i % 17;
    }
    
    /* 3. Nested reductions with custom operator and volatile 
       Increases chance of temporary generation */
    volatile int volatile_limit = 500;
    #pragma omp parallel for \
        reduction(max:max_val) reduction(min:min_val) \
        if(volatile_limit > 250)  /* Volatile condition */
    for (int i = 0; i < n; i++) {
        int val = global_data[i] * 2;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    /* 4. Array reduction with custom declared reduction
       Complex case that may need temporaries */
    #pragma omp parallel for reduction(vec_add:array_sum)
    for (int i = 0; i < n; i++) {
        int idx = i % 100;
        array_sum[idx] += global_data[i] % 11;
    }
    
    /* 5. Nowait clause with uneven work distribution
       Creates complex scheduling that may need temporaries */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for nowait reduction(+:sum)
        for (int i = 0; i < n/2; i++) {
            sum += global_data[i];
        }
        
        #pragma omp for reduction(+:sum)
        for (int i = n/2; i < n; i++) {
            sum += global_data[i] * 2;
        }
    }
    
    /* 6. Final clause with function call in condition
       May generate condition temporaries */
    int final_threshold = 100;
    #pragma omp parallel for final(argc > final_threshold) \
        reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += global_data[i] % 5;
    }
    
    /* Ensure all results are used */
    int total = sum + max_val + min_val + scan_sum + array_sum[0];
    printf("Results: sum=%d, max=%d, min=%d, scan=%d, total=%d\n",
           sum, max_val, min_val, scan_sum, total);
    
    return 0;
}
