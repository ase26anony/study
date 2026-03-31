/* test_openmp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_TASKS 10

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n, double *result_sum, double *result_max) {
    double local_sum = 0.0;
    double local_max = -INFINITY;
    
    /* 1. Use 'for' clause with arguments - triggers OMP_CLAUSE_FOR */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        reduction(+:local_sum) reduction(max:local_max) if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            double val = data[i] * (j + 1);
            local_sum += val;
            if (val > local_max) local_max = val;
        }
    }
    
    /* 2. Use 'parallel' clause - triggers OMP_CLAUSE_PARALLEL */
    #pragma omp parallel if(n > 500) num_threads(4) default(shared) \
        private(local_sum, local_max) firstprivate(n)
    {
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        
        /* Nested directive combining 'parallel' and 'sections' */
        #pragma omp parallel sections if(tid == 0) num_threads(2)
        {
            /* 3. Use 'sections' clause - triggers OMP_CLAUSE_SECTIONS */
            #pragma omp section
            {
                double section_sum = 0.0;
                for (int i = tid; i < n; i += nthreads) {
                    section_sum += sin(data[i]);
                }
                #pragma omp atomic
                local_sum += section_sum;
            }
            
            #pragma omp section
            {
                double section_max = -INFINITY;
                for (int i = tid; i < n; i += nthreads) {
                    double val = cos(data[i]);
                    if (val > section_max) section_max = val;
                }
                #pragma omp atomic
                if (section_max > local_max) local_max = section_max;
            }
        }
    }
    
    *result_sum = local_sum;
    *result_max = local_max;
}

/* Complex macro using _Pragma to create nested patterns */
#define CREATE_TASKGROUP_WITH_REDUCTION(var, op) \
    _Pragma("omp taskgroup task_reduction(" #op ":" #var ")") \
    { \
        for (int _i = 0; _i < NUM_TASKS; _i++) { \
            _Pragma("omp task in_reduction(" #op ":" #var ")") \
            { \
                var op##= _i * 0.1; \
                /* Force diagnostic with clause name */ \
                _Pragma("omp error message(\"Processing task with for clause\")") \
            } \
        } \
    }

void __attribute__((optimize("O0"))) task_based_computation(double *data, int n, double *final_result) {
    double task_sum = 0.0;
    
    /* 4. Use 'taskgroup' clause - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Use the macro to create taskgroup with reduction */
            CREATE_TASKGROUP_WITH_REDUCTION(task_sum, +);
            
            /* Alternative direct usage */
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < n; i += n / NUM_TASKS) {
                    #pragma omp task in_reduction(+:task_sum) \
                        if(i < n/2) final(i % 2 == 0)
                    {
                        double partial = 0.0;
                        int end = (i + n/NUM_TASKS) < n ? (i + n/NUM_TASKS) : n;
                        for (int j = i; j < end; j++) {
                            partial += data[j] * data[j];
                        }
                        task_sum += partial;
                        
                        /* Trigger diagnostic that includes clause names */
                        #ifdef __GNUC__
                        #pragma omp error severity(warning) message("Task contains for clause computation")
                        #endif
                    }
                }
            }
        }
    }
    
    *final_result = task_sum;
}

/* Function with switch statement containing OpenMP directives */
void __attribute__((optimize("O0"))) switch_with_openmp(int mode, double *data, int n, double *output) {
    switch (mode) {
        case 0:
            /* Combined parallel for directive */
            #pragma omp parallel for schedule(dynamic) if(n > 100) \
                ordered proc_bind(spread)
            for (int i = 0; i < n; i++) {
                #pragma omp ordered
                data[i] = sqrt(fabs(data[i]));
            }
            *output = data[0];
            break;
            
        case 1:
            /* Nested parallel sections */
            #pragma omp parallel
            {
                #pragma omp sections nowait
                {
                    #pragma omp section
                    {
                        #pragma omp parallel for simd simdlen(4) \
                            linear(i:1) if(omp_get_num_threads() > 1)
                        for (int i = 0; i < n/2; i++) {
                            data[i] = exp(data[i]);
                        }
                    }
                    
                    #pragma omp section
                    {
                        #pragma omp taskloop grainsize(64) \
                            nogroup if(n > 1000)
                        for (int i = n/2; i < n; i++) {
                            data[i] = log(fabs(data[i]) + 1.0);
                        }
                    }
                }
            }
            *output = data[n-1];
            break;
            
        default:
            /* Taskgroup with error directive */
            #pragma omp taskgroup task_reduction(max:*output)
            {
                #pragma omp task in_reduction(max:*output)
                {
                    double max_val = -INFINITY;
                    for (int i = 0; i < n; i++) {
                        if (data[i] > max_val) max_val = data[i];
                    }
                    *output = max_val;
                    
                    /* Force pretty-printing of clause names */
                    #pragma omp error message("Found for clause in default case")
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
    
    /* Initialize array with parallel for - uses 'for' clause */
    #pragma omp parallel for schedule(dynamic) if(ARRAY_SIZE > 1000) \
        shared(data) default(none)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i % 100) * 0.01 + sin(i * 0.1);
    }
    
    double sum_result, max_result;
    process_with_openmp(data, ARRAY_SIZE, &sum_result, &max_result);
    
    double task_result;
    task_based_computation(data, ARRAY_SIZE, &task_result);
    
    double switch_result;
    switch_with_openmp(0, data, ARRAY_SIZE / 2, &switch_result);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = sum_result + max_result + task_result + switch_result;
    printf("Computed checksum: %.6f\n", checksum);
    
    /* Additional complex pattern in main */
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp taskgroup task_reduction(+:checksum)
            {
                #pragma omp task in_reduction(+:checksum)
                {
                    for (int i = 0; i < 100; i++) {
                        checksum += i * 0.001;
                    }
                }
                
                #pragma omp task in_reduction(+:checksum)
                {
                    #pragma omp parallel sections
                    {
                        #pragma omp section
                        {
                            #pragma omp error message("Section 1 with for clause")
                        }
                        #pragma omp section
                        {
                            #pragma omp error message("Section 2 with parallel clause")
                        }
                    }
                }
            }
        }
    }
    
    printf("Final checksum: %.6f\n", checksum);
    
    free(data);
    return 0;
}
