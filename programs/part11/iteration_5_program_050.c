/* test_omp_clauses.c - Targeting uncovered pretty-print cases for OMP_CLAUSE_FOR, 
   OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_with_for_clause(double *data, int n) {
    int i, j;
    
    /* Combined parallel for directive - triggers both PARALLEL and FOR clauses */
    #pragma omp parallel for schedule(dynamic) private(i) shared(data, n) \
        if(n > 1000) num_threads(omp_get_max_threads())
    for (i = 0; i < n; i++) {
        data[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Complex for clause with multiple arguments - ensures non-trivial representation */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom: data[0:n]) \
        num_teams(omp_get_num_teams()) thread_limit(64)
    for (i = 0; i < sqrt(n); i++) {
        for (j = 0; j < sqrt(n); j++) {
            int idx = i * (int)sqrt(n) + j;
            if (idx < n) {
                data[idx] = data[idx] * 1.1 + 0.5;
            }
        }
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) process_with_sections_clause(double *data, int n) {
    double sum = 0.0, max_val = -1e30;
    
    /* Combined parallel sections directive - triggers both PARALLEL and SECTIONS clauses */
    #pragma omp parallel sections private(sum, max_val) shared(data, n) \
        reduction(+:sum) reduction(max:max_val)
    {
        #pragma omp section
        {
            sum = 0.0;
            #pragma omp parallel for reduction(+:sum) schedule(guided)
            for (int i = 0; i < n/2; i++) {
                sum += data[i];
            }
            /* Nested directive inside section */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    data[0] = sum / (n/2);
                }
            }
        }
        
        #pragma omp section
        {
            max_val = data[0];
            #pragma omp parallel for reduction(max:max_val) schedule(runtime)
            for (int i = n/2; i < n; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
        }
        
        #pragma omp section
        {
            /* Empty section to ensure sections clause is fully represented */
            #pragma omp critical
            {
                printf("Processing middle section\n");
            }
        }
    }
    
    return sum + max_val;
}

/* Complex function with taskgroup clause */
void __attribute__((optimize("O0"))) process_with_taskgroup_clause(double *data, int n) {
    double task_sum = 0.0;
    
    /* Taskgroup with explicit task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task in_reduction(+:task_sum) firstprivate(i) \
                    if(i % 2 == 0) final(i > 5) mergeable
                {
                    double local_sum = 0.0;
                    int start = i * (n / 10);
                    int end = (i + 1) * (n / 10);
                    
                    for (int j = start; j < end && j < n; j++) {
                        local_sum += data[j] * data[j];
                    }
                    
                    task_sum += sqrt(local_sum);
                    
                    /* Force diagnostic with clause name in message */
                    if (i == 3) {
                        #pragma omp error severity(warning) \
                            message("Task iteration 3 completed with for clause optimization")
                    }
                }
            }
        }
        
        /* Nested taskgroup without reduction */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                /* Use _Pragma to create complex pattern */
                _Pragma("omp critical")
                {
                    printf("Inner task completed\n");
                }
            }
        }
    }
    
    /* Store result */
    if (n > 0) data[0] = task_sum;
}

/* Function that triggers pretty-printing through error directives */
void __attribute__((optimize("O0"))) trigger_clause_printing(void) {
    int i;
    
    /* Multiple error directives with different clause names */
    #pragma omp parallel for schedule(static)
    for (i = 0; i < 5; i++) {
        switch (i) {
            case 0:
                /* Error directive mentioning 'for' clause */
                #pragma omp error severity(message) \
                    message("Testing FOR clause pretty-printing")
                break;
            case 1:
                /* Error directive mentioning 'parallel' clause */
                #pragma omp error severity(warning) \
                    message("Testing PARALLEL clause representation")
                break;
            case 2:
                /* Error directive mentioning 'sections' clause */
                #pragma omp error \
                    message("Testing SECTIONS clause in pretty-printer")
                break;
            case 3:
                /* Error directive mentioning 'taskgroup' clause */
                #pragma omp error severity(message) \
                    message("Testing TASKGROUP clause output")
                break;
            default:
                /* Combined directive in error context */
                #pragma omp error \
                    message("Testing combined: parallel for sections taskgroup")
                break;
        }
    }
    
    /* Complex nested pragma using _Pragma */
    #define NESTED_PRAGMA(x) _Pragma("omp parallel sections") \
        { \
            _Pragma("omp section") \
            { \
                _Pragma("omp taskgroup task_reduction(+:x)") \
                { \
                    _Pragma("omp task in_reduction(+:x)") \
                    x += 1.0; \
                } \
            } \
        }
    
    double test_var = 0.0;
    NESTED_PRAGMA(test_var)
}

/* Main function with mixed OpenMP and C constructs */
int main(void) {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double checksum = 0.0;
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize OpenMP */
    omp_set_num_threads(omp_get_max_threads());
    
    /* Phase 1: Process with for clause */
    printf("Phase 1: Processing with FOR clause...\n");
    process_with_for_clause(data, ARRAY_SIZE);
    
    /* Phase 2: Process with sections clause */
    printf("Phase 2: Processing with SECTIONS clause...\n");
    checksum += process_with_sections_clause(data, ARRAY_SIZE);
    
    /* Phase 3: Process with taskgroup clause */
    printf("Phase 3: Processing with TASKGROUP clause...\n");
    process_with_taskgroup_clause(data, ARRAY_SIZE);
    checksum += data[0];
    
    /* Phase 4: Trigger clause printing through diagnostics */
    printf("Phase 4: Triggering clause pretty-printing...\n");
    trigger_clause_printing();
    
    /* Additional complex nesting to ensure coverage */
    {
        int i, j;
        #pragma omp parallel private(i, j) shared(data, checksum)
        {
            #pragma omp for collapse(2) schedule(static, 8) nowait
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    int idx = (i * 100 + j) % ARRAY_SIZE;
                    if (idx >= 0 && idx < ARRAY_SIZE) {
                        #pragma omp atomic
                        data[idx] += 0.001 * (i + j);
                    }
                }
            }
            
            #pragma omp single
            {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        /* Nested parallel region inside task */
                        #pragma omp parallel for simd simdlen(4) \
                            schedule(nonmonotonic:dynamic)
                        for (int k = 0; k < ARRAY_SIZE/10; k++) {
                            data[k] = fmod(data[k], 1.0);
                        }
                    }
                }
            }
            
            #pragma omp barrier
            
            #pragma omp for reduction(+:checksum) ordered
            for (i = 0; i < ARRAY_SIZE; i += ARRAY_SIZE/100) {
                checksum += data[i];
                #pragma omp ordered
                {
                    /* Empty ordered block to create ordered clause */
                }
            }
        }
    }
    
    /* Final computation to prevent dead code elimination */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
        if(ARRAY_SIZE > 5000) proc_bind(spread)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_result += data[i] * (i % 100);
    }
    
    checksum += final_result;
    
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(data);
    return 0;
}
