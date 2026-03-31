/* test_openmp_clauses.c - Targeting uncovered pretty-print lines for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define CHUNK_SIZE 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"), noinline)) 
process_with_for_clause(double *data, int n) {
    int i, j;
    
    /* TARGET: OMP_CLAUSE_FOR - Use in combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(i, j) shared(data) if(n > 1000)
    for (i = 0; i < n; i += CHUNK_SIZE) {
        for (j = 0; j < CHUNK_SIZE && (i + j) < n; j++) {
            data[i + j] = sin((double)(i + j)) * cos((double)(i + j));
        }
    }
    
    /* Nested directive to ensure clause appears in tree */
    #pragma omp parallel
    {
        #pragma omp for nowait schedule(dynamic, CHUNK_SIZE)
        for (i = 0; i < n; i++) {
            data[i] = sqrt(fabs(data[i]));
        }
    }
}

/* Complex control flow with sections clause */
double __attribute__((optimize("O0")))
compute_with_sections(double *data, int n) {
    double sum = 0.0, max_val = -INFINITY;
    
    /* TARGET: OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS combined */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        num_threads(4) if(n > 500)
    {
        /* TARGET: Multiple section blocks for sections clause */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += data[i];
            }
            /* Runtime call within structured block */
            int tid = omp_get_thread_num();
            printf("Section 1: thread %d processed %d elements\n", tid, n/2);
        }
        
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (data[i] > max_val) {
                    max_val = data[i];
                }
            }
            int tid = omp_get_thread_num();
            printf("Section 2: thread %d processed %d elements\n", tid, n - n/2);
        }
        
        /* Additional section to create more complex tree */
        #pragma omp section
        {
            /* Empty but forces clause representation */
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
void __attribute__((optimize("O0"), noinline))
process_with_taskgroup(double *data, int n, double *result) {
    double local_sum = 0.0;
    
    /* TARGET: OMP_CLAUSE_TASKGROUP with task_reduction argument */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:local_sum)
            {
                for (int i = 0; i < n; i += CHUNK_SIZE) {
                    #pragma omp task in_reduction(+:local_sum) \
                        firstprivate(i) shared(data)
                    {
                        double chunk_sum = 0.0;
                        int end = (i + CHUNK_SIZE < n) ? i + CHUNK_SIZE : n;
                        for (int j = i; j < end; j++) {
                            chunk_sum += data[j] * data[j];
                        }
                        local_sum += chunk_sum;
                        
                        /* Nested task with error directive */
                        #pragma omp task
                        {
                            /* Force diagnostic with clause name in message */
                            #pragma omp error message("Task completed for chunk") severity(warning)
                            
                            /* Macro expansion to create complex pattern */
                            #define EMIT_CLAUES _Pragma("omp error message(\"for clause\")")
                            EMIT_CLAUES;
                        }
                    }
                }
            }
        }
    }
    
    *result = local_sum;
}

/* Function with switch statement embedding OpenMP */
void __attribute__((optimize("O0")))
conditional_omp_processing(int mode, double *data, int n, double *output) {
    switch (mode) {
        case 1:
            /* TARGET: OMP_CLAUSE_FOR in nested context */
            #pragma omp parallel
            {
                #pragma omp for ordered
                for (int i = 0; i < n; i++) {
                    #pragma omp ordered
                    {
                        data[i] = data[i] * 2.0;
                    }
                }
            }
            break;
            
        case 2:
            /* TARGET: OMP_CLAUSE_SECTIONS in switch case */
            #pragma omp parallel sections private(output)
            {
                #pragma omp section
                {
                    *output = 0.0;
                }
                #pragma omp section
                {
                    for (int i = 0; i < n; i++) {
                        data[i] = 1.0 / (1.0 + data[i]);
                    }
                }
            }
            break;
            
        default:
            /* Mixed directives in default case */
            #pragma omp parallel
            {
                #pragma omp master
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            /* Error directive with clause reference */
                            #pragma omp error message("parallel clause used in default") severity(message)
                        }
                    }
                }
            }
            break;
    }
}

int main() {
    double *array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double checksum = 0.0;
    
    /* Phase 1: Initialize with for clause */
    printf("Phase 1: Initializing array with for clause...\n");
    process_with_for_clause(array, ARRAY_SIZE);
    
    /* Phase 2: Process with sections clause */
    printf("Phase 2: Processing with sections clause...\n");
    checksum += compute_with_sections(array, ARRAY_SIZE);
    
    /* Phase 3: Task-based computation with taskgroup clause */
    printf("Phase 3: Task-based computation with taskgroup clause...\n");
    double task_result;
    process_with_taskgroup(array, ARRAY_SIZE, &task_result);
    checksum += task_result;
    
    /* Phase 4: Conditional processing with mixed clauses */
    printf("Phase 4: Conditional processing...\n");
    double output;
    for (int mode = 0; mode < 3; mode++) {
        conditional_omp_processing(mode, array, ARRAY_SIZE, &output);
        checksum += output;
    }
    
    /* Additional complex pattern using _Pragma */
    {
        #pragma omp parallel
        {
            #define APPLY_FOR _Pragma("omp for schedule(guided)")
            APPLY_FOR
            for (int i = 0; i < ARRAY_SIZE; i++) {
                array[i] = fmod(array[i], 1.0);
            }
        }
        
        /* Combined directive triggering multiple clauses */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: array[0:ARRAY_SIZE]) if(ARRAY_SIZE > 1000)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = array[i] * array[i];
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) schedule(static)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    checksum += final_sum;
    
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
