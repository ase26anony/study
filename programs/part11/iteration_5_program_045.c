/* tree-pretty-print coverage test for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
   OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1000
#define M 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(double *data, int n, double *results) {
    double sum = 0.0;
    double max_val = -INFINITY;
    
    /* TARGET: OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR combined */
    /* This should trigger pretty-printing of both clauses */
    #pragma omp parallel for schedule(dynamic) reduction(+:sum) if(n > 100)
    for (int i = 0; i < n; i++) {
        data[i] = sin(i * 0.01) * cos(i * 0.005);
        sum += data[i];
        
        /* Nested OpenMP construct with complex clause combination */
        if (i % 100 == 0) {
            /* TARGET: OMP_CLAUSE_FOR with arguments */
            /* This creates a non-trivial clause representation */
            #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
                private(j) lastprivate(k) if(omp_get_num_threads() > 1)
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 10; k++) {
                    data[i] += 0.001 * (j + k);
                }
            }
        }
    }
    results[0] = sum;
    
    /* TARGET: OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS combined */
    /* Nested within switch to test scope handling */
    switch ((int)sum % 3) {
        case 0: {
            #pragma omp parallel sections private(sum) shared(max_val, data, n) \
                if(omp_in_parallel())
            {
                /* TARGET: OMP_CLAUSE_SECTIONS with multiple section blocks */
                #pragma omp section
                {
                    double local_sum = 0.0;
                    for (int i = 0; i < n/2; i++) {
                        local_sum += data[i];
                    }
                    #pragma omp atomic
                    sum += local_sum;
                }
                
                #pragma omp section
                {
                    double local_max = -INFINITY;
                    for (int i = n/2; i < n; i++) {
                        if (data[i] > local_max) local_max = data[i];
                    }
                    #pragma omp critical
                    {
                        if (local_max > max_val) max_val = local_max;
                    }
                }
                
                /* Additional section with nested directive */
                #pragma omp section
                {
                    /* Force diagnostic with clause name in message */
                    #pragma omp error severity(warning) message("Processing section with for clause")
                    for (int i = 0; i < 10; i++) {
                        data[i] *= 1.01;
                    }
                }
            }
            results[1] = max_val;
            results[2] = sum;
            break;
        }
        default:
            results[1] = 0.0;
            results[2] = 0.0;
    }
}

/* Another function to test taskgroup clause */
void __attribute__((optimize("O0"), noinline))
process_with_taskgroup(double *data, int n, double *result) {
    double total = 0.0;
    
    /* TARGET: OMP_CLAUSE_TASKGROUP with task_reduction argument */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (int i = 0; i < n; i += n/10) {
                    int start = i;
                    int end = (i + n/10 < n) ? i + n/10 : n;
                    
                    /* Create tasks with dependency */
                    #pragma omp task firstprivate(start, end) shared(data, total) \
                        depend(out: data[start]) priority(1)
                    {
                        double partial = 0.0;
                        for (int j = start; j < end; j++) {
                            partial += sqrt(fabs(data[j]));
                        }
                        
                        /* Nested task with error directive */
                        #pragma omp task shared(total, partial) if(0)
                        {
                            /* Diagnostic that should trigger pretty-printing */
                            _Pragma("omp error severity(message) message(\"Task with for clause\")")
                            #pragma omp atomic
                            total += partial;
                        }
                    }
                }
                
                /* Taskwait to ensure all tasks complete */
                #pragma omp taskwait
            }
        }
    }
    
    *result = total;
    
    /* Complex macro expansion with _Pragma to test pretty-printer */
    #define CREATE_PARALLEL_FOR(iter, limit) \
        _Pragma("omp parallel for schedule(guided) num_threads(4)") \
        for (iter = 0; iter < limit; iter++)
    
    int idx;
    CREATE_PARALLEL_FOR(idx, 100)
    {
        data[idx % n] += 0.001 * idx;
    }
}

/* Function with mixed OpenMP and complex control flow */
void __attribute__((optimize("O0")))
nested_openmp_test(double *arr, int size) {
    /* Loop with embedded OpenMP directives */
    for (int outer = 0; outer < 5; outer++) {
        if (outer % 2 == 0) {
            /* TARGET: Multiple clauses in one directive */
            #pragma omp parallel for simd schedule(nonmonotonic:dynamic) \
                aligned(arr:64) if(size > 500) nontemporal(arr)
            for (int i = 0; i < size; i++) {
                arr[i] = arr[i] * 1.1 + outer * 0.01;
            }
        } else {
            /* TARGET: sections clause with reduction */
            #pragma omp parallel sections reduction(+:outer) \
                proc_bind(spread)
            {
                #pragma omp section
                {
                    for (int i = 0; i < size/2; i++) {
                        arr[i] = sin(arr[i]);
                    }
                }
                #pragma omp section
                {
                    for (int i = size/2; i < size; i++) {
                        arr[i] = cos(arr[i]);
                    }
                }
            }
        }
        
        /* Conditional compilation with OpenMP */
        #ifdef _OPENMP
        #pragma omp error severity(warning) message("Iteration with parallel for clauses")
        #endif
    }
}

int main() {
    double *data = (double*)malloc(N * sizeof(double));
    double results[5] = {0};
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some values */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
        data[i] = i * 0.01;
    }
    
    /* Test 1: Process with parallel for and sections */
    process_with_openmp(data, N, results);
    
    /* Test 2: Process with taskgroup */
    process_with_taskgroup(data, N, &results[3]);
    
    /* Test 3: Nested OpenMP test */
    nested_openmp_test(data, N);
    
    /* Final computation to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) \
        if(N > 100) num_threads(omp_get_max_threads())
    for (int i = 0; i < N; i++) {
        checksum += data[i];
    }
    
    /* Use results to prevent optimization */
    results[4] = checksum;
    
    printf("Results: ");
    for (int i = 0; i < 5; i++) {
        printf("%.6f ", results[i]);
    }
    printf("\nChecksum: %.6f\n", checksum);
    
    free(data);
    
    /* Final error directive with all target clause names */
    #pragma omp error severity(message) \
        message("Coverage test complete: for, parallel, sections, taskgroup")
    
    return 0;
}
