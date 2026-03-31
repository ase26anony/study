/* test_openmp_clauses.c
 * Targets uncovered lines in GCC's tree-pretty-print.cc:
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define ARRAY_SIZE 10000
#define CHUNK_SIZE 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(double *data, int n, double *result_sum, double *result_max) {
    double local_sum = 0.0;
    double local_max = -__DBL_MAX__;
    
    /* Combined parallel and for clauses - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) reduction(+:local_sum) \
                         reduction(max:local_max) if(n > 1000)
    for (int i = 0; i < n; i++) {
        data[i] = (double)i * 1.5;
        local_sum += data[i];
        if (data[i] > local_max) local_max = data[i];
        
        /* Nested OpenMP construct with complex for clause */
        if (i % 1000 == 0) {
            /* Complex for clause with multiple arguments */
            #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
                             simdlen(4) if(omp_get_num_threads() > 1)
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 10; k++) {
                    data[(i + j + k) % n] *= 1.001;
                }
            }
        }
    }
    
    *result_sum = local_sum;
    *result_max = local_max;
}

/* Another function using sections clause */
void __attribute__((optimize("O0")))
process_sections(double *data, int n, double *section_results) {
    /* Combined parallel and sections clauses */
    #pragma omp parallel sections num_threads(4) private(n) \
                         shared(data, section_results)
    {
        /* First section */
        #pragma omp section
        {
            double sum = 0.0;
            for (int i = 0; i < n/2; i++) {
                sum += data[i];
            }
            section_results[0] = sum;
            
            /* Error directive with clause name in message */
            #pragma omp error severity(warning) message("Processing section with for clause")
        }
        
        /* Second section */
        #pragma omp section
        {
            double max_val = -__DBL_MAX__;
            for (int i = n/2; i < n; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
            section_results[1] = max_val;
        }
        
        /* Third section using macro expansion with _Pragma */
        #pragma omp section
        {
            #define TASK_PRG _Pragma("omp taskgroup task_reduction(+:section_results[2])")
            TASK_PRG {
                section_results[2] = 0.0;
                for (int i = 0; i < n; i += 100) {
                    #pragma omp task in_reduction(+:section_results[2])
                    {
                        double partial = 0.0;
                        for (int j = i; j < i + 100 && j < n; j++) {
                            partial += data[j];
                        }
                        section_results[2] += partial;
                    }
                }
            }
            #undef TASK_PRG
        }
    }
}

/* Function using taskgroup clause explicitly */
void __attribute__((optimize("O0")))
process_taskgroup(double *data, int n, double *task_result) {
    *task_result = 0.0;
    
    /* Explicit taskgroup clause with task_reduction */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:*task_result)
            {
                for (int i = 0; i < n; i += 500) {
                    #pragma omp task in_reduction(+:*task_result)
                    {
                        double partial_sum = 0.0;
                        int end = (i + 500 < n) ? i + 500 : n;
                        for (int j = i; j < end; j++) {
                            partial_sum += data[j] * 0.5;
                        }
                        *task_result += partial_sum;
                    }
                }
                
                /* Nested task with error directive */
                #pragma omp task
                {
                    /* Force pretty-printing of for clause via error message */
                    #pragma omp error severity(message) \
                                message("Task completed: for clause processed")
                }
            }
        }
    }
}

/* Complex control flow with OpenMP directives */
void __attribute__((optimize("O0")))
complex_flow_with_openmp(double *data, int n) {
    int switch_var = n % 3;
    
    switch (switch_var) {
        case 0: {
            /* Parallel for in switch case */
            #pragma omp parallel for if(n > 100) schedule(guided)
            for (int i = 0; i < n; i++) {
                data[i] = data[i] * 2.0;
            }
            break;
        }
        case 1: {
            /* Parallel sections in switch case */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    for (int i = 0; i < n/2; i++) {
                        data[i] += 1.0;
                    }
                }
                #pragma omp section
                {
                    for (int i = n/2; i < n; i++) {
                        data[i] -= 1.0;
                    }
                }
            }
            break;
        }
        case 2: {
            /* Taskgroup in switch case */
            double temp = 0.0;
            #pragma omp parallel
            {
                #pragma omp single
                {
                    #pragma omp taskgroup task_reduction(+:temp)
                    {
                        #pragma omp task in_reduction(+:temp)
                        { temp += 1.0; }
                        #pragma omp task in_reduction(+:temp)
                        { temp += 2.0; }
                    }
                }
            }
            data[0] += temp;
            break;
        }
    }
    
    /* Loop with nested OpenMP */
    for (int iter = 0; iter < 3; iter++) {
        if (iter == 1) {
            /* Distribute parallel for simd with complex for clause */
            #pragma omp distribute parallel for simd schedule(static, 8) \
                         collapse(2) simdlen(8) ordered(2)
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 16; j++) {
                    int idx = (i * 16 + j) % n;
                    data[idx] = data[idx] * (1.0 + iter * 0.1);
                }
            }
        }
    }
}

int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double sum_result, max_result;
    double section_results[3] = {0.0, 0.0, 0.0};
    double taskgroup_result = 0.0;
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    
    /* Process with combined parallel for clause */
    process_with_openmp(data, ARRAY_SIZE, &sum_result, &max_result);
    
    /* Process with parallel sections clause */
    process_sections(data, ARRAY_SIZE, section_results);
    
    /* Process with explicit taskgroup clause */
    process_taskgroup(data, ARRAY_SIZE, &taskgroup_result);
    
    /* Complex control flow */
    complex_flow_with_openmp(data, ARRAY_SIZE);
    
    /* Final computation to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) schedule(static)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i];
    }
    
    checksum += sum_result + max_result + section_results[0] + 
                section_results[1] + section_results[2] + taskgroup_result;
    
    printf("Final checksum: %f\n", checksum);
    printf("Results: sum=%f, max=%f\n", sum_result, max_result);
    printf("Sections: %f, %f, %f\n", section_results[0], section_results[1], section_results[2]);
    printf("Taskgroup result: %f\n", taskgroup_result);
    
    free(data);
    return 0;
}
