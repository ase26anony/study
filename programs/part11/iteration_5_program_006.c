/* test_omp_clauses.c - Target coverage for tree-pretty-print.cc lines 1434-1445 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define CHUNK_SIZE 64

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_for_clause(double *arr, int n) {
    int i, j;
    double local_sum = 0.0;
    
    /* Combined directive with 'for' clause - will trigger pretty-printing */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(i, j) reduction(+:local_sum) if(n > 100)
    for (i = 0; i < n; i += CHUNK_SIZE) {
        for (j = 0; j < CHUNK_SIZE && (i + j) < n; j++) {
            arr[i + j] = sin((double)(i + j)) * cos((double)(i + j));
            local_sum += arr[i + j];
        }
    }
    
    /* Nested directive with 'for' clause */
    #pragma omp parallel
    {
        #pragma omp for nowait schedule(dynamic)
        for (int k = 0; k < omp_get_num_threads(); k++) {
            /* Force diagnostic with clause name in message */
            #pragma omp error severity(warning) message("for clause encountered in thread")
        }
    }
    
    printf("For clause processing complete, sum = %f\n", local_sum);
}

/* Function using 'parallel' and 'sections' clauses */
void __attribute__((optimize("O0"))) process_with_sections(double *arr, int n, 
                                                          double *sum_result, 
                                                          double *max_result) {
    *sum_result = 0.0;
    *max_result = -INFINITY;
    
    /* Combined directive with 'parallel' and 'sections' clauses */
    #pragma omp parallel sections reduction(+:*sum_result) reduction(max:*max_result) \
        num_threads(4) if(n > 500)
    {
        /* First section - sum calculation */
        #pragma omp section
        {
            double local_sum = 0.0;
            for (int i = 0; i < n/2; i++) {
                local_sum += arr[i];
            }
            *sum_result += local_sum;
            
            /* Nested diagnostic with clause reflection */
            #pragma omp error severity(message) message("sections clause in section 1")
        }
        
        /* Second section - max calculation */
        #pragma omp section
        {
            double local_max = -INFINITY;
            for (int i = n/2; i < n; i++) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            if (local_max > *max_result) *max_result = local_max;
            
            /* Complex macro expansion with _Pragma */
            #define SECTION_MESSAGE(msg) _Pragma("omp error severity(warning) message(msg)")
            SECTION_MESSAGE("parallel sections clause processed");
        }
        
        /* Third section - additional processing */
        #pragma omp section
        {
            /* Empty section but creates the clause structure */
            #pragma omp atomic
            (*sum_result) += 0.0;
        }
        
        /* Fourth section - runtime API with clause context */
        #pragma omp section
        {
            int num_threads = omp_get_num_threads();
            #pragma omp critical
            printf("Parallel sections executing with %d threads\n", num_threads);
        }
    }
}

/* Function using 'taskgroup' clause */
void __attribute__((optimize("O0"))) process_with_taskgroup(double *arr, int n, 
                                                           double *task_sum) {
    *task_sum = 0.0;
    
    /* Taskgroup with explicit task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:*task_sum)
        {
            /* Spawn multiple tasks */
            for (int i = 0; i < n; i += n/10) {
                #pragma omp task in_reduction(+:*task_sum) firstprivate(i) \
                    if(i < n/2) final(i >= n/2)
                {
                    double partial_sum = 0.0;
                    int end = (i + n/10) < n ? (i + n/10) : n;
                    for (int j = i; j < end; j++) {
                        partial_sum += arr[j] * arr[j];
                    }
                    *task_sum += partial_sum;
                    
                    /* Nested task with error directive */
                    if (i == 0) {
                        #pragma omp task
                        {
                            #pragma omp error severity(warning) \
                                message("taskgroup clause with nested task")
                        }
                    }
                }
            }
            
            /* Additional task with complex clause combination */
            #pragma omp task untied mergeable
            {
                /* Force pretty-printing of taskgroup clause */
                #pragma omp error severity(message) \
                    message("End of taskgroup region")
            }
        }
    }
}

/* Main function with mixed OpenMP and C constructs */
int main(void) {
    double *array = (double*)malloc(N * sizeof(double));
    double sum_result, max_result, task_sum;
    double checksum = 0.0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array using parallel for with schedule clause */
    #pragma omp parallel for schedule(dynamic) shared(array) \
        if(N > 100) num_threads(omp_get_max_threads())
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
    }
    
    /* Switch statement with OpenMP directives inside */
    int mode = 2;  /* Can be modified */
    switch (mode) {
        case 1:
            /* Process with for clause */
            process_with_for_clause(array, N);
            break;
        case 2:
            /* Process with sections */
            process_with_sections(array, N, &sum_result, &max_result);
            checksum += sum_result + max_result;
            printf("Sections result: sum=%f, max=%f\n", sum_result, max_result);
            
            /* Process with taskgroup */
            process_with_taskgroup(array, N, &task_sum);
            checksum += task_sum;
            printf("Taskgroup result: sum=%f\n", task_sum);
            break;
        default:
            /* Combined directive in default case */
            #pragma omp parallel for simd schedule(guided)
            for (int i = 0; i < N; i++) {
                array[i] = sqrt(array[i]);
            }
            break;
    }
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum) \
        if(checksum > 0.0) proc_bind(close)
    for (int i = 0; i < N; i++) {
        checksum += array[i] * 0.001;
    }
    
    /* Additional complex directive nesting */
    {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom:array[0:N]) if(target:omp_get_num_devices() > 0)
        for (int i = 0; i < N; i++) {
            array[i] = exp(-array[i]);
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    
    free(array);
    return 0;
}
