/* test_openmp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_for_clause(double *data, int n) {
    double local_sum = 0.0;
    
    /* Use 'for' clause with explicit arguments - targets OMP_CLAUSE_FOR */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        reduction(+:local_sum) private(n) if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            data[i] = sin(data[i]) * cos(data[i]);
            local_sum += data[i];
        }
    }
    
    /* Nested directive combining 'parallel' and 'for' clauses */
    #pragma omp parallel for ordered schedule(dynamic, 8) \
        num_threads(omp_get_max_threads()/2) \
        if(omp_in_parallel())
    for (int i = 0; i < n/2; i++) {
        #pragma omp ordered
        data[i] = sqrt(fabs(data[i])) + local_sum;
    }
    
    /* Force diagnostic with clause name in message */
    #pragma omp error severity(warning) message("Processing with 'for' clause completed")
}

/* Function using 'parallel' and 'sections' clauses */
void __attribute__((optimize("O0"))) process_with_sections(double *data, int n, 
                                                          double *sum_result, 
                                                          double *max_result) {
    double sum = 0.0;
    double max_val = -INFINITY;
    
    /* Combined 'parallel' and 'sections' clauses */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(n) copyin(data[0:n]) proc_bind(close)
    {
        /* First section - OMP_CLAUSE_SECTIONS */
        #pragma omp section
        {
            for (int i = 0; i < n; i += 2) {
                sum += data[i] * data[i];
            }
            /* Nested parallel region inside section */
            #pragma omp parallel for simd reduction(+:sum) if(n > 500)
            for (int i = 0; i < n/4; i++) {
                sum += data[i] * 0.5;
            }
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = 1; i < n; i += 2) {
                if (data[i] > max_val) {
                    max_val = data[i];
                }
            }
            /* Another directive with 'for' clause */
            #pragma omp for simd reduction(max:max_val) schedule(guided)
            for (int i = 0; i < n/4; i++) {
                max_val = fmax(max_val, data[n - i - 1]);
            }
        }
        
        /* Third section with error directive */
        #pragma omp section
        {
            #pragma omp error severity(message) \
                message("In sections clause with parallel execution")
        }
    }
    
    *sum_result = sum;
    *max_result = max_val;
}

/* Complex macro expansion using _Pragma for 'taskgroup' clause */
#define CREATE_TASKGROUP(reducer, var, init) \
    _Pragma("omp taskgroup task_reduction(+:" #reducer ")") \
    { \
        reducer = init; \
        _Pragma("omp task in_reduction(+:" #reducer ")") \
        { \
            reducer += var; \
        } \
        _Pragma("omp task in_reduction(+:" #reducer ") if(0)") \
        { \
            reducer += var * 0.5; \
        } \
    }

/* Function using 'taskgroup' clause - targets OMP_CLAUSE_TASKGROUP */
void __attribute__((optimize("O0"))) process_with_taskgroup(double *data, int n, 
                                                           double *task_result) {
    double task_sum = 0.0;
    double chunk_sum = 0.0;
    
    /* Direct use of taskgroup clause with task_reduction */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:task_sum) \
            allocate(omp_default_mem_alloc: task_sum)
        {
            /* Spawn multiple tasks */
            for (int i = 0; i < n; i += 100) {
                #pragma omp task in_reduction(+:task_sum) \
                    firstprivate(i) if(i < n/2)
                {
                    double local = 0.0;
                    int end = (i + 100 < n) ? i + 100 : n;
                    for (int j = i; j < end; j++) {
                        local += data[j] * data[j];
                    }
                    task_sum += local;
                    
                    /* Nested task with error containing clause name */
                    #pragma omp task if(0)
                    {
                        #pragma omp error severity(warning) \
                            message("Inside task with taskgroup clause")
                    }
                }
            }
            
            /* Wait for all tasks in the taskgroup */
            #pragma omp taskwait
        }
        
        /* Use macro expansion for another taskgroup */
        CREATE_TASKGROUP(chunk_sum, data[n/2], 1.0);
        
        /* Combined directive with multiple clauses */
        #pragma omp parallel for reduction(+:chunk_sum) \
            if(task_sum > 0.0)
        for (int i = 0; i < n; i++) {
            chunk_sum += data[i] * 0.1;
        }
    }
    
    *task_result = task_sum + chunk_sum;
}

/* Main function with complex control flow */
int main(int argc, char *argv[]) {
    double *data = (double *)malloc(ARRAY_SIZE * sizeof(double));
    double final_sum = 0.0;
    double section_sum, section_max, task_result;
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data with random values */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)rand() / RAND_MAX;
    }
    
    /* Switch statement embedding OpenMP directives */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        switch (iter % 4) {
            case 0:
                /* Use 'for' clause in parallel directive */
                process_with_for_clause(data, ARRAY_SIZE);
                break;
            case 1:
                /* Use 'parallel' and 'sections' clauses */
                process_with_sections(data, ARRAY_SIZE, &section_sum, &section_max);
                final_sum += section_sum + section_max;
                break;
            case 2:
                /* Use 'taskgroup' clause */
                process_with_taskgroup(data, ARRAY_SIZE, &task_result);
                final_sum += task_result;
                break;
            case 3:
                /* Combined directive in main */
                #pragma omp parallel for reduction(+:final_sum) \
                    schedule(static) if(iter > 10)
                for (int i = 0; i < ARRAY_SIZE/10; i++) {
                    final_sum += data[i] * iter;
                }
                break;
        }
        
        /* Conditional compilation with _Pragma */
        if (iter % 10 == 0) {
            _Pragma("omp error severity(message) message(\"Iteration checkpoint with for/parallel/sections/taskgroup clauses\")")
        }
    }
    
    /* Final computation with nested directives */
    double checksum = 0.0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:checksum) nowait
        for (int i = 0; i < ARRAY_SIZE; i++) {
            checksum += data[i];
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:checksum)
            {
                #pragma omp task in_reduction(+:checksum)
                {
                    checksum += final_sum;
                }
            }
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    printf("Clause coverage test completed.\n");
    
    free(data);
    return 0;
}
