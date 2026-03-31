/* test_omp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define SIZE 1000
#define CHUNK 64

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"), noinline)) 
process_with_combined_directives(double *arr, int n, double *results) {
    double sum = 0.0, max_val = -INFINITY;
    int i, j;
    
    /* 1. Combined parallel for directive - triggers 'for' and 'parallel' clauses */
    #pragma omp parallel for simd schedule(static, CHUNK) collapse(2) \
            private(i, j) reduction(+:sum) if(n > 100)
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double val = arr[i] * arr[j] / (i + j + 1.0);
            sum += val;
        }
    }
    results[0] = sum;
    
    /* 2. Parallel sections with nested constructs */
    #pragma omp parallel sections private(i) reduction(max:max_val) \
            num_threads(4) if(omp_get_max_threads() > 1)
    {
        /* First section with error directive containing clause name */
        #pragma omp section
        {
            for (i = 0; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
            /* Force diagnostic with clause name in message */
            #pragma omp error severity(warning) message("Processing 'for' clause in section 1")
        }
        
        /* Second section with complex nested directive */
        #pragma omp section
        {
            double local_sum = 0.0;
            #pragma omp parallel for simd schedule(dynamic) \
                    reduction(+:local_sum) nowait
            for (i = 0; i < n; i++) {
                local_sum += sqrt(fabs(arr[i]));
            }
            if (local_sum > max_val) max_val = local_sum;
            
            /* Another diagnostic trigger */
            _Pragma("omp error severity(message) message(\"sections clause processed\")")
        }
    }
    results[1] = max_val;
}

/* Function using taskgroup clause */
void __attribute__((optimize("O0"))) 
process_with_taskgroup(double *arr, int n, double *task_result) {
    double sum = 0.0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:sum)
        {
            for (int i = 0; i < n; i += CHUNK) {
                #pragma omp task in_reduction(+:sum) firstprivate(i) \
                        if(i < n/2) final(i >= n - CHUNK)
                {
                    double chunk_sum = 0.0;
                    int end = (i + CHUNK < n) ? i + CHUNK : n;
                    for (int j = i; j < end; j++) {
                        chunk_sum += arr[j] * arr[j];
                    }
                    sum += chunk_sum;
                    
                    /* Nested task with error directive */
                    #pragma omp task if(0)
                    {
                        #pragma omp error severity(warning) \
                                message("taskgroup clause with reduction")
                    }
                }
            }
        }
        
        /* Additional parallel for inside parallel region */
        #pragma omp for schedule(guided) nowait
        for (int i = 0; i < n; i++) {
            arr[i] = sum / (i + 1.0);
        }
    }
    
    *task_result = sum;
}

/* Complex control flow with embedded OpenMP */
double __attribute__((optimize("O0"), noinline))
complex_control_flow(double *arr, int n) {
    double result = 0.0;
    int mode = n % 3;
    
    switch (mode) {
        case 0: {
            /* Distribute parallel for simd with multiple clauses */
            #pragma omp target teams distribute parallel for simd \
                    map(tofrom: arr[0:n]) schedule(static, 4) collapse(2) \
                    if(target: n > 500) num_teams(4) thread_limit(32)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    arr[i] += sin(arr[j]) * cos((double)i/j);
                }
            }
            result = arr[0];
            break;
        }
            
        case 1: {
            /* Nested parallel regions */
            #pragma omp parallel if(n > 1000) default(none) shared(arr, n, result)
            {
                #pragma omp for schedule(runtime) ordered
                for (int i = 0; i < n; i++) {
                    #pragma omp ordered
                    {
                        arr[i] = log(fabs(arr[i]) + 1.0);
                    }
                }
                
                #pragma omp single
                {
                    #pragma omp taskloop grainsize(16) nogroup \
                            in_reduction(+:result)
                    for (int i = 0; i < n; i++) {
                        result += arr[i];
                    }
                }
            }
            break;
        }
            
        case 2: {
            /* Sections with taskgroup inside */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp taskgroup task_reduction(*:result)
                    {
                        #pragma omp task in_reduction(*:result)
                        { result = 1.0; }
                        
                        for (int i = 0; i < n; i++) {
                            #pragma omp task in_reduction(*:result) if(i % 2)
                            { result *= (arr[i] + 1.0); }
                        }
                    }
                }
                
                #pragma omp section
                {
                    /* Mixed directive with distribute */
                    #pragma omp distribute parallel for simd \
                            simdlen(8) safelen(16) linear(i:1)
                    for (int i = 0; i < n; i++) {
                        arr[i] = pow(arr[i], result);
                    }
                }
            }
            break;
        }
    }
    
    return result;
}

int main() {
    double *array = (double*)malloc(SIZE * SIZE * sizeof(double));
    double results[3] = {0.0};
    double task_result = 0.0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with parallel for */
    #pragma omp parallel for schedule(dynamic) \
            if(SIZE > 100) num_threads(omp_get_max_threads())
    for (int i = 0; i < SIZE * SIZE; i++) {
        array[i] = (double)(i % 100) * 0.01 + sin((double)i * 0.1);
    }
    
    /* Process with combined directives */
    process_with_combined_directives(array, SIZE, results);
    
    /* Process with taskgroup */
    process_with_taskgroup(array, SIZE, &task_result);
    
    /* Complex control flow */
    double complex_result = complex_control_flow(array, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = results[0] + results[1] + task_result + complex_result;
    
    /* Final parallel region with error directive */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp error severity(message) \
                    message("Final: for, parallel, sections, taskgroup clauses processed")
        }
        
        #pragma omp for schedule(static) nowait
        for (int i = 0; i < SIZE; i++) {
            checksum += array[i] * 0.001;
        }
    }
    
    printf("Checksum: %.6f\n", checksum);
    
    free(array);
    return 0;
}
