/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define N 1000
#define CHUNK_SIZE 64

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_array(double *arr, int n) {
    double sum = 0.0;
    double max_val = -__DBL_MAX__;
    
    /* 1. Use 'for' clause with arguments - triggers OMP_CLAUSE_FOR */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(sum) shared(arr, n) if(n > 100)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i] += 0.1 * (i + j);
        }
    }
    
    /* 2. Use 'parallel' clause - triggers OMP_CLAUSE_PARALLEL */
    #pragma omp parallel default(none) shared(arr, n, sum, max_val) \
        num_threads(omp_get_max_threads())
    {
        double local_sum = 0.0;
        double local_max = -__DBL_MAX__;
        
        /* Nested directive combining 'parallel' and 'for' */
        #pragma omp for schedule(dynamic, CHUNK_SIZE) nowait
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
            if (arr[i] > local_max) local_max = arr[i];
        }
        
        #pragma omp atomic
        sum += local_sum;
        
        #pragma omp critical
        {
            if (local_max > max_val) max_val = local_max;
        }
        
        /* Force diagnostic with clause name in message */
        #pragma omp error severity(warning) message("Processing with parallel clause")
    }
    
    /* 3. Use 'sections' clause - triggers OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(sum, max_val) shared(arr, n)
    {
        #pragma omp section
        {
            sum = 0.0;
            #pragma omp parallel for reduction(+:sum) if(n > 500)
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            printf("Section 1 sum: %f\n", sum);
        }
        
        #pragma omp section
        {
            max_val = -__DBL_MAX__;
            #pragma omp parallel for reduction(max:max_val)
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
            printf("Section 2 max: %f\n", max_val);
        }
        
        /* Additional section with error directive */
        #pragma omp section
        {
            /* Use _Pragma to create complex pattern */
            #define OMP_ERROR_MSG(msg) _Pragma("omp error severity(message) message(msg)")
            OMP_ERROR_MSG("sections clause processing complete");
        }
    }
    
    /* 4. Use 'taskgroup' clause - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    #pragma omp single
    {
        double task_sum = 0.0;
        
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task in_reduction(+:task_sum) firstprivate(i) \
                    depend(out: arr[i % n])
                {
                    double val = arr[i % n] * i;
                    task_sum += val;
                    
                    /* Nested task with error containing clause name */
                    if (i == 5) {
                        #pragma omp task
                        {
                            /* This should trigger pretty-printing of 'for' clause */
                            #pragma omp error severity(warning) \
                                message("Task 5: simulating for clause context")
                        }
                    }
                }
            }
        }
        
        printf("Taskgroup reduction sum: %f\n", task_sum);
    }
}

/* Complex control flow with OpenMP directives */
void __attribute__((optimize("O0"))) nested_omp_constructs(int mode) {
    switch (mode) {
        case 1: {
            /* Combined parallel for directive */
            #pragma omp parallel for ordered schedule(guided)
            for (int i = 0; i < 100; i++) {
                #pragma omp ordered
                {
                    /* Empty but forces ordered clause representation */
                }
            }
            break;
        }
        case 2: {
            /* Parallel sections with nested for */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp parallel for simd safelen(8)
                    for (int i = 0; i < 50; i++) {
                        /* Simd loop */
                    }
                }
                #pragma omp section
                {
                    /* Use macro to expand to pragma */
                    #define PARALLEL_FOR _Pragma("omp parallel for")
                    PARALLEL_FOR
                    for (int i = 0; i < 50; i++) {
                        /* Another loop */
                    }
                }
            }
            break;
        }
        default:
            /* Taskgroup with depend clause */
            #pragma omp taskgroup task_reduction(*:mode)
            {
                #pragma omp task in_reduction(*:mode)
                {
                    mode *= 2;
                }
            }
    }
}

/* Function that uses OpenMP runtime API with clauses */
void __attribute__((optimize("O0"))) runtime_integration(void) {
    int num_threads = 0;
    
    #pragma omp parallel if(omp_get_max_threads() > 1) \
        default(shared) private(num_threads)
    {
        num_threads = omp_get_num_threads();
        
        #pragma omp master
        {
            printf("Running with %d threads\n", num_threads);
            
            /* Force error with clause reference */
            #pragma omp error severity(message) \
                message("parallel clause active with runtime threads")
        }
        
        /* Barrier to ensure all threads reach this point */
        #pragma omp barrier
        
        #pragma omp for schedule(runtime) nowait
        for (int i = 0; i < num_threads * 10; i++) {
            /* Work distributed based on runtime schedule */
        }
    }
}

int main(void) {
    double *array = (double*)malloc(N * sizeof(double));
    if (!array) return 1;
    
    /* Initialize array in parallel */
    #pragma omp parallel for schedule(static) if(N > 100)
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
    }
    
    /* Process with all clause types */
    process_array(array, N);
    
    /* Test nested constructs */
    for (int mode = 0; mode < 3; mode++) {
        nested_omp_constructs(mode);
    }
    
    /* Runtime integration test */
    runtime_integration();
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) \
        schedule(dynamic, CHUNK_SIZE)
    for (int i = 0; i < N; i++) {
        checksum += array[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    free(array);
    return 0;
}
