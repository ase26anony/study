/* test_omp_clauses.c - Target coverage for tree-pretty-print.cc lines 1434-1445 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_array_parallel_for(double *arr, int n) {
    int i;
    /* Combined parallel and for clauses - triggers both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic) private(i) shared(arr, n)
    for (i = 0; i < n; i++) {
        arr[i] = (double)i * 3.14159;
    }
    
    /* Nested directive with for clause in complex context */
    #pragma omp target teams distribute parallel for simd schedule(static, 4) collapse(2) \
                map(tofrom: arr[0:n])
    for (i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            arr[i*2 + j] += 1.0;
        }
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) compute_reductions(double *arr, int n) {
    double sum = 0.0;
    double max_val = arr[0];
    
    /* Combined parallel and sections clauses */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < n; i += 2) {
                sum += arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = 1; i < n; i += 2) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
        }
        
        /* Additional section with error directive containing clause name */
        #pragma omp section
        {
            /* Force diagnostic with clause name in message */
            #pragma omp error severity(warning) message("Processing with for clause")
            int dummy = 0;
            #pragma omp atomic
            dummy += 1;
        }
    }
    
    return sum + max_val;
}

/* Complex task construct with taskgroup clause */
void __attribute__((optimize("O0"))) process_tasks(double *arr, int n, double *result) {
    double task_sum = 0.0;
    
    /* Taskgroup with explicit task_reduction clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < 4; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i) shared(arr, n)
                    {
                        double local_sum = 0.0;
                        int start = i * (n / 4);
                        int end = (i + 1) * (n / 4);
                        
                        for (int j = start; j < end; j++) {
                            local_sum += arr[j];
                        }
                        
                        #pragma omp atomic
                        task_sum += local_sum;
                        
                        /* Nested error directive using _Pragma */
                        #define EMIT_CLAUES_MSG() _Pragma("omp error severity(message) message(\"taskgroup clause active\")")
                        EMIT_CLAUES_MSG();
                    }
                }
            }
        }
    }
    
    *result = task_sum;
}

/* Function with mixed OpenMP and complex C control flow */
double __attribute__((optimize("O0"))) complex_control_flow(double *arr, int n) {
    double final_result = 0.0;
    int i;
    
    /* OpenMP inside switch statement */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        switch (iter % 3) {
            case 0:
                /* Parallel for with runtime calls */
                #pragma omp parallel for schedule(runtime) if(n > 1000)
                for (i = 0; i < n; i++) {
                    arr[i] += omp_get_thread_num() * 0.001;
                }
                break;
                
            case 1:
                /* Sections with conditional compilation */
                #ifdef _OPENMP
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        #pragma omp critical
                        final_result += 1.0;
                    }
                    #pragma omp section
                    {
                        #pragma omp critical
                        final_result -= 0.5;
                    }
                }
                #endif
                break;
                
            case 2:
                /* Taskgroup in loop */
                #pragma omp parallel
                {
                    #pragma omp single
                    {
                        #pragma omp taskgroup
                        {
                            #pragma omp task
                            {
                                /* Force diagnostic during compilation */
                                #pragma GCC diagnostic push
                                #pragma GCC diagnostic warning "-Wunknown-pragmas"
                                #pragma omp fake_clause_to_trigger_error
                                #pragma GCC diagnostic pop
                            }
                        }
                    }
                }
                break;
        }
    }
    
    return final_result;
}

int main() {
    double *array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double checksum = 0.0;
    
    /* Process 1: Parallel for */
    process_array_parallel_for(array, ARRAY_SIZE);
    
    /* Process 2: Parallel sections with reductions */
    checksum += compute_reductions(array, ARRAY_SIZE);
    
    /* Process 3: Taskgroup with tasks */
    double task_result;
    process_tasks(array, ARRAY_SIZE, &task_result);
    checksum += task_result;
    
    /* Process 4: Complex control flow */
    checksum += complex_control_flow(array, ARRAY_SIZE);
    
    /* Final computation to prevent dead code elimination */
    double final_validation = 0.0;
    #pragma omp parallel for reduction(+:final_validation)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_validation += array[i];
    }
    
    checksum += final_validation;
    
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
