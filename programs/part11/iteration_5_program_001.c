/* test_omp_clauses.c - Targeting uncovered pretty-print cases in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n) {
    double sum = 0.0;
    double max_val = -INFINITY;
    int i, j;
    
    /* 1. Use 'for' clause in a combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(i, j) shared(data) reduction(+:sum) if(n > 1000)
    for (i = 0; i < n; i++) {
        for (j = 0; j < NUM_ITERATIONS; j++) {
            data[i] = sin(i * 0.01 + j * 0.001);
            sum += data[i];
        }
    }
    
    /* Force diagnostic with 'for' clause name in message */
    #pragma omp error severity(warning) message("Processing with 'for' clause completed")
    
    /* 2. Use 'parallel' clause in combined directive */
    #pragma omp parallel sections private(i) shared(data, max_val) \
        num_threads(omp_get_max_threads() / 2) if(omp_in_parallel())
    {
        /* First section */
        #pragma omp section
        {
            double local_max = -INFINITY;
            #pragma omp parallel for reduction(max:local_max)
            for (i = 0; i < n/2; i++) {
                if (data[i] > local_max) local_max = data[i];
            }
            #pragma omp atomic
            max_val = (local_max > max_val) ? local_max : max_val;
        }
        
        /* Second section */
        #pragma omp section
        {
            double local_sum = 0.0;
            #pragma omp parallel for reduction(+:local_sum) schedule(guided)
            for (i = n/2; i < n; i++) {
                local_sum += cos(data[i]);
            }
            #pragma omp atomic
            sum += local_sum;
        }
    }
    
    /* 3. Use 'sections' clause explicitly */
    #pragma omp parallel if(n > 500)
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                /* Process first quarter */
                for (i = 0; i < n/4; i++) {
                    data[i] = sqrt(fabs(data[i]));
                }
            }
            
            #pragma omp section
            {
                /* Process second quarter */
                for (i = n/4; i < n/2; i++) {
                    data[i] = log(fabs(data[i]) + 1.0);
                }
            }
            
            #pragma omp section
            {
                /* Process third quarter */
                for (i = n/2; i < 3*n/4; i++) {
                    data[i] = data[i] * data[i];
                }
            }
        }
        
        #pragma omp barrier
        
        /* 4. Use 'taskgroup' clause with task_reduction */
        #pragma omp single
        {
            double task_sum = 0.0;
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double partial = 0.0;
                        int start = i * (n/10);
                        int end = (i + 1) * (n/10);
                        for (int k = start; k < end; k++) {
                            partial += sin(data[k]);
                        }
                        task_sum += partial;
                        
                        /* Nested task with error directive */
                        #pragma omp task if(0)
                        {
                            /* This will trigger pretty-printing of clauses */
                            _Pragma("omp error severity(message) message(\"Inside task with taskgroup clause\")")
                        }
                    }
                }
            }
            sum += task_sum;
        }
    }
    
    /* Complex macro expansion to force pretty-printing */
    #define EMIT_CLause_DIAG(clause) _Pragma("omp error severity(warning) message(\"Clause: " #clause "\")")
    
    /* Emit diagnostics for each target clause */
    EMIT_CLause_DIAG(for);
    EMIT_CLause_DIAG(parallel);
    EMIT_CLause_DIAG(sections);
    EMIT_CLause_DIAG(taskgroup);
    
    /* Final computation using results */
    double checksum = sum + max_val;
    printf("Intermediate checksum: %f\n", checksum);
}

/* Another function with different OpenMP patterns */
void __attribute__((optimize("O0"))) nested_omp_constructs(int depth) {
    if (depth <= 0) return;
    
    switch (depth % 4) {
        case 0:
            /* Combined parallel for with multiple clauses */
            #pragma omp parallel for ordered schedule(dynamic, 2) \
                if(depth > 2) num_threads(4)
            for (int i = 0; i < 10; i++) {
                #pragma omp ordered
                {
                    printf("Ordered iteration %d at depth %d\n", i, depth);
                }
            }
            break;
            
        case 1:
            /* Parallel sections with nested tasks */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        nested_omp_constructs(depth - 1);
                    }
                }
                
                #pragma omp section
                {
                    #pragma omp task
                    nested_omp_constructs(depth - 2);
                }
            }
            break;
            
        case 2:
            /* Task with taskgroup and reduction */
            {
                double reduction_var = 0.0;
                #pragma omp taskgroup task_reduction(+:reduction_var)
                {
                    for (int i = 0; i < 5; i++) {
                        #pragma omp task in_reduction(+:reduction_var)
                        {
                            reduction_var += i * 0.1;
                        }
                    }
                }
                printf("Taskgroup reduction result: %f\n", reduction_var);
            }
            break;
            
        case 3:
            /* Distribute parallel for simd with collapse */
            #pragma omp target teams distribute parallel for simd \
                map(tofrom:depth) collapse(2) if(omp_get_num_teams() > 0)
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    depth += i * j;
                }
            }
            break;
    }
}

int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize OpenMP */
    omp_set_nested(1);
    omp_set_max_active_levels(4);
    
    printf("Starting OpenMP computation with clause coverage...\n");
    
    /* Process data with various OpenMP clauses */
    process_with_openmp(data, ARRAY_SIZE);
    
    /* Use nested constructs to trigger more pretty-printing */
    nested_omp_constructs(3);
    
    /* Final computation to prevent dead code elimination */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) schedule(static)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += data[i];
    }
    
    printf("Final checksum: %.6f\n", final_sum);
    
    /* Trigger one more diagnostic with combined clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Force pretty-printing with error directive */
                    #pragma omp error severity(message) \
                        message("Final: for parallel sections taskgroup")
                }
            }
        }
    }
    
    free(data);
    return 0;
}
