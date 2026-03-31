/* test_openmp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define M 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n, double *results) {
    double sum = 0.0;
    double max_val = -INFINITY;
    
    /* 1. Use 'for' clause in combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(sum) shared(data, results) if(n > 100)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            data[i * M + j] = sin(i * 0.01) * cos(j * 0.01);
        }
    }
    
    /* 2. Use 'parallel' clause in combined directive */
    #pragma omp parallel reduction(+:sum) reduction(max:max_val) \
        num_threads(omp_get_max_threads())
    {
        int tid = omp_get_thread_num();
        
        /* 3. Use 'sections' clause within parallel region */
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                /* First section computes sum */
                double local_sum = 0.0;
                for (int i = tid * (n/omp_get_num_threads()); 
                     i < (tid+1) * (n/omp_get_num_threads()); i++) {
                    for (int j = 0; j < M; j++) {
                        local_sum += data[i * M + j];
                    }
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp section  
            {
                /* Second section finds maximum */
                double local_max = -INFINITY;
                for (int i = tid * (n/omp_get_num_threads()); 
                     i < (tid+1) * (n/omp_get_num_threads()); i++) {
                    for (int j = 0; j < M; j++) {
                        if (data[i * M + j] > local_max) {
                            local_max = data[i * M + j];
                        }
                    }
                }
                #pragma omp critical
                {
                    if (local_max > max_val) max_val = local_max;
                }
            }
        } /* end sections */
    } /* end parallel */
    
    results[0] = sum;
    results[1] = max_val;
    
    /* 4. Use 'taskgroup' clause with task_reduction */
    double task_sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double partial = 0.0;
                        for (int j = 0; j < n/10; j++) {
                            int idx = i * (n/10) + j;
                            if (idx < n) {
                                for (int k = 0; k < M; k++) {
                                    partial += data[idx * M + k] * 0.5;
                                }
                            }
                        }
                        task_sum += partial;
                        
                        /* Force diagnostic with clause name in message */
                        if (i == 5) {
                            /* This should trigger pretty-printing of 'for' clause */
                            #pragma omp error message("Clause 'for' encountered in task") severity(warning)
                        }
                    }
                }
            }
        }
    }
    
    results[2] = task_sum;
}

/* Complex control flow with nested OpenMP directives */
void __attribute__((optimize("O0"))) complex_flow_with_clauses(int mode) {
    switch(mode) {
        case 1: {
            /* Nested parallel for with runtime schedule */
            #pragma omp parallel for schedule(runtime) if(omp_in_parallel())
            for (int i = 0; i < 100; i++) {
                /* Use _Pragma to create complex pattern */
                _Pragma("omp atomic")
                mode += i;
            }
            break;
        }
        case 2: {
            /* Parallel sections with nested tasks */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            /* Macro expansion with clause */
                            #define TASK_REDUCE _Pragma("omp taskgroup task_reduction(+:mode)")
                            TASK_REDUCE
                            mode *= 2;
                        }
                    }
                }
                #pragma omp section
                {
                    /* Empty section but still triggers sections clause */
                }
            }
            break;
        }
    }
}

/* Function using OpenMP runtime API with clause reflection */
double __attribute__((optimize("O0"))) compute_with_runtime_reflection(double *arr, int size) {
    double result = 0.0;
    int num_threads = 0;
    
    /* Conditional parallel region that retains clause info */
    #pragma omp parallel if(size > 500) reduction(+:result) copyin(num_threads)
    {
        num_threads = omp_get_num_threads();
        int chunk = size / num_threads;
        int tid = omp_get_thread_num();
        int start = tid * chunk;
        int end = (tid == num_threads - 1) ? size : (tid + 1) * chunk;
        
        /* Distribute parallel for simd with for clause */
        #pragma omp distribute parallel for simd reduction(+:result) \
            schedule(static) if(omp_get_num_threads() > 1)
        for (int i = start; i < end; i++) {
            result += arr[i] * arr[i];
        }
    }
    
    /* Additional diagnostic to force pretty-printing */
    if (result > 1000.0) {
        /* This should trigger pretty-printing of clauses */
        #pragma omp error message("Result too large in compute_with_runtime_reflection") severity(message)
    }
    
    return result / size;
}

int main() {
    double *data = (double*)malloc(N * M * sizeof(double));
    double results[3];
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    
    /* Process with all target clauses */
    process_with_openmp(data, N, results);
    
    /* Use complex flow */
    complex_flow_with_clauses(1);
    complex_flow_with_clauses(2);
    
    /* Compute with runtime reflection */
    double reflection_result = compute_with_runtime_reflection(data, N * M);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = results[0] + results[1] + results[2] + reflection_result;
    
    printf("Checksum: %f\n", checksum);
    printf("Results: sum=%f, max=%f, task_sum=%f, reflection=%f\n", 
           results[0], results[1], results[2], reflection_result);
    
    /* Additional OpenMP constructs in main for coverage */
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Final diagnostic with clause names */
                    #pragma omp error message("Final: clauses for, parallel, sections, taskgroup processed") severity(warning)
                }
            }
        }
    }
    
    free(data);
    return 0;
}
