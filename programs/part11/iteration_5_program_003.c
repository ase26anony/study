/* test_openmp_clauses.c
 * Targets uncovered lines in tree-pretty-print.cc: OMP_CLAUSE_FOR, 
 * OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1000
#define NESTED_LOOP_SIZE 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(double* data, int size, double* results) {
    double sum = 0.0;
    double max_val = -1e308;
    
    /* 1. Combined parallel and for clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic) reduction(+:sum) if(size > 100)
    for (int i = 0; i < size; i++) {
        data[i] = (double)i * 1.5;
        sum += data[i];
        
        /* Use runtime API inside the clause region */
        if (omp_get_thread_num() == 0 && i == 0) {
            /* Force diagnostic with clause name in message */
            #pragma omp message("Processing with parallel for clause")
        }
    }
    results[0] = sum;
    
    /* 2. Complex for clause with multiple arguments - triggers OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for simd \
            schedule(static, 4) collapse(2) map(tofrom:data[0:size])
    for (int i = 0; i < NESTED_LOOP_SIZE; i++) {
        for (int j = 0; j < NESTED_LOOP_SIZE; j++) {
            int idx = (i * NESTED_LOOP_SIZE + j) % size;
            if (idx < size) {
                data[idx] += 0.01 * (i + j);
            }
        }
    }
    
    /* 3. Combined parallel and sections clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(sum) reduction(max:max_val)
    {
        #pragma omp section
        {
            sum = 0.0;
            #pragma omp parallel for reduction(+:sum) if(size > 500)
            for (int i = 0; i < size/2; i++) {
                sum += data[i];
            }
            results[1] = sum;
            
            /* Nested error directive with clause reference */
            #pragma omp error severity(warning) message("Inside sections clause, checking for clause")
        }
        
        #pragma omp section
        {
            max_val = data[0];
            #pragma omp parallel for reduction(max:max_val)
            for (int i = size/2; i < size; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
            results[2] = max_val;
        }
    }
    
    /* 4. Taskgroup clause with task_reduction - triggers OMP_CLAUSE_TASKGROUP */
    double task_sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        int chunk = size / 10;
                        int start = i * chunk;
                        int end = (i == 9) ? size : (i + 1) * chunk;
                        
                        for (int j = start; j < end; j++) {
                            local_sum += data[j];
                        }
                        task_sum += local_sum;
                        
                        /* Macro expansion with _Pragma to create complex pattern */
                        #define EMIT_CLause_WARNING(clause) \
                            _Pragma("omp error severity(message) message(\"Task processed with " #clause " clause\")")
                        
                        if (i == 0) {
                            EMIT_CLause_WARNING(for);
                        }
                    }
                }
            }
        }
    }
    results[3] = task_sum;
}

/* Complex control flow with embedded OpenMP */
void __attribute__((optimize("O0")))
nested_function_with_openmp(double* data, int size) {
    int switch_var = size % 3;
    
    switch (switch_var) {
        case 0:
            /* Directives inside switch case */
            #pragma omp parallel for schedule(guided)
            for (int i = 0; i < size; i += 2) {
                data[i] *= 1.1;
            }
            break;
            
        case 1:
            /* Sections inside another control structure */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp parallel for simd
                    for (int i = 0; i < size; i++) {
                        data[i] += 0.5;
                    }
                }
                #pragma omp section
                {
                    for (int i = 0; i < size; i++) {
                        if (data[i] > 1000.0) {
                            /* Error directive referencing clause */
                            #pragma omp error severity(warning) \
                                    message("Value too large in sections clause")
                        }
                    }
                }
            }
            break;
            
        case 2:
            /* Taskgroup in default case */
            #pragma omp parallel
            {
                #pragma omp single
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            /* Empty task but clause still present */
                        }
                    }
                }
            }
            break;
    }
}

int main() {
    double* data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double results[4] = {0.0};
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize OpenMP if needed */
    omp_set_num_threads(4);
    omp_set_dynamic(0);
    
    /* Process with various OpenMP clauses */
    process_with_openmp(data, ARRAY_SIZE, results);
    
    /* More complex nested processing */
    for (int iter = 0; iter < 3; iter++) {
        nested_function_with_openmp(data, ARRAY_SIZE);
        
        /* Mixed directive with combined clauses */
        #pragma omp parallel for simd schedule(nonmonotonic:dynamic)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = data[i] / (iter + 2.0);
        }
    }
    
    /* Final computation with all clause types */
    double final_sum = 0.0;
    
    /* Combined parallel for with if clause */
    #pragma omp parallel for reduction(+:final_sum) if(ARRAY_SIZE > 100)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += data[i];
    }
    
    /* Additional sections for coverage */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            /* Use _Pragma for macro expansion */
            #define LOG_CLause(clause) _Pragma("omp message(\"Using " #clause " clause\")")
            LOG_CLause(sections);
        }
        #pragma omp section
        {
            /* Another taskgroup */
            #pragma omp taskgroup task_reduction(+:final_sum)
            {
                #pragma omp task in_reduction(+:final_sum)
                {
                    final_sum += 1.0;
                }
            }
        }
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Results: %f, %f, %f, %f\n", results[0], results[1], results[2], results[3]);
    printf("Final sum: %f\n", final_sum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(data);
    return 0;
}
