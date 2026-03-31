/* test_openmp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define CHUNK_SIZE 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n) {
    double sum = 0.0;
    double max_val = -INFINITY;
    int i, j;
    
    /* 1. TARGET: OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL 
       Combined directive with explicit arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
            private(j) shared(data) reduction(+:sum) if(n > 1000)
    for (i = 0; i < n; i += CHUNK_SIZE) {
        for (j = 0; j < CHUNK_SIZE && (i + j) < n; j++) {
            data[i + j] = (double)(i + j) * 0.01;
            sum += data[i + j];
        }
    }
    
    /* Force diagnostic with clause name in message */
    #pragma omp error severity(warning) message("Processing with for clause completed")
    
    /* 2. TARGET: OMP_CLAUSE_SECTIONS and OMP_CLAUSE_PARALLEL 
       Nested within switch statement for complex scope */
    int section_id = omp_get_thread_num() % 2;
    switch (section_id) {
        case 0: {
            /* Combined parallel sections directive */
            #pragma omp parallel sections private(i) shared(data, n, &max_val) \
                    num_threads(4) if(n > 500)
            {
                /* First section with reduction */
                #pragma omp section
                {
                    double local_max = -INFINITY;
                    for (i = 0; i < n/2; i++) {
                        if (data[i] > local_max) local_max = data[i];
                    }
                    #pragma omp critical
                    {
                        if (local_max > max_val) max_val = local_max;
                    }
                }
                
                /* Second section with different operation */
                #pragma omp section
                {
                    double local_sum = 0.0;
                    for (i = n/2; i < n; i++) {
                        local_sum += sqrt(fabs(data[i]));
                    }
                    #pragma omp atomic
                    sum += local_sum;
                }
                
                /* Third section for additional coverage */
                #pragma omp section
                {
                    #pragma omp error severity(message) \
                            message("Inside sections clause region")
                }
            }
            break;
        }
        default:
            /* Alternative path with _Pragma for complex pattern */
            _Pragma("omp parallel for schedule(dynamic)")
            for (i = 0; i < n; i++) {
                data[i] += 1.0;
            }
            break;
    }
    
    /* 3. TARGET: OMP_CLAUSE_TASKGROUP 
       Within nested function for scope complexity */
    void __attribute__((optimize("O0"))) process_tasks(double *arr, int size, double *result) {
        double task_sum = 0.0;
        
        /* Taskgroup with explicit task_reduction clause */
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            /* Spawn multiple tasks */
            for (int idx = 0; idx < size; idx += size/10) {
                #pragma omp task firstprivate(idx) shared(arr, size) \
                        in_reduction(+:task_sum) if(idx < size/2)
                {
                    double local = 0.0;
                    int end = idx + size/10;
                    if (end > size) end = size;
                    for (int k = idx; k < end; k++) {
                        local += arr[k] * arr[k];
                    }
                    task_sum += local;
                    
                    /* Nested task with error directive containing clause name */
                    #pragma omp task if(0)
                    {
                        #pragma omp error severity(warning) \
                                message("Nested task referencing for clause")
                    }
                }
            }
            
            /* Additional task with sections */
            #pragma omp task
            {
                #pragma omp sections
                {
                    #pragma omp section
                    { task_sum *= 0.99; }
                    #pragma omp section  
                    { task_sum += 1.0; }
                }
            }
        }
        
        *result = task_sum;
    }
    
    double task_result;
    process_tasks(data, n, &task_result);
    
    /* Final computation using all results */
    sum = sum + max_val + task_result;
    
    /* Diagnostic that should trigger pretty-printing */
    #pragma omp error severity(warning) \
            message("Clauses used: for, parallel, sections, taskgroup")
}

/* Another function with different clause combinations */
void __attribute__((optimize("O0"))) test_mixed_clauses(int iterations) {
    /* Complex directive combining multiple target clauses */
    #pragma omp target teams distribute parallel for simd \
            map(tofrom:iterations) if(iterations>100) schedule(guided)
    for (int i = 0; i < iterations; i++) {
        /* Empty but ensures directive is processed */
    }
    
    /* Standalone sections directive */
    #pragma omp sections
    {
        #pragma omp section
        {
            /* Nested parallel for */
            #pragma omp parallel for
            for (int j = 0; j < 10; j++) {
                /* Minimal work */
            }
        }
        #pragma omp section
        {
            /* Taskgroup within section */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Empty task */
                }
            }
        }
    }
}

int main() {
    double *array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!array) return 1;
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    omp_set_dynamic(0);
    
    /* Process with various OpenMP clauses */
    process_with_openmp(array, ARRAY_SIZE);
    
    /* Additional test with different clause combinations */
    test_mixed_clauses(1000);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) schedule(static)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array[i];
    }
    
    /* Use results to prevent optimization */
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
