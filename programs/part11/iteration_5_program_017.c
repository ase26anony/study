/* test_omp_clauses.c - Targeting uncovered pretty-print lines for OMP_CLAUSE_FOR, 
   OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP structure */
void __attribute__((optimize("O0"))) process_with_for_clause(double *data, int n) {
    int i, j;
    
    /* Target: OMP_CLAUSE_FOR - with explicit arguments for non-trivial representation */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(i, j) shared(data) if(n > 1000)
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            data[i] += sin(i * 0.01) * cos(j * 0.01);
        }
    }
    
    /* Force diagnostic with clause name in message */
    #pragma omp error severity(warning) message("for clause processed")
}

/* Function using parallel sections */
double __attribute__((optimize("O0"))) compute_with_sections(double *data, int n) {
    double sum = 0.0, max_val = -INFINITY;
    
    /* Target: OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS combined */
    #pragma omp parallel sections private(sum) reduction(max:max_val) \
        num_threads(4) if(n > 500)
    {
        /* First section with its own reduction */
        #pragma omp section
        {
            sum = 0.0;
            #pragma omp parallel for reduction(+:sum) schedule(guided)
            for (int i = 0; i < n/2; i++) {
                sum += data[i];
            }
            #pragma omp critical
            printf("Section 1 sum: %f (threads: %d)\n", sum, omp_get_num_threads());
        }
        
        /* Second section */
        #pragma omp section
        {
            max_val = data[0];
            #pragma omp parallel for reduction(max:max_val) schedule(dynamic, 8)
            for (int i = n/2; i < n; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
        }
    }
    
    /* Diagnostic for sections clause */
    #pragma omp error severity(message) message("sections clause completed")
    
    return sum + max_val;
}

/* Complex nested function with taskgroup */
void __attribute__((optimize("O0"))) nested_task_processing(double *data, int n, double *result) {
    double task_sum = 0.0;
    int i;
    
    /* Switch statement embedding OpenMP for complex control flow */
    switch (n % 3) {
        case 0:
            /* Target: OMP_CLAUSE_TASKGROUP with task_reduction argument */
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (i = 0; i < n; i += n/4) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i) \
                        if(i < n/2) final(i > n/4)
                    {
                        double local_sum = 0.0;
                        int end = (i + n/4 < n) ? i + n/4 : n;
                        for (int j = i; j < end; j++) {
                            local_sum += data[j] * data[j];
                        }
                        task_sum += sqrt(local_sum);
                        
                        /* Nested task with error directive */
                        #pragma omp task
                        {
                            #pragma omp error severity(warning) \
                                message("Nested task in taskgroup with for clause reference")
                        }
                    }
                }
            }
            *result = task_sum;
            break;
            
        case 1:
            /* Alternative path with parallel for */
            #pragma omp parallel for reduction(+:task_sum) schedule(runtime)
            for (i = 0; i < n; i++) {
                task_sum += data[i];
            }
            *result = task_sum;
            break;
            
        default:
            *result = 0.0;
    }
    
    /* Macro expansion with _Pragma for complex pretty-printing */
    #define EMIT_CLause_DIAG(clause) _Pragma("omp error severity(message) message(\"" #clause " clause printed\")")
    EMIT_CLause_DIAG(taskgroup);
}

/* Main function with mixed OpenMP constructs */
int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double checksum = 0.0;
    int i;
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Phase 1: Initialize array with parallel for (targets OMP_CLAUSE_FOR) */
    printf("Initializing array...\n");
    #pragma omp parallel for schedule(dynamic) default(none) shared(data) \
        if(ARRAY_SIZE > 100) ordered
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i % 100) * 0.01;
        #pragma omp ordered
        if (i % 1000 == 0) {
            /* Runtime call within clause context */
            int tid = omp_get_thread_num();
            data[i] += tid * 0.001;
        }
    }
    
    /* Phase 2: Process with for clause in complex directive */
    process_with_for_clause(data, ARRAY_SIZE);
    
    /* Phase 3: Compute with parallel sections (targets OMP_CLAUSE_SECTIONS) */
    printf("Computing with sections...\n");
    checksum += compute_with_sections(data, ARRAY_SIZE);
    
    /* Phase 4: Taskgroup processing (targets OMP_CLAUSE_TASKGROUP) */
    printf("Processing with taskgroup...\n");
    double task_result;
    nested_task_processing(data, ARRAY_SIZE, &task_result);
    checksum += task_result;
    
    /* Phase 5: Additional combined directive for coverage */
    printf("Final combined processing...\n");
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < ARRAY_SIZE/10; i++) {
            data[i] *= 1.1;
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Force diagnostic for parallel clause */
                    #pragma omp error severity(warning) message("parallel clause in final section")
                }
            }
        }
    }
    
    /* Final computation to prevent dead code elimination */
    double final_sum = 0.0;
    #pragma omp parallel for simd reduction(+:final_sum) aligned(data:64) \
        safelen(16) linear(i:1)
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_sum += data[i];
    }
    checksum += final_sum;
    
    printf("Final checksum: %f\n", checksum);
    
    free(data);
    return 0;
}
