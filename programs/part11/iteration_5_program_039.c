/* test_openmp_clauses.c - Targeting uncovered pretty-print lines for OMP clauses */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_array_parallel_for(double *arr, int n) {
    int i;
    
    /* TARGET: OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL combined */
    #pragma omp parallel for schedule(static, 4) collapse(2) if(n > 1000)
    for (i = 0; i < n; i++) {
        arr[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Complex directive with multiple clauses including 'for' */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        num_threads(omp_get_max_threads() / 2)
    for (i = 0; i < n; i += 2) {
        arr[i] = sqrt(fabs(arr[i])) + 1.0;
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) compute_reductions(double *arr, int n) {
    double sum = 0.0;
    double max_val = -INFINITY;
    
    /* TARGET: OMP_CLAUSE_SECTIONS and OMP_CLAUSE_PARALLEL combined */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
        }
        
        /* Additional section to ensure sections clause is fully exercised */
        #pragma omp section
        {
            /* Trigger diagnostic with clause name in message */
            #pragma omp error severity(warning) message("Processing sections clause")
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
double __attribute__((optimize("O0"))) process_with_taskgroup(double *arr, int n) {
    double total = 0.0;
    
    /* TARGET: OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (int i = 0; i < n; i += n/10) {
                    #pragma omp task in_reduction(+:total) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        int end = i + n/10;
                        if (end > n) end = n;
                        
                        for (int j = i; j < end; j++) {
                            local_sum += arr[j] * arr[j];
                        }
                        
                        total += local_sum;
                        
                        /* Nested directive inside task */
                        if (local_sum > 100.0) {
                            #pragma omp parallel for schedule(dynamic)
                            for (int k = 0; k < 10; k++) {
                                /* Force pretty-printing of 'for' clause */
                                #pragma omp error severity(message) \
                                    message("Task contains for clause directive")
                            }
                        }
                    }
                }
            }
        }
    }
    
    return total;
}

/* Complex control flow with mixed OpenMP directives */
void __attribute__((optimize("O0"))) nested_control_flow(double *arr, int n) {
    int i, j;
    
    /* Switch statement with OpenMP inside */
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0:
                /* Directives in switch case */
                #pragma omp parallel for private(j) schedule(guided)
                for (i = 0; i < n; i++) {
                    for (j = 0; j < 10; j++) {
                        arr[i] += j * 0.1;
                    }
                }
                break;
                
            case 1:
                /* Another sections directive */
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        #pragma omp parallel for simd
                        for (i = 0; i < n; i++) {
                            arr[i] = log(fabs(arr[i]) + 1.0);
                        }
                    }
                    
                    #pragma omp section
                    {
                        /* Empty section but still triggers clause */
                    }
                }
                break;
                
            case 2:
                /* Taskgroup with reduction */
                {
                    double local_sum = 0.0;
                    #pragma omp taskgroup task_reduction(+:local_sum)
                    {
                        #pragma omp task in_reduction(+:local_sum)
                        {
                            local_sum = 1.0;
                        }
                    }
                }
                break;
        }
    }
}

/* Macro expansion to force pretty-printing */
#define CREATE_PARALLEL_FOR_LOOP(arr, n) \
    _Pragma("omp parallel for schedule(static)") \
    for (int i = 0; i < (n); i++) { \
        (arr)[i] *= 1.1; \
    }

#define CREATE_SECTIONS_REGION() \
    _Pragma("omp parallel sections") { \
        _Pragma("omp section") { int x = 1; } \
        _Pragma("omp section") { int y = 2; } \
    }

int main() {
    double *array = (double *)malloc(ARRAY_SIZE * sizeof(double));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double checksum = 0.0;
    
    /* Initialize array with parallel for */
    process_array_parallel_for(array, ARRAY_SIZE);
    
    /* Compute using sections */
    checksum += compute_reductions(array, ARRAY_SIZE);
    
    /* Process with taskgroup */
    checksum += process_with_taskgroup(array, ARRAY_SIZE);
    
    /* Nested control flow */
    nested_control_flow(array, ARRAY_SIZE);
    
    /* Use macro expansions */
    CREATE_PARALLEL_FOR_LOOP(array, ARRAY_SIZE);
    CREATE_SECTIONS_REGION();
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array[i];
    }
    
    /* Additional directive to trigger diagnostics */
    #pragma omp error severity(warning) \
        message("Final check: for, parallel, sections, taskgroup clauses processed")
    
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
