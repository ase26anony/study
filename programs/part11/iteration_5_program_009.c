/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define N 1000
#define M 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_array(double *arr, int n, double *results) {
    double sum = 0.0;
    double max_val = -__DBL_MAX__;
    
    /* Combined parallel and for clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic) reduction(+:sum) if(n > 100)
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i * 1.5;
        sum += arr[i];
        /* Use runtime API inside clause region */
        if (omp_get_thread_num() == 0 && i == 0) {
            /* Force diagnostic with clause name in message */
            #pragma omp message("Processing for clause with dynamic scheduling")
        }
    }
    results[0] = sum;
    
    /* Combined parallel and sections clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(sum, max_val) num_threads(4)
    {
        #pragma omp section
        {
            sum = 0.0;
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            results[1] = sum;
            /* Nested diagnostic with sections clause reference */
            #pragma omp error message("Inside sections clause region")
        }
        
        #pragma omp section
        {
            max_val = arr[0];
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
            results[2] = max_val;
        }
    }
}

/* Complex directive with for clause and arguments */
void __attribute__((optimize("O0"))) distributed_computation(double *arr, int n, int m, double *result) {
    /* Multi-clause directive with explicit for clause representation */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom:arr[0:n*m]) if(n*m > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            arr[idx] = arr[idx] * 2.0 + (double)(i + j);
        }
    }
    
    /* Calculate result with taskgroup clause */
    double total = 0.0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Taskgroup clause with task_reduction argument */
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (int i = 0; i < n; i++) {
                    #pragma omp task in_reduction(+:total) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        for (int j = 0; j < m; j++) {
                            local_sum += arr[i * m + j];
                        }
                        #pragma omp atomic
                        total += local_sum;
                        
                        /* Force pretty-printing of taskgroup clause */
                        if (i == 0) {
                            /* Use _Pragma to create complex pattern */
                            #define TASKGROUP_WARNING _Pragma("omp error message(\"taskgroup clause active\")")
                            TASKGROUP_WARNING;
                        }
                    }
                }
            }
        }
    }
    *result = total;
}

/* Function with mixed OpenMP and complex C control flow */
int __attribute__((optimize("O0"))) nested_control_flow(double *arr, int size) {
    int special_cases = 0;
    
    /* OpenMP inside switch statement */
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0:
                /* Directive with for clause inside switch */
                #pragma omp parallel for ordered schedule(guided)
                for (int i = 0; i < size; i += 2) {
                    #pragma omp ordered
                    {
                        arr[i] += 1.0;
                    }
                    /* Trigger diagnostic during compilation */
                    if (i == 0 && omp_get_thread_num() == 0) {
                        #pragma omp error severity(warning) message("for clause with ordered region")
                    }
                }
                break;
                
            case 1:
                /* Sections clause in nested context */
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        /* Macro expansion with _Pragma */
                        #define PARALLEL_SECTIONS _Pragma("omp parallel sections")
                        PARALLEL_SECTIONS
                        {
                            #pragma omp section
                            { special_cases++; }
                            #pragma omp section  
                            { special_cases++; }
                        }
                    }
                    #pragma omp section
                    {
                        for (int i = 1; i < size; i += 2) {
                            arr[i] -= 0.5;
                        }
                    }
                }
                break;
                
            case 2:
                /* Taskgroup in complex nesting */
                {
                    double task_sum = 0.0;
                    #pragma omp parallel
                    #pragma omp single
                    #pragma omp taskgroup task_reduction(+:task_sum)
                    {
                        #pragma omp task in_reduction(+:task_sum)
                        { task_sum += arr[0]; }
                        #pragma omp task in_reduction(+:task_sum)
                        { task_sum += arr[size-1]; }
                    }
                    special_cases += (int)task_sum;
                }
                break;
        }
    }
    
    return special_cases;
}

int main() {
    double *array = (double*)malloc(N * M * sizeof(double));
    double results[3];
    double distributed_result;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    omp_set_dynamic(0);
    
    /* Process 1: Combined parallel for and sections */
    process_array(array, N, results);
    
    /* Process 2: Distributed computation with taskgroup */
    distributed_computation(array, N, M, &distributed_result);
    
    /* Process 3: Nested control flow with various clauses */
    int special_count = nested_control_flow(array, N);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) schedule(static)
    for (int i = 0; i < N * M; i++) {
        checksum += array[i];
    }
    
    checksum += results[0] + results[1] + results[2] + distributed_result + special_count;
    
    printf("Final checksum: %f\n", checksum);
    printf("Special cases count: %d\n", special_count);
    
    /* Additional diagnostic triggers */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Final error directive with all clause names */
            #pragma omp error message("Clauses tested: for, parallel, sections, taskgroup")
        }
    }
    
    free(array);
    return 0;
}
