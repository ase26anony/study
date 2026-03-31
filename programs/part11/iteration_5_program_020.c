/* Target coverage for tree-pretty-print.cc lines 1434-1445 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1000
#define NUM_ITERATIONS 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n) {
    double sum = 0.0;
    double max_val = -1e30;
    
    /* 1. Combined parallel and for clause - triggers OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp parallel for schedule(dynamic) reduction(+:sum) if(n > 100)
    for (int i = 0; i < n; i++) {
        data[i] = (double)i * 1.5;
        sum += data[i];
        
        /* Use runtime API inside clause block */
        if (omp_get_thread_num() == 0 && i == 0) {
            /* Diagnostic that may trigger pretty-printing */
            #pragma omp message "Processing for clause with dynamic scheduling"
        }
    }
    
    /* 2. Complex for clause with multiple arguments - deeper representation */
    #pragma omp target teams distribute parallel for simd \
            schedule(static, 4) collapse(2) map(tofrom:data[0:n])
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < n) {
                data[idx] *= 1.1;
            }
        }
    }
    
    /* 3. Combined parallel and sections clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(max_val) shared(data, n)
    {
        /* First section */
        #pragma omp section
        {
            max_val = data[0];
            for (int i = 1; i < n/2; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
            /* Nested diagnostic with clause reference */
            #pragma omp error severity(warning) message("sections clause in use")
        }
        
        /* Second section */
        #pragma omp section
        {
            double local_max = data[n/2];
            for (int i = n/2 + 1; i < n; i++) {
                if (data[i] > local_max) local_max = data[i];
            }
            #pragma omp critical
            {
                if (local_max > max_val) max_val = local_max;
            }
        }
        
        /* Third section with nested for */
        #pragma omp section
        {
            double checksum = 0.0;
            #pragma omp parallel for reduction(+:checksum)
            for (int i = 0; i < n; i += 3) {
                checksum += data[i];
            }
            /* Force pretty-printing through macro expansion */
            #define OMP_ERROR_MSG(msg) _Pragma("omp error message \"" msg "\"")
            OMP_ERROR_MSG("Nested for clause inside sections");
        }
    }
    
    printf("After sections: max = %f, sum = %f\n", max_val, sum);
}

/* Function specifically for taskgroup clause coverage */
void __attribute__((optimize("O0"))) taskgroup_example(double *data, int n) {
    double task_sum = 0.0;
    
    /* 4. Taskgroup clause with task_reduction - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < n; i += 10) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        int end = (i + 10 < n) ? i + 10 : n;
                        for (int j = i; j < end; j++) {
                            local_sum += data[j];
                        }
                        task_sum += local_sum;
                        
                        /* Nested directive that references 'for' clause */
                        if (local_sum > 1000) {
                            #pragma omp error severity(message) \
                                message("Task found large sum, for clause was used earlier")
                        }
                    }
                }
            }
            
            /* Additional taskgroup with nested parallel for */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Complex nested structure */
                    #pragma omp parallel for schedule(guided)
                    for (int i = 0; i < n/2; i++) {
                        data[i] = data[i] * 0.5;
                    }
                }
                
                #pragma omp task
                {
                    #pragma omp parallel for schedule(runtime)
                    for (int i = n/2; i < n; i++) {
                        data[i] = data[i] * 2.0;
                    }
                }
            }
        }
    }
    
    printf("Taskgroup reduction sum: %f\n", task_sum);
}

/* Complex control flow with embedded OpenMP */
int __attribute__((optimize("O0"))) nested_switch_example(double *data, int n) {
    int mode = n % 4;
    double result = 0.0;
    
    switch (mode) {
        case 0:
            /* Directives inside switch case */
            #pragma omp parallel for simd reduction(+:result) schedule(static)
            for (int i = 0; i < n; i++) {
                result += data[i];
            }
            break;
            
        case 1:
            #pragma omp parallel sections reduction(max:result)
            {
                #pragma omp section
                {
                    for (int i = 0; i < n/2; i++) {
                        if (data[i] > result) result = data[i];
                    }
                }
                #pragma omp section
                {
                    for (int i = n/2; i < n; i++) {
                        if (data[i] > result) result = data[i];
                    }
                }
            }
            break;
            
        case 2:
            {
                /* Block with taskgroup */
                #pragma omp taskgroup task_reduction(+:result)
                {
                    #pragma omp task in_reduction(+:result)
                    {
                        for (int i = 0; i < n; i += 2) {
                            result += data[i];
                        }
                    }
                    #pragma omp task in_reduction(+:result)
                    {
                        for (int i = 1; i < n; i += 2) {
                            result += data[i];
                        }
                    }
                }
            }
            break;
            
        case 3:
            /* Combined directive with all target clauses */
            #pragma omp parallel
            {
                #pragma omp for schedule(dynamic, 2) nowait
                for (int i = 0; i < n; i++) {
                    data[i] += 1.0;
                }
                
                #pragma omp sections
                {
                    #pragma omp section
                    { result = data[0]; }
                    #pragma omp section
                    { result = data[n-1]; }
                }
                
                #pragma omp taskgroup
                {
                    #pragma omp task
                    { result += 1.0; }
                }
            }
            break;
    }
    
    return (int)result;
}

int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) return 1;
    
    /* Initialize with OpenMP parallel for */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i;
    }
    
    /* Process with various OpenMP constructs */
    process_with_openmp(data, ARRAY_SIZE);
    taskgroup_example(data, ARRAY_SIZE);
    
    /* Complex nested loop with OpenMP */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        double temp = 0.0;
        
        /* Mixed directives in loop */
        if (iter % 3 == 0) {
            #pragma omp parallel for reduction(+:temp) collapse(2)
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < ARRAY_SIZE/10; j++) {
                    int idx = i * (ARRAY_SIZE/10) + j;
                    if (idx < ARRAY_SIZE) {
                        temp += data[idx];
                    }
                }
            }
        } else if (iter % 3 == 1) {
            #pragma omp parallel sections reduction(+:temp)
            {
                #pragma omp section
                {
                    for (int i = 0; i < ARRAY_SIZE/2; i++) {
                        temp += data[i] * 0.5;
                    }
                }
                #pragma omp section
                {
                    for (int i = ARRAY_SIZE/2; i < ARRAY_SIZE; i++) {
                        temp += data[i] * 1.5;
                    }
                }
            }
        } else {
            #pragma omp taskgroup task_reduction(+:temp)
            {
                #pragma omp task in_reduction(+:temp)
                {
                    for (int i = 0; i < ARRAY_SIZE; i += 3) {
                        temp += data[i];
                    }
                }
                #pragma omp task in_reduction(+:temp)
                {
                    for (int i = 1; i < ARRAY_SIZE; i += 3) {
                        temp += data[i];
                    }
                }
                #pragma omp task in_reduction(+:temp)
                {
                    for (int i = 2; i < ARRAY_SIZE; i += 3) {
                        temp += data[i];
                    }
                }
            }
        }
        
        /* Use result to prevent dead code elimination */
        data[iter % ARRAY_SIZE] = temp / ARRAY_SIZE;
    }
    
    /* Final computation using switch example */
    int checksum = nested_switch_example(data, ARRAY_SIZE);
    printf("Final checksum: %d\n", checksum);
    
    /* Force diagnostic that may trigger pretty-printing */
    #pragma omp error severity(warning) \
        message("Compilation should have printed for, parallel, sections, and taskgroup clauses")
    
    free(data);
    return 0;
}
