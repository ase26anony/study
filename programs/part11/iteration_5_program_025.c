/* test_omp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define CHUNK_SIZE 64

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_combined_directives(double *arr, int n) {
    double sum = 0.0;
    double max_val = -1e30;
    
    /* Combined parallel and for clause - targets OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) reduction(+:sum) if(n > 1000)
    for (int i = 0; i < n; i++) {
        arr[i] = sin(i * 0.01) + cos(i * 0.005);
        sum += arr[i];
        
        /* Nested OpenMP directive with for clause in different context */
        if (i % 100 == 0) {
            #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
                private(i) /* This creates OMP_CLAUSE_FOR with arguments */
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 10; k++) {
                    /* Dummy computation */
                    double tmp = j * k * 0.1;
                    arr[i] += tmp;
                }
            }
        }
    }
    
    /* Combined parallel and sections clause - targets OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(sum) reduction(max:max_val) \
        num_threads(omp_get_max_threads() / 2 + 1)
    {
        /* First section */
        #pragma omp section
        {
            sum = 0.0;
            for (int i = 0; i < n/2; i++) {
                if (arr[i] > max_val) max_val = arr[i];
                sum += arr[i];
            }
            printf("Section 1: sum = %f, max = %f\n", sum, max_val);
        }
        
        /* Second section with nested OpenMP */
        #pragma omp section
        {
            sum = 0.0;
            #pragma omp parallel for schedule(guided) if(n > 500)
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
                sum += arr[i];
                
                /* Force diagnostic with clause name in message */
                if (i == n/2 + 1) {
                    #pragma omp error severity(warning) message("Processing for clause in section")
                }
            }
            printf("Section 2: sum = %f, max = %f\n", sum, max_val);
        }
        
        /* Third section using _Pragma for complex pattern */
        #pragma omp section
        {
            #define EMIT_PARALLEL_FOR _Pragma("omp parallel for schedule(static)")
            EMIT_PARALLEL_FOR
            for (int i = 0; i < 100; i++) {
                /* Dummy work */
                arr[i % n] += 0.001 * i;
            }
        }
    }
}

/* Function specifically for taskgroup clause */
double __attribute__((optimize("O0"))) task_based_computation(double *arr, int n) {
    double total = 0.0;
    double partial_sums[4] = {0.0};
    
    /* Taskgroup with explicit task_reduction clause - targets OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (int t = 0; t < 4; t++) {
                    #pragma omp task in_reduction(+:total) firstprivate(t) \
                        depend(out: partial_sums[t])
                    {
                        double local_sum = 0.0;
                        int start = t * (n / 4);
                        int end = (t == 3) ? n : (t + 1) * (n / 4);
                        
                        for (int i = start; i < end; i++) {
                            local_sum += arr[i] * arr[i];
                        }
                        partial_sums[t] = local_sum;
                        
                        /* Nested task with error directive containing clause name */
                        #pragma omp task if(0)
                        {
                            #pragma omp error severity(message) \
                                message("Inside taskgroup clause context")
                        }
                    }
                }
                
                /* Tasks with depend clauses */
                #pragma omp task depend(in: partial_sums[0], partial_sums[1], \
                                         partial_sums[2], partial_sums[3]) \
                    in_reduction(+:total)
                {
                    for (int t = 0; t < 4; t++) {
                        total += partial_sums[t];
                    }
                    
                    /* Complex macro expansion with clause names */
                    #define EMIT_SECTIONS _Pragma("omp sections")
                    #define EMIT_SECTION _Pragma("omp section")
                    
                    EMIT_SECTIONS
                    {
                        EMIT_SECTION
                        { /* Empty but triggers sections clause */ }
                        EMIT_SECTION
                        { /* Empty but triggers sections clause */ }
                    }
                }
            }
        }
    }
    
    return total;
}

/* Complex control flow with mixed OpenMP directives */
void __attribute__((optimize("O0"))) nested_control_flow(double *arr, int n) {
    int i;
    
    /* Switch statement with OpenMP inside cases */
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0:
                /* Parallel for in switch case */
                #pragma omp parallel for schedule(runtime) \
                    if(omp_in_parallel()) ordered
                for (i = 0; i < n; i += 2) {
                    arr[i] = sqrt(fabs(arr[i]));
                }
                break;
                
            case 1:
                /* Sections in switch case */
                #pragma omp parallel sections lastprivate(i)
                {
                    #pragma omp section
                    {
                        for (i = 0; i < n/3; i++) {
                            arr[i] = pow(arr[i], 1.5);
                        }
                    }
                    #pragma omp section
                    {
                        for (i = n/3; i < 2*n/3; i++) {
                            arr[i] = log1p(fabs(arr[i]));
                        }
                    }
                    #pragma omp section
                    {
                        for (i = 2*n/3; i < n; i++) {
                            arr[i] = arr[i] * 0.5 + 1.0;
                        }
                    }
                }
                break;
                
            case 2:
                /* Taskgroup in switch case */
                #pragma omp parallel
                {
                    #pragma omp single
                    {
                        #pragma omp taskgroup
                        {
                            #pragma omp task
                            {
                                /* Force diagnostic with all clause names */
                                #pragma omp error severity(warning) \
                                    message("Clauses: for, parallel, sections, taskgroup")
                            }
                        }
                    }
                }
                break;
        }
    }
}

int main() {
    double *array = (double*)malloc(N * sizeof(double));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double checksum = 0.0;
    
    /* Initialize array with parallel for */
    #pragma omp parallel for schedule(static) if(N > 1000)
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
    }
    
    /* Process with combined directives */
    process_with_combined_directives(array, N);
    
    /* Task-based computation */
    double task_result = task_based_computation(array, N);
    checksum += task_result;
    
    /* Nested control flow */
    nested_control_flow(array, N);
    
    /* Final computation with various OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < N; i++) {
            array[i] = array[i] * 0.99;
        }
        
        #pragma omp barrier
        
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    checksum += array[0];
                }
            }
            #pragma omp section
            {
                #pragma omp task
                checksum += array[N-1];
            }
        }
    }
    
    /* Final reduction */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) schedule(dynamic)
    for (int i = 0; i < N; i++) {
        final_sum += array[i];
    }
    
    checksum += final_sum;
    
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
