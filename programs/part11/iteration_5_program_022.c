/* test_omp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define ARRAY_SIZE 10000
#define CHUNK_SIZE 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_array(double *arr, int n, double *sum_result, double *max_result) {
    double local_sum = 0.0;
    double local_max = -__DBL_MAX__;
    
    /* 1. parallel for clause - triggers OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) reduction(+:local_sum) reduction(max:local_max)
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i * 1.5;
        local_sum += arr[i];
        if (arr[i] > local_max) local_max = arr[i];
    }
    
    /* Combined parallel for with collapse - complex for clause representation */
    #pragma omp target teams distribute parallel for simd collapse(2) schedule(static, 4) if(n > 1000)
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            arr[i * 50 + j] += 0.001;
        }
    }
    
    *sum_result = local_sum;
    *max_result = local_max;
}

/* Function using sections clause - triggers OMP_CLAUSE_SECTIONS */
void __attribute__((optimize("O0"))) compute_reductions(double *arr, int n, double *sum1, double *sum2) {
    double sum_even = 0.0, sum_odd = 0.0;
    
    /* 2. parallel sections clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(arr) shared(sum_even, sum_odd, n)
    {
        #pragma omp section
        {
            for (int i = 0; i < n; i += 2) {
                sum_even += arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = 1; i < n; i += 2) {
                sum_odd += arr[i];
            }
        }
        
        /* Additional section to ensure sections clause is fully represented */
        #pragma omp section
        {
            /* Empty section but still triggers clause processing */
        }
    }
    
    *sum1 = sum_even;
    *sum2 = sum_odd;
}

/* Function using taskgroup clause - triggers OMP_CLAUSE_TASKGROUP */
double __attribute__((optimize("O0"))) task_based_computation(double *arr, int n) {
    double total = 0.0;
    
    /* 3. taskgroup clause with task_reduction */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (int i = 0; i < n; i += CHUNK_SIZE) {
                    #pragma omp task in_reduction(+:total) firstprivate(i)
                    {
                        double chunk_sum = 0.0;
                        int end = (i + CHUNK_SIZE < n) ? i + CHUNK_SIZE : n;
                        for (int j = i; j < end; j++) {
                            chunk_sum += arr[j];
                        }
                        total += chunk_sum;
                        
                        /* Nested task with error directive containing clause name */
                        #pragma omp task
                        {
                            /* Force pretty-printing of clause names via error directive */
                            #pragma omp error message("Task contains for clause analysis")
                            
                            /* Additional directive to ensure for clause appears in dump */
                            #pragma omp parallel for simd simdlen(4)
                            for (int k = 0; k < 10; k++) {
                                /* Dummy computation */
                                arr[0] += 0.0001 * k;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return total;
}

/* Complex macro expansion using _Pragma to create parsing challenges */
#define CREATE_PARALLEL_REGION(iterations) \
    _Pragma("omp parallel for if(iterations > 100)") \
    for (int macro_i = 0; macro_i < iterations; macro_i++)

#define CREATE_SECTIONS_REGION \
    _Pragma("omp parallel sections") \
    { \
        _Pragma("omp section") \
        { /* section 1 */ } \
        _Pragma("omp section") \
        { /* section 2 */ } \
    }

/* Function with mixed OpenMP and C control flow */
void __attribute__((optimize("O0"))) complex_control_flow(double *arr, int n) {
    int switch_var = omp_get_thread_num() % 3;
    
    switch (switch_var) {
        case 0:
            /* Directives inside switch cases */
            #pragma omp parallel for ordered
            for (int i = 0; i < n; i++) {
                #pragma omp ordered
                arr[i] = arr[i] * 2.0;
            }
            break;
            
        case 1:
            /* Nested parallel regions */
            #pragma omp parallel
            {
                #pragma omp for nowait
                for (int i = 0; i < n/2; i++) {
                    arr[i] = arr[i] / 2.0;
                }
                
                #pragma omp sections
                {
                    #pragma omp section
                    { arr[0] += 1.0; }
                    
                    #pragma omp section  
                    { arr[1] -= 1.0; }
                }
            }
            break;
            
        case 2:
            /* Using macro expansions */
            CREATE_PARALLEL_REGION(n/4);
            for (int i = 0; i < n/4; i++) {
                arr[i] = -arr[i];
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
    
    double sum_result, max_result, sum_even, sum_odd, task_total;
    
    /* Process 1: Initial computation with parallel for */
    process_array(array, ARRAY_SIZE, &sum_result, &max_result);
    
    /* Process 2: Sections-based reduction */
    compute_reductions(array, ARRAY_SIZE, &sum_even, &sum_odd);
    
    /* Process 3: Taskgroup-based computation */
    task_total = task_based_computation(array, ARRAY_SIZE);
    
    /* Process 4: Complex control flow */
    complex_control_flow(array, ARRAY_SIZE);
    
    /* Final checksum to prevent dead code elimination */
    double final_checksum = sum_result + max_result + sum_even + sum_odd + task_total;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array[i];
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Threads used: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
