/* test_omp_clauses.c - Targeting uncovered pretty-print lines for OMP_CLAUSE_FOR, 
   OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_with_for_clause(double *data) {
    /* Combined parallel and for clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic) num_threads(4)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i * 3.14159;
    }
    
    /* Complex for clause with multiple arguments */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) if(omp_get_num_threads() > 1)
    for (int i = 0; i < ARRAY_SIZE/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < ARRAY_SIZE) {
                data[idx] += (double)(i + j) * 0.01;
            }
        }
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) process_with_sections_clause(double *data) {
    double sum = 0.0;
    double max_val = data[0];
    
    /* Combined parallel and sections clause */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(data) shared(sum, max_val)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < ARRAY_SIZE/2; i++) {
                sum += data[i];
            }
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                if (data[i] > max_val) {
                    max_val = data[i];
                }
            }
        }
        
        /* Third section with nested for */
        #pragma omp section
        {
            #pragma omp parallel for
            for (int i = 0; i < ARRAY_SIZE; i += 10) {
                data[i] *= 1.01;
            }
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
double __attribute__((optimize("O0"))) process_with_taskgroup_clause(double *data) {
    double total = 0.0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:total) firstprivate(i)
                    {
                        double partial = 0.0;
                        int start = i * (ARRAY_SIZE/10);
                        int end = (i + 1) * (ARRAY_SIZE/10);
                        
                        for (int j = start; j < end && j < ARRAY_SIZE; j++) {
                            partial += data[j];
                        }
                        
                        total += partial;
                        
                        /* Force diagnostic with clause name in message */
                        if (i == 5) {
                            /* This should trigger pretty-printing of clauses */
                            #pragma omp error severity(warning) message("Task with for clause simulation")
                            
                            /* Using _Pragma to create complex pattern */
                            #define EMIT_FOR_CLAUSE _Pragma("omp error message(\"for clause\")")
                            EMIT_FOR_CLAUSE;
                        }
                    }
                }
            }
        }
    }
    
    return total;
}

/* Complex control flow with mixed OpenMP directives */
void __attribute__((optimize("O0"))) nested_omp_constructs(double *data) {
    int switch_var = omp_get_thread_num() % 3;
    
    switch (switch_var) {
        case 0: {
            /* Directives inside switch case */
            #pragma omp parallel for ordered
            for (int i = 0; i < ARRAY_SIZE; i++) {
                data[i] = data[i] * 2.0;
                #pragma omp ordered
                {
                    /* Empty ordered region */
                }
            }
            break;
        }
        case 1: {
            /* Nested parallel regions */
            #pragma omp parallel
            {
                #pragma omp for nowait
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    data[i] += (double)i;
                }
            }
            break;
        }
        case 2: {
            /* Sections inside parallel */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp simd
                    for (int i = 0; i < ARRAY_SIZE; i++) {
                        data[i] -= 1.0;
                    }
                }
                #pragma omp section
                {
                    /* Another for clause */
                    #pragma omp for simd
                    for (int i = 0; i < ARRAY_SIZE; i++) {
                        data[i] /= 2.0;
                    }
                }
            }
            break;
        }
    }
}

/* Main function with execution flow as specified */
int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double checksum = 0.0;
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    omp_set_dynamic(0);
    
    /* Phase 1: Parallel for with schedule(dynamic) */
    printf("Phase 1: Initializing array with parallel for\n");
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)(i + 1);
    }
    
    /* Phase 2: Process with for clause function */
    printf("Phase 2: Processing with for clause constructs\n");
    process_with_for_clause(data);
    
    /* Phase 3: Parallel sections with reductions */
    printf("Phase 3: Processing with sections clause\n");
    checksum += process_with_sections_clause(data);
    
    /* Phase 4: Taskgroup with task_reduction */
    printf("Phase 4: Processing with taskgroup clause\n");
    checksum += process_with_taskgroup_clause(data);
    
    /* Phase 5: Nested constructs */
    printf("Phase 5: Nested OpenMP constructs\n");
    nested_omp_constructs(data);
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i];
    }
    
    /* Additional diagnostic triggers */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Force error with clause names */
            #pragma omp error severity(message) \
                message("Testing clause pretty-printing: for parallel sections taskgroup")
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(data);
    return 0;
}
