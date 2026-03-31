/* test_omp_clauses.c - Targeting uncovered pretty-print cases for OMP_CLAUSE_FOR, 
   OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define CHUNK_SIZE 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_array_parallel_for(double *arr, int n) {
    int i;
    /* Combined parallel and for clause - triggers both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) private(i) \
        if(n > 1000) num_threads(omp_get_max_threads())
    for (i = 0; i < n; i++) {
        arr[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Nested directive with for clause in complex context */
    if (n > 500) {
        #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
            aligned(arr:64) if(parallel: omp_get_num_threads() > 1)
        for (i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < n) {
                    arr[idx] = arr[idx] * 1.5 + 0.1;
                }
            }
        }
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) compute_reductions(double *arr, int n) {
    double sum = 0.0, max_val = -1e30;
    
    /* Combined parallel sections - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(arr) shared(n)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            /* Force diagnostic with clause name in message */
            #pragma omp error severity(warning) message("Processing in sections clause region")
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
        }
        
        /* Third section with nested for */
        #pragma omp section
        {
            double local_sum = 0.0;
            #pragma omp parallel for reduction(+:local_sum) if(n > 2000)
            for (int i = 0; i < n; i += 3) {
                local_sum += sqrt(fabs(arr[i]));
            }
            sum += local_sum;
        }
    }
    
    return sum + max_val;
}

/* Complex macro using _Pragma to create clause patterns */
#define CREATE_TASKGROUP(reducer, var) \
    _Pragma("omp taskgroup task_reduction(+:" #var ")") \
    { \
        _Pragma("omp task in_reduction(+:" #var ")") \
        var += reducer; \
        _Pragma("omp task in_reduction(+:" #var ")") \
        var += reducer * 2; \
    }

/* Function using taskgroup clause */
double __attribute__((optimize("O0"))) process_with_taskgroup(double *arr, int n) {
    double task_result = 0.0;
    
    /* Taskgroup with explicit task_reduction clause - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_result) \
                allocate(omp_high_bw_mem_alloc: task_result)
            {
                for (int i = 0; i < n; i += n/10) {
                    int end = (i + n/10) < n ? (i + n/10) : n;
                    
                    #pragma omp task firstprivate(i, end) in_reduction(+:task_result) \
                        if(end - i > 10) priority(omp_get_thread_num())
                    {
                        double local_sum = 0.0;
                        for (int j = i; j < end; j++) {
                            local_sum += arr[j] * arr[j];
                        }
                        task_result += local_sum;
                        
                        /* Nested task with error directive containing clause name */
                        if (local_sum > 1000.0) {
                            #pragma omp task
                            {
                                /* Force pretty-printing of 'for' clause name */
                                #pragma omp error severity(message) \
                                    message("Task encountered large sum, would use for clause")
                            }
                        }
                    }
                }
            }
            
            /* Use macro expansion for additional taskgroup */
            CREATE_TASKGROUP(1.5, task_result);
        }
    }
    
    return task_result;
}

/* Function with switch statement containing OpenMP directives */
void __attribute__((optimize("O0"))) process_by_mode(double *arr, int n, int mode) {
    switch (mode) {
        case 1: {
            /* Directives inside switch case */
            #pragma omp parallel for simd schedule(guided) safelen(16) \
                if(simd: n > 100) nontemporal(arr)
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] * 0.5;
            }
            break;
        }
        case 2: {
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp parallel for
                    for (int i = 0; i < n/2; i++) {
                        arr[i] = arr[i] + 1.0;
                    }
                }
                #pragma omp section
                {
                    #pragma omp parallel for
                    for (int i = n/2; i < n; i++) {
                        arr[i] = arr[i] - 1.0;
                    }
                }
            }
            break;
        }
        case 3: {
            double sum = 0.0;
            #pragma omp parallel
            {
                #pragma omp for reduction(+:sum) nowait
                for (int i = 0; i < n; i++) {
                    sum += arr[i];
                }
                
                #pragma omp taskgroup task_reduction(+:sum)
                {
                    #pragma omp task in_reduction(+:sum)
                    sum += 100.0;
                }
            }
            break;
        }
    }
}

int main() {
    double *array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double checksum = 0.0;
    
    /* Phase 1: Use parallel for clause */
    process_array_parallel_for(array, ARRAY_SIZE);
    
    /* Phase 2: Use parallel sections clause */
    checksum += compute_reductions(array, ARRAY_SIZE);
    
    /* Phase 3: Use taskgroup clause */
    checksum += process_with_taskgroup(array, ARRAY_SIZE);
    
    /* Phase 4: Mixed processing with switch */
    for (int mode = 1; mode <= 3; mode++) {
        process_by_mode(array, ARRAY_SIZE, mode);
        
        /* Additional parallel for with runtime schedule */
        #pragma omp parallel for schedule(runtime) \
            if(omp_get_num_procs() > 2) proc_bind(spread)
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = fmod(array[i], 10.0);
        }
    }
    
    /* Final computation to prevent dead code elimination */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        order(concurrent) lastprivate(i)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    checksum += final_sum;
    
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
