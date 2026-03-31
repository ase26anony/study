/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define M 100
#define CHUNK_SIZE 4

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"), noinline)) 
process_with_parallel_for(double *arr, int n) {
    int i;
    
    /* Combined parallel and for clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(static, CHUNK_SIZE) private(i) \
        if(n > 100) num_threads(omp_get_max_threads()/2)
    for (i = 0; i < n; i++) {
        arr[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Complex directive with for clause and arguments */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom:arr[0:n]) \
        if(omp_get_num_teams() > 1)
    for (i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            arr[i*2 + j] += 0.5 * arr[i*2 + j];
        }
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) 
process_with_sections(double *arr, int n) {
    double sum = 0.0, max_val = -1e30;
    
    /* Combined parallel and sections clause */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(n) shared(arr)
    {
        /* OMP_CLAUSE_SECTIONS will be pretty-printed here */
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
        
        /* Additional section with nested directive */
        #pragma omp section
        {
            #pragma omp parallel for simd reduction(+:sum)
            for (int i = 0; i < n; i += 2) {
                sum += arr[i] * 0.1;
            }
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
void __attribute__((optimize("O0"))) 
process_with_taskgroup(double *arr, int n, double *result) {
    double sum = 0.0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:sum)
        {
            /* OMP_CLAUSE_TASKGROUP will be pretty-printed here */
            for (int i = 0; i < n; i += M) {
                #pragma omp task in_reduction(+:sum) firstprivate(i) \
                    if(i < n/2) final(i > n/4)
                {
                    double local_sum = 0.0;
                    int end = (i + M < n) ? i + M : n;
                    for (int j = i; j < end; j++) {
                        local_sum += arr[j];
                    }
                    sum += local_sum;
                    
                    /* Force diagnostic with clause name in message */
                    if (local_sum > 1000.0) {
                        /* This should trigger pretty-printing of clauses */
                        #pragma omp error severity(warning) \
                            message("High sum in task with for clause context")
                    }
                }
            }
        }
        
        /* Nested taskgroup inside parallel region */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                /* Use _Pragma to create complex pattern */
                _Pragma("omp critical(update)")
                {
                    *result = sum;
                }
            }
        }
    }
}

/* Complex control flow with mixed OpenMP directives */
void __attribute__((optimize("O0"))) 
complex_control_flow(double *arr, int n) {
    int i, j;
    
    /* Switch statement with OpenMP inside cases */
    for (i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                /* Directive with for clause in switch case */
                #pragma omp parallel for ordered schedule(dynamic) \
                    if(omp_in_parallel())
                for (j = 0; j < n/3; j++) {
                    #pragma omp ordered
                    arr[j] = arr[j] * 2.0;
                }
                break;
                
            case 1:
                /* Sections clause in another case */
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        /* Macro expansion with _Pragma */
                        #define PROCESS_CHUNK(start, end) \
                            _Pragma("omp parallel for simd") \
                            for (int k = start; k < end; k++) { \
                                arr[k] = sqrt(fabs(arr[k])); \
                            }
                        
                        PROCESS_CHUNK(n/3, 2*n/3)
                        #undef PROCESS_CHUNK
                    }
                    
                    #pragma omp section
                    {
                        /* Empty section but clause still present */
                    }
                }
                break;
                
            case 2:
                /* Taskgroup in the last case */
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        /* Nested loop with OpenMP */
                        #pragma omp parallel for collapse(2) \
                            schedule(guided) if(n > 500)
                        for (int x = 2*n/3; x < n; x += 10) {
                            for (int y = 0; y < 10 && x+y < n; y++) {
                                arr[x+y] = log(fabs(arr[x+y]) + 1.0);
                            }
                        }
                    }
                }
                break;
        }
    }
}

/* Function that forces diagnostic generation */
void __attribute__((optimize("O0"))) 
force_diagnostics(void) {
    int dummy = 0;
    
    /* These should trigger pretty-printing during compilation */
    
    /* 1. Error directive with clause names in message */
    #pragma omp error severity(warning) \
        message("Testing pretty-print of clauses: for, parallel, sections, taskgroup")
    
    /* 2. Invalid clause usage to force error messages */
    #if 0  /* Disabled to avoid compilation failure, but shows pattern */
    #pragma omp parallel for invalid_clause  /* Will cause error with pretty-printed clause */
    for (int i = 0; i < 10; i++) {
        dummy += i;
    }
    #endif
    
    /* 3. Complex macro with _Pragma */
    #define OMP_FOR_LOOP(var, start, end) \
        _Pragma("omp parallel for schedule(static)") \
        for (int var = start; var < end; var++)
    
    OMP_FOR_LOOP(k, 0, 5) {
        dummy += k;
    }
    #undef OMP_FOR_LOOP
}

int main(void) {
    double *array;
    double result1, result2, final_result;
    int i;
    
    /* Allocate and initialize array */
    array = (double *)malloc(N * sizeof(double));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some values */
    #pragma omp parallel for schedule(static)
    for (i = 0; i < N; i++) {
        array[i] = (double)i / N;
    }
    
    /* Call functions with different OpenMP clauses */
    process_with_parallel_for(array, N);
    
    result1 = process_with_sections(array, N);
    
    process_with_taskgroup(array, N, &result2);
    
    complex_control_flow(array, N);
    
    force_diagnostics();
    
    /* Compute final checksum to prevent dead code elimination */
    final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
        schedule(dynamic, 2) if(N > 100)
    for (i = 0; i < N; i++) {
        final_result += array[i];
    }
    
    final_result += result1 + result2;
    
    printf("Final result: %f\n", final_result);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    
    return 0;
}
