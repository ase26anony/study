/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define ARRAY_SIZE 10000
#define NUM_TASKS 10

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_array_parallel_for(double *arr) {
    #pragma omp parallel for schedule(dynamic) default(none) shared(arr)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = (double)i * 1.5;
    }
    
    /* Nested directive combining parallel and for clauses */
    #pragma omp parallel for simd collapse(2) schedule(static, 4) \
        if(omp_get_num_threads() > 1)
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i * 100 + j] += 0.001;
        }
    }
}

/* Function using parallel sections clause */
double __attribute__((optimize("O0"))) compute_reductions(double *arr) {
    double sum = 0.0;
    double max_val = -__DBL_MAX__;
    
    #pragma omp parallel sections default(none) shared(arr, sum, max_val) \
        num_threads(2)
    {
        #pragma omp section
        {
            #pragma omp parallel for reduction(+:sum) if(omp_in_parallel())
            for (int i = 0; i < ARRAY_SIZE; i++) {
                sum += arr[i];
            }
        }
        
        #pragma omp section
        {
            #pragma omp parallel for reduction(max:max_val)
            for (int i = 0; i < ARRAY_SIZE; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
    }
    
    /* Force diagnostic with sections clause name */
    #pragma omp error severity(warning) message("Processing sections clause")
    
    return sum + max_val;
}

/* Complex directive with distribute parallel for simd */
void __attribute__((optimize("O0"))) process_matrix(double *matrix) {
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom: matrix[0:10000])
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i * 100 + j] *= 2.0;
        }
    }
}

/* Function using taskgroup clause */
double __attribute__((optimize("O0"))) task_based_computation(double *arr) {
    double task_sum = 0.0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel master default(none) shared(arr, task_sum)
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            for (int t = 0; t < NUM_TASKS; t++) {
                #pragma omp task in_reduction(+:task_sum) firstprivate(t) \
                    shared(arr)
                {
                    double local_sum = 0.0;
                    int chunk = ARRAY_SIZE / NUM_TASKS;
                    int start = t * chunk;
                    int end = (t == NUM_TASKS - 1) ? ARRAY_SIZE : start + chunk;
                    
                    for (int i = start; i < end; i++) {
                        local_sum += arr[i];
                    }
                    
                    task_sum += local_sum;
                    
                    /* Nested task with error directive containing clause name */
                    #pragma omp task if(0)
                    {
                        /* This will trigger pretty-printing of 'for' clause */
                        _Pragma("omp error severity(message) message(\"for clause in task\")")
                    }
                }
            }
        }
    }
    
    /* Macro expansion to create complex pretty-printing pattern */
    #define EMIT_CLAUSE_NAME(clause) _Pragma("omp error message(\"" #clause "\")")
    EMIT_CLAUSE_NAME(taskgroup);
    
    return task_sum;
}

/* Function with switch statement containing OpenMP directives */
void __attribute__((optimize("O0"))) process_with_switch(int mode, double *arr) {
    switch (mode) {
        case 1:
            /* Combined parallel for directive */
            #pragma omp parallel for ordered schedule(guided)
            for (int i = 0; i < ARRAY_SIZE; i++) {
                #pragma omp ordered
                arr[i] = arr[i] / (i + 1);
            }
            break;
            
        case 2:
            /* Nested parallel regions */
            #pragma omp parallel default(none) shared(arr)
            {
                #pragma omp for nowait
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    arr[i] += omp_get_thread_num();
                }
            }
            break;
            
        default:
            /* Directive with multiple clauses */
            #pragma omp parallel for simd schedule(nonmonotonic:dynamic) \
                if(omp_get_max_threads() > 4)
            for (int i = 0; i < ARRAY_SIZE; i++) {
                arr[i] = sqrt(arr[i]);
            }
    }
}

int main() {
    double *array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!array) return 1;
    
    double checksum = 0.0;
    
    /* 1. Use parallel for clause */
    process_array_parallel_for(array);
    
    /* 2. Use parallel sections clause */
    checksum += compute_reductions(array);
    
    /* 3. Use taskgroup clause */
    checksum += task_based_computation(array);
    
    /* 4. Complex matrix processing with combined clauses */
    process_matrix(array);
    
    /* 5. Process with switch containing various directives */
    for (int mode = 1; mode <= 3; mode++) {
        process_with_switch(mode, array);
    }
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum) \
        if(ARRAY_SIZE > 1000)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array[i];
    }
    
    /* Force diagnostic for parallel clause */
    #pragma omp error severity(warning) \
        message("parallel clause pretty-printing test")
    
    printf("Final checksum: %f\n", checksum);
    printf("Using %d OpenMP threads\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
