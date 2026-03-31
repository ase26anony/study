/* test_openmp_clauses.c - Targeting uncovered pretty-print lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n) {
    double sum = 0.0;
    double max_val = -INFINITY;
    int i, j;
    
    /* TARGET: OMP_CLAUSE_FOR - Use in combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(j) shared(data) reduction(+:sum) if(n > 1000)
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            data[i] = sin(i * 0.01) * cos(j * 0.01);
            sum += data[i];
        }
    }
    
    /* Force diagnostic with clause name in message */
    #ifdef GENERATE_DIAGNOSTIC
    #pragma omp error severity(warning) message("Processing clause: for")
    #endif
    
    /* TARGET: OMP_CLAUSE_PARALLEL - Use in combined directive */
    #pragma omp parallel default(none) shared(data, n, max_val) num_threads(4)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        
        /* Nested directive with 'for' clause */
        #pragma omp for schedule(dynamic, 8) nowait
        for (i = 0; i < n; i++) {
            if (data[i] > max_val) {
                #pragma omp critical
                {
                    if (data[i] > max_val) max_val = data[i];
                }
            }
        }
        
        /* Use _Pragma for complex pattern */
        _Pragma("omp barrier")
        
        /* TARGET: OMP_CLAUSE_SECTIONS - Inside parallel region */
        #pragma omp sections private(i) firstprivate(tid)
        {
            #pragma omp section
            {
                double local_sum = 0.0;
                for (i = tid * (n/nthreads); i < (tid+1) * (n/nthreads); i++) {
                    local_sum += data[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp section
            {
                /* Another sections directive nested inside */
                #pragma omp sections
                {
                    #pragma omp section
                    { /* Empty section but creates the structure */ }
                    #pragma omp section
                    { /* Second empty section */ }
                }
            }
        }
    }
    
    /* TARGET: OMP_CLAUSE_TASKGROUP - With task_reduction argument */
    double task_sum = 0.0;
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task in_reduction(+:task_sum) firstprivate(i) shared(data, n)
                {
                    double partial = 0.0;
                    int start = i * (n/10);
                    int end = (i+1) * (n/10);
                    for (int k = start; k < end && k < n; k++) {
                        partial += sqrt(fabs(data[k]));
                    }
                    task_sum += partial;
                    
                    /* Nested task with error directive containing clause name */
                    #pragma omp task
                    {
                        #pragma omp error severity(message) \
                            message("Task completed for iteration with clause: taskgroup")
                    }
                }
            }
        }
    }
    
    /* Complex control flow with OpenMP directives */
    switch ((int)sum % 3) {
        case 0:
            /* Another parallel for directive in switch case */
            #pragma omp parallel for simd schedule(guided)
            for (i = 0; i < n/2; i++) {
                data[i] *= 1.1;
            }
            break;
        case 1:
            /* Parallel sections in another case */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp parallel for
                    for (i = 0; i < n; i += 2) {
                        data[i] = pow(data[i], 1.5);
                    }
                }
                #pragma omp section
                {
                    for (i = 1; i < n; i += 2) {
                        data[i] = log1p(fabs(data[i]));
                    }
                }
            }
            break;
        default:
            /* Taskgroup in default case */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Force pretty-printing during error */
                    #ifdef GENERATE_ERROR
                    #pragma omp error severity(fatal) \
                        message("Unhandled case with clauses: parallel sections for taskgroup")
                    #endif
                }
            }
    }
    
    printf("Processed: sum=%.2f, max=%.2f, task_sum=%.2f\n", sum, max_val, task_sum);
}

/* Another function with different clause combinations */
void __attribute__((optimize("O0"))) nested_function_openmp(int *result) {
    int matrix[100][100];
    int i, j;
    
    /* Combined parallel for with multiple clauses */
    #pragma omp parallel for private(j) collapse(2) \
        schedule(nonmonotonic:dynamic) ordered(2)
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix[i][j] = i * j + omp_get_thread_num();
            #pragma omp ordered depend(sink: i-1, j) depend(sink: i, j-1)
            /* Computation */
            #pragma omp ordered depend(source)
        }
    }
    
    /* Macro expansion with _Pragma for complex pretty-printing */
    #define APPLY_PARALLEL_SECTIONS(expr) \
        _Pragma("omp parallel sections") \
        { \
            _Pragma("omp section") expr \
            _Pragma("omp section") expr \
        }
    
    APPLY_PARALLEL_SECTIONS({ *result += 1; })
    
    /* Taskgroup with reduction in loop */
    int task_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_result)
            {
                for (i = 0; i < 50; i++) {
                    #pragma omp task in_reduction(+:task_result)
                    {
                        task_result += i;
                    }
                }
            }
        }
    }
    
    *result += task_result;
}

int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) return 1;
    
    int checksum = 0;
    
    /* Multiple iterations to ensure coverage */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        /* Initialize with parallel for */
        #pragma omp parallel for schedule(dynamic) if(ARRAY_SIZE > 1000)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = (i + iter) * 0.001;
        }
        
        /* Process with various OpenMP constructs */
        process_with_openmp(data, ARRAY_SIZE);
        
        /* Call function with different clause combinations */
        nested_function_openmp(&checksum);
        
        /* Final parallel sections with reduction */
        double final_sum = 0.0;
        #pragma omp parallel sections reduction(+:final_sum)
        {
            #pragma omp section
            {
                #pragma omp parallel for reduction(+:final_sum)
                for (int i = 0; i < ARRAY_SIZE/2; i++) {
                    final_sum += data[i];
                }
            }
            #pragma omp section
            {
                #pragma omp parallel for reduction(+:final_sum)
                for (int i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                    final_sum += data[i];
                }
            }
        }
        
        checksum += (int)final_sum;
        
        /* Taskgroup at top level */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                /* Diagnostic with clause names */
                #pragma omp error severity(warning) \
                    message("Iteration complete with clauses: for parallel sections taskgroup")
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup with OpenMP (unusual but valid) */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                free(data);
            }
        }
    }
    
    return 0;
}
