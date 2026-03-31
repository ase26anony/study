/* test_omp_clauses.c - Targeting uncovered pretty-print cases for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function attribute to prevent optimization from removing OpenMP constructs */
__attribute__((optimize("O0"))) 
void process_with_for_clause(double *data, int n) {
    int i, j;
    
    /* Using 'for' clause in a combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(i, j) shared(data, n)
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            data[i] += (i * j) * 0.001;
        }
    }
    
    /* Nested directive that will invoke pretty-printing of 'for' clause */
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic) nowait
        for (i = 0; i < n; i++) {
            data[i] *= 1.01;
        }
    }
}

/* Function using sections clause */
__attribute__((optimize("O0")))
double process_with_sections_clause(double *data, int n) {
    double sum = 0.0, max_val = data[0];
    
    /* Combined parallel sections directive */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        num_threads(4) if(n > 1000)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += data[i];
            }
            /* Diagnostic that forces pretty-printing of clause names */
            #pragma omp error severity(warning) message("Processing in sections clause region")
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (data[i] > max_val) {
                    max_val = data[i];
                }
            }
        }
        
        /* Third section using macro with _Pragma */
        #pragma omp section
        {
            #define OMP_WARNING(msg) _Pragma("omp error severity(warning) message(msg)")
            OMP_WARNING("Inside sections clause")
            #undef OMP_WARNING
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
__attribute__((optimize("O0")))
double process_with_taskgroup_clause(double *data, int n) {
    double total = 0.0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (int i = 0; i < n; i++) {
                    #pragma omp task in_reduction(+:total) firstprivate(i) \
                        if(i % 100 == 0)
                    {
                        double val = data[i] * 0.5;
                        total += val;
                        
                        /* Nested task with error directive mentioning 'for' clause */
                        if (i == 0) {
                            #pragma omp task
                            {
                                #pragma omp error severity(message) \
                                    message("Taskgroup clause with nested task referencing for clause")
                            }
                        }
                    }
                }
            }
        }
    }
    
    return total;
}

/* Complex control flow with mixed OpenMP directives */
__attribute__((optimize("O0")))
void complex_control_flow(double *data, int n) {
    int mode = omp_get_thread_num() % 3;
    
    switch (mode) {
        case 0: {
            /* Switch case with parallel for */
            #pragma omp parallel for ordered
            for (int i = 0; i < n; i++) {
                #pragma omp ordered
                {
                    data[i] += i * 0.001;
                }
            }
            break;
        }
        case 1: {
            /* Nested parallel regions */
            #pragma omp parallel
            {
                #pragma omp master
                {
                    #pragma omp taskloop grainsize(64)
                    for (int i = 0; i < n; i++) {
                        data[i] = data[i] * data[i];
                    }
                }
            }
            break;
        }
        case 2: {
            /* Combined parallel sections with for loop inside */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp for simd
                    for (int i = 0; i < n; i++) {
                        data[i] += 1.0;
                    }
                }
                #pragma omp section
                {
                    for (int i = 0; i < n; i++) {
                        data[i] *= 0.99;
                    }
                }
            }
            break;
        }
    }
}

/* Main function with execution flow as specified */
int main() {
    double *array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double checksum = 0.0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array in parallel */
    #pragma omp parallel for schedule(dynamic) if(ARRAY_SIZE > 1000)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (double)i / ARRAY_SIZE;
    }
    
    /* Process with for clause (triggers OMP_CLAUSE_FOR) */
    process_with_for_clause(array, ARRAY_SIZE);
    
    /* Process with sections clause (triggers OMP_CLAUSE_SECTIONS) */
    checksum += process_with_sections_clause(array, ARRAY_SIZE);
    
    /* Process with taskgroup clause (triggers OMP_CLAUSE_TASKGROUP) */
    checksum += process_with_taskgroup_clause(array, ARRAY_SIZE);
    
    /* Complex control flow mixing clauses */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        complex_control_flow(array, ARRAY_SIZE);
        
        /* Additional parallel region to ensure parallel clause is processed */
        #pragma omp parallel if(iter % 10 == 0)
        {
            #pragma omp for nowait
            for (int i = 0; i < ARRAY_SIZE; i++) {
                array[i] = array[i] * 0.999 + 0.001;
            }
            
            /* Force diagnostic with clause name in message */
            if (omp_get_thread_num() == 0 && iter == NUM_ITERATIONS/2) {
                #pragma omp error severity(warning) \
                    message("Parallel clause execution at iteration midpoint")
            }
        }
    }
    
    /* Final reduction */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        schedule(guided) num_threads(omp_get_max_threads())
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    checksum += final_sum;
    
    /* Print checksum to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
