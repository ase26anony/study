/* test_openmp_clauses.c
 * Targets uncovered lines in tree-pretty-print.cc:
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(double *data, int size, double *result_sum, double *result_max) {
    double local_sum = 0.0;
    double local_max = -__DBL_MAX__;
    
    /* 1. PARALLEL and FOR clauses - combined directive */
    #pragma omp parallel for schedule(dynamic) reduction(+:local_sum) \
        if(size > 1000) num_threads(omp_get_max_threads())
    for (int i = 0; i < size; i++) {
        data[i] = (double)(i * 3.14159) / size;
        local_sum += data[i];
    }
    
    /* 2. PARALLEL and SECTIONS clauses - combined directive */
    #pragma omp parallel sections private(local_max) shared(data, size)
    {
        #pragma omp section
        {
            double section_max = -__DBL_MAX__;
            for (int i = 0; i < size/2; i++) {
                if (data[i] > section_max) section_max = data[i];
            }
            #pragma omp atomic
            if (section_max > local_max) local_max = section_max;
        }
        
        #pragma omp section
        {
            double section_max = -__DBL_MAX__;
            for (int i = size/2; i < size; i++) {
                if (data[i] > section_max) section_max = data[i];
            }
            #pragma omp atomic
            if (section_max > local_max) local_max = section_max;
        }
    }
    
    /* 3. Complex FOR clause with arguments in combined directive */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom:data[0:size]) \
        if(omp_get_num_teams() > 0)
    for (int i = 0; i < size/10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < size) {
                data[idx] = data[idx] * 2.0;
            }
        }
    }
    
    /* 4. TASKGROUP clause with task_reduction */
    #pragma omp taskgroup task_reduction(+:local_sum)
    {
        /* Nested function to create complex scope */
        void process_chunk(int start, int end) {
            #pragma omp task firstprivate(start, end) shared(data) \
                in_reduction(+:local_sum)
            {
                double chunk_sum = 0.0;
                for (int i = start; i < end; i++) {
                    chunk_sum += data[i];
                }
                #pragma omp atomic
                local_sum += chunk_sum;
                
                /* Force diagnostic with clause name in message */
                #pragma omp error message("Processing with FOR clause simulation")
            }
        }
        
        int chunk_size = size / 4;
        for (int c = 0; c < 4; c++) {
            int start = c * chunk_size;
            int end = (c == 3) ? size : (c + 1) * chunk_size;
            process_chunk(start, end);
        }
        
        /* Additional task with error directive containing clause name */
        #pragma omp task
        {
            /* This should trigger pretty-printing of clause names */
            _Pragma("omp error severity(warning) message(\"FOR clause in task\")")
            
            /* Use runtime API in clause context */
            if (omp_in_parallel()) {
                local_max *= 1.01;
            }
        }
    }
    
    *result_sum = local_sum;
    *result_max = local_max;
}

/* Another function with mixed control flow and OpenMP */
void __attribute__((optimize("O0")))
complex_control_flow(int n) {
    double *temp = (double*)malloc(n * sizeof(double));
    if (!temp) return;
    
    /* Switch statement with OpenMP directives */
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0:
                /* PARALLEL clause in standalone directive */
                #pragma omp parallel if(n > 1000)
                {
                    int tid = omp_get_thread_num();
                    #pragma omp for schedule(guided)
                    for (int i = 0; i < n; i++) {
                        temp[i] = (double)tid / (omp_get_num_threads() + 1);
                    }
                }
                break;
                
            case 1:
                /* SECTIONS clause in combined directive */
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        #pragma omp taskloop
                        for (int i = 0; i < n/2; i++) {
                            temp[i] *= 0.5;
                        }
                    }
                    #pragma omp section
                    {
                        #pragma omp simd
                        for (int i = n/2; i < n; i++) {
                            temp[i] *= 2.0;
                        }
                    }
                }
                break;
                
            case 2:
                /* TASKGROUP with nested tasks */
                #pragma omp taskgroup
                {
                    #pragma omp task untied mergeable
                    {
                        /* Error directive with multiple clause names */
                        #pragma omp error severity(message) \
                            message("TASKGROUP clause with PARALLEL and SECTIONS simulation")
                    }
                    
                    #pragma omp task
                    {
                        /* Use _Pragma for macro expansion */
                        #define OMP_ERROR_MSG(msg) _Pragma("omp error message(msg)")
                        OMP_ERROR_MSG("FOR clause via macro")
                    }
                }
                break;
        }
    }
    
    free(temp);
}

int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double total_sum = 0.0;
    double global_max = -__DBL_MAX__;
    
    /* Process multiple times to ensure coverage */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        double iter_sum, iter_max;
        
        /* Call function with various OpenMP clauses */
        process_with_openmp(data, ARRAY_SIZE, &iter_sum, &iter_max);
        
        total_sum += iter_sum;
        if (iter_max > global_max) global_max = iter_max;
        
        /* Call function with complex control flow */
        complex_control_flow(ARRAY_SIZE / 10);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = total_sum + global_max;
    printf("Checksum: %f\n", checksum);
    printf("Array[0] = %f, Array[%d] = %f\n", 
           data[0], ARRAY_SIZE-1, data[ARRAY_SIZE-1]);
    
    free(data);
    return 0;
}
