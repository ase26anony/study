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

void process_with_scan(int* data, int n, int argc) {
    int scan_sum = 0;
    volatile int sink = 0; /* Prevent optimization */
    
    /* OMP_CLAUSE__SCANTEMP_ - explicit scan directive */
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
            simdlen(4) if(argc > 3) /* Also triggers _condtemp_ */
    for (int i = 0; i < n; i++) {
        scan_sum += data[i];
        #pragma omp scan inclusive(scan_sum)
        data[i] = scan_sum;
        sink = data[i]; /* Force side effect */
    }
    
    printf("Scan sum: %d\n", scan_sum);
}

int main(int argc, char** argv) {
    const int N = (argc > 1) ? atoi(argv[1]) : 1000;
    volatile int iter_limit = N; /* Prevent constant folding */
    
    int* array = (int*)malloc(N * sizeof(int));
    int* array2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays with non-trivial pattern */
    #pragma omp parallel for simd if(argc > 2) /* _condtemp_ */
    for (int i = 0; i < N; i++) {
        array[i] = i % 100;
        array2[i] = (i * 3) % 97;
    }
    
    /* Complex reduction triggering _reductemp_ */
    int sum1 = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    volatile int cond = (argc > 1);
    
    /* Multiple reductions in single construct - likely generates _reductemp_ */
    #pragma omp target teams distribute parallel for simd \
            reduction(+:sum1, sum2) \
            reduction(max:max_val) reduction(min:min_val) \
            map(to: array[0:N]) map(from: sum1, sum2) \
            if(cond) /* _condtemp_ */ \
            num_teams(4) thread_limit(64)
    for (int i = 0; i < iter_limit; i++) {
        sum1 += array[i];
        sum2 += array2[i % N];
        if (array[i] > max_val) max_val = array[i];
        if (array[i] < min_val) min_val = array[i];
    }
    
    printf("Reduction results: sum1=%d, sum2=%d, max=%d, min=%d\n", 
           sum1, sum2, max_val, min_val);
    
    /* Array reduction - may generate additional temporaries */
    int arr_red[100] = {0};
    #pragma omp parallel for reduction(vec_add : arr_red) \
            if(argc > 4) /* _condtemp_ */
    for (int i = 0; i < N; i++) {
        arr_red[i % 100] += array[i];
    }
    
    /* Nested parallelism with reduction */
    #pragma omp parallel if(argc > 5) /* Outer _condtemp_ */
    {
        int local_sum = 0;
        #pragma omp for reduction(+:local_sum) nowait
        for (int i = 0; i < N; i++) {
            local_sum += array[i];
        }
        #pragma omp atomic
        sum1 += local_sum;
    }
    
    /* Process with scan (triggers _scantemp_) */
    process_with_scan(array, (N > 100) ? 100 : N, argc);
    
    /* Use global_data with enter clause */
    #pragma omp target update to(global_data[0:100])
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: global_data[0:100]) \
            if(argc > 6) /* _condtemp_ */
    for (int i = 0; i < 100; i++) {
        global_data[i] += i;
    }
    
    /* Final reduction with volatile to force memory ops */
    volatile int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) \
            if(final_sum == 0) /* Non-trivial condition for _condtemp_ */
    for (int i = 0; i < N; i++) {
        final_sum += array[i];
    }
    
    printf("Final sum: %d\n", (int)final_sum);
    
    free(array);
    free(array2);
    
    return 0;
}
