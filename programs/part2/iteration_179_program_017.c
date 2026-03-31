/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_internal_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Initialize target data */
#pragma omp declare target enter(target_data)

/* Custom reduction for complex cases */
#pragma omp declare reduction(complex_add : double : omp_out += omp_in * omp_in) \
    initializer(omp_priv = 0)

int main(int argc, char **argv) {
    volatile int use_argc = argc; /* Prevent optimization */
    int N = 1000;
    
    /* Adjust N based on runtime to prevent constant folding */
    if (use_argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 100;
    
    /* Initialize arrays with non-trivial patterns */
    int data[1000];
    double ddata[1000];
    for (int i = 0; i < N; i++) {
        data[i] = i * (i % 7);
        ddata[i] = (i % 13) * 0.5;
    }
    
    /* For OMP_CLAUSE__REDUCTEMP_ and OMP_CLAUSE__CONDTEMP_ */
    /* Complex target construct with reduction and conditional */
    long sum = 0;
    volatile long sum_sink = 0;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) \
        if(target: use_argc > 2) \
        map(to: data[0:N]) map(from: sum)
    for (int i = 0; i < N; i++) {
        sum += data[i];
    }
    sum_sink = sum; /* Force side effect */
    
    /* Multiple reductions in single construct */
    int max_val = -1000, min_val = 1000;
    volatile int max_sink = 0, min_sink = 0;
    
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        if(parallel: use_argc > 1)
    for (int i = 0; i < N; i++) {
        if (data[i] > max_val) max_val = data[i];
        if (data[i] < min_val) min_val = data[i];
    }
    max_sink = max_val;
    min_sink = min_val;
    
    /* Array reduction (OpenMP 5.1) - may generate complex temporaries */
    double arr_sum[10] = {0};
    volatile double arr_sink = 0;
    
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (int i = 0; i < N; i++) {
        arr_sum[i % 10] += ddata[i];
    }
    for (int j = 0; j < 10; j++) arr_sink += arr_sum[j];
    
    /* For OMP_CLAUSE__SCANTEMP_ */
    /* Scan directive with inscan reduction */
    double scan_sum = 0.0;
    double scan_results[1000];
    volatile double scan_sink = 0;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (int i = 0; i < N; i++) {
        scan_sum += ddata[i];
        #pragma omp scan inclusive(scan_sum)
        scan_results[i] = scan_sum;
    }
    scan_sink = scan_results[N-1];
    
    /* Nested parallelism with custom reduction */
    double custom_reduce = 0.0;
    volatile double custom_sink = 0;
    
    #pragma omp parallel sections reduction(complex_add:custom_reduce)
    {
        #pragma omp section
        {
            #pragma omp parallel for reduction(complex_add:custom_reduce) \
                if(parallel: use_argc > 3)
            for (int i = 0; i < N/2; i++) {
                custom_reduce += ddata[i];
            }
        }
        
        #pragma omp section
        {
            #pragma omp parallel for reduction(complex_add:custom_reduce) \
                if(parallel: use_argc > 3)
            for (int i = N/2; i < N; i++) {
                custom_reduce += ddata[i];
            }
        }
    }
    custom_sink = custom_reduce;
    
    /* Final reduction with nowait to create scheduling complexity */
    int final_sum = 0;
    volatile int final_sink = 0;
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:final_sum) nowait
        for (int i = 0; i < N; i++) {
            final_sum += data[i] % 17;
        }
        
        #pragma omp barrier
        
        #pragma omp single
        {
            final_sink = final_sum;
        }
    }
    
    /* Use target_data to ensure enter clause is processed */
    #pragma omp target update to(target_data[0:100])
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: target_data[0:100])
    for (int i = 0; i < 100; i++) {
        target_data[i] += i;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %ld %d %d %f %f %f %d\n", 
           sum_sink, max_sink, min_sink, arr_sink, 
           scan_sink, custom_sink, final_sink);
    
    return 0;
}
