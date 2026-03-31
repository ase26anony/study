/* test_openmp_clauses.c
 * Targets uncovered lines in tree-pretty-print.cc: OMP_CLAUSE_FOR, 
 * OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

/* Function with optimization disabled to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_for_clause(int n, double *arr) {
    /* Combined parallel and for clause - triggers both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic) num_threads(4)
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i / (n + 1.0);
    }
    
    /* Complex for clause with multiple arguments */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom: arr[0:n])
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < n) {
                arr[idx] = arr[idx] * 2.0;
            }
        }
    }
    
    /* Error directive with for clause in message - forces pretty-printing */
    #pragma omp error severity(warning) message("Processing with for clause")
    ;
}

/* Another function with sections clause */
void __attribute__((optimize("O0"))) process_with_sections_clause(int n, double *arr, double *sum, double *max) {
    /* Combined parallel and sections clause */
    #pragma omp parallel sections private(n) shared(arr, sum, max)
    {
        #pragma omp section
        {
            double local_sum = 0.0;
            for (int i = 0; i < n; i++) {
                local_sum += arr[i];
            }
            #pragma omp atomic
            *sum += local_sum;
        }
        
        #pragma omp section
        {
            double local_max = -1e30;
            for (int i = 0; i < n; i++) {
                if (arr[i] > local_max) {
                    local_max = arr[i];
                }
            }
            #pragma omp atomic
            if (local_max > *max) *max = local_max;
        }
        
        /* Nested directive with error containing sections clause name */
        #pragma omp section
        {
            #pragma omp error severity(message) message("sections clause processing")
            for (int i = 0; i < n; i += 10) {
                arr[i] = arr[i] * 0.5;
            }
        }
    }
}

/* Function with taskgroup clause */
void __attribute__((optimize("O0"))) process_with_taskgroup_clause(int n, double *arr, double *result) {
    double sum = 0.0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:sum)
            {
                for (int i = 0; i < n; i += 100) {
                    #pragma omp task in_reduction(+:sum) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        int end = (i + 100 < n) ? i + 100 : n;
                        for (int j = i; j < end; j++) {
                            local_sum += arr[j];
                        }
                        sum += local_sum;
                    }
                }
            }
            
            /* Error directive with taskgroup clause name */
            #pragma omp error severity(warning) message("taskgroup clause completed")
        }
    }
    
    *result = sum;
}

/* Complex nested function with mixed clauses */
void __attribute__((optimize("O0"))) nested_processing(int n, double *arr) {
    /* Switch statement with OpenMP inside cases */
    int mode = n % 3;
    
    switch (mode) {
        case 0: {
            /* Parallel for inside switch case */
            #pragma omp parallel for if(n > 1000) schedule(guided)
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] * 3.14159;
            }
            break;
        }
        case 1: {
            /* Parallel sections inside switch case */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    for (int i = 0; i < n/2; i++) {
                        arr[i] = arr[i] + 1.0;
                    }
                }
                #pragma omp section
                {
                    for (int i = n/2; i < n; i++) {
                        arr[i] = arr[i] - 1.0;
                    }
                }
            }
            break;
        }
        case 2: {
            /* Taskgroup inside switch case */
            double temp = 0.0;
            #pragma omp parallel
            {
                #pragma omp single
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task shared(temp)
                        {
                            temp = omp_get_num_threads();
                        }
                    }
                }
            }
            /* Use _Pragma for complex pattern */
            _Pragma("omp error severity(message) message(\"for clause in _Pragma\")")
            break;
        }
    }
}

/* Main function with complex control flow */
int main() {
    const int N = 10000;
    double *array = (double*)malloc(N * sizeof(double));
    double sum = 0.0, max_val = -1e30, task_result = 0.0;
    
    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with parallel for clause */
    process_with_for_clause(N, array);
    
    /* Process with parallel sections clause */
    process_with_sections_clause(N, array, &sum, &max_val);
    
    /* Process with taskgroup clause */
    process_with_taskgroup_clause(N, array, &task_result);
    
    /* Nested processing with mixed clauses */
    nested_processing(N, array);
    
    /* Final computation to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += array[i];
    }
    
    checksum += sum + max_val + task_result;
    
    printf("Final checksum: %f\n", checksum);
    printf("Array[0] = %f, Array[%d] = %f\n", array[0], N-1, array[N-1]);
    
    free(array);
    return 0;
}
