/* test_openmp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define M 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n) {
    double sum = 0.0;
    double max_val = -INFINITY;
    
    /* 1. Use 'for' clause with arguments - triggers OMP_CLAUSE_FOR */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(sum) shared(data) if(n > 100)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            data[i * M + j] = sin(i * 0.01) * cos(j * 0.01);
        }
    }
    
    /* 2. Use 'parallel' clause in combined directive - triggers OMP_CLAUSE_PARALLEL */
    #pragma omp parallel reduction(+:sum) reduction(max:max_val) \
        num_threads(4) if(omp_get_max_threads() > 1)
    {
        /* Runtime call inside parallel region */
        int tid = omp_get_thread_num();
        int nthreads = omp_get_num_threads();
        
        /* Nested loop with for clause */
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < n; i++) {
            sum += data[i * M + tid];
        }
        
        /* Force diagnostic with clause name in message */
        #pragma omp error severity(warning) message("for clause used here")
    }
    
    /* 3. Use 'sections' clause - triggers OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(max_val) shared(data, sum)
    {
        #pragma omp section
        {
            max_val = data[0];
            #pragma omp parallel for reduction(max:max_val)
            for (int i = 0; i < n * M; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
        }
        
        #pragma omp section
        {
            double local_sum = 0.0;
            #pragma omp parallel for reduction(+:local_sum) schedule(guided)
            for (int i = 0; i < n * M; i++) {
                local_sum += fabs(data[i]);
            }
            sum += local_sum;
        }
    }
    
    /* 4. Use 'taskgroup' clause - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            double task_sum = 0.0;
            
            /* Taskgroup with reduction clause */
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double partial = 0.0;
                        for (int j = 0; j < n/10; j++) {
                            int idx = i * (n/10) + j;
                            if (idx < n * M) {
                                partial += data[idx] * data[idx];
                            }
                        }
                        task_sum += partial;
                        
                        /* Nested task with error directive */
                        #pragma omp task
                        {
                            /* Use _Pragma to create complex pattern */
                            _Pragma("omp error severity(message) message(\"parallel clause in nested task\")")
                        }
                    }
                }
            }
            sum += sqrt(task_sum);
        }
    }
    
    printf("Intermediate sum: %f, max: %f\n", sum, max_val);
}

/* Another function with mixed control flow */
void __attribute__((optimize("O0"))) complex_flow_openmp(double *data, int n) {
    int i;
    
    /* OpenMP inside switch statement */
    switch (n % 3) {
        case 0:
            #pragma omp parallel for schedule(static)
            for (i = 0; i < n; i++) {
                data[i] *= 2.0;
            }
            break;
            
        case 1:
            #pragma omp parallel sections
            {
                #pragma omp section
                { /* Empty section but still triggers sections clause */ }
                #pragma omp section
                { data[0] = 0.0; }
            }
            break;
            
        case 2:
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Macro with _Pragma for parallel clause */
                    #define EMIT_PARALLEL_MSG _Pragma("omp error message(\"parallel\")")
                    EMIT_PARALLEL_MSG;
                }
            }
            break;
    }
    
    /* Nested loops with OpenMP */
    for (int outer = 0; outer < 2; outer++) {
        #pragma omp parallel if(outer == 1)
        {
            #pragma omp for nowait
            for (int i = 0; i < n; i++) {
                data[i] += outer * 0.5;
            }
        }
    }
}

int main() {
    double *data = (double*)malloc(N * M * sizeof(double));
    if (!data) return 1;
    
    /* Initialize with parallel for - uses 'for' clause */
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < N * M; i++) {
        data[i] = (double)i / (N * M);
    }
    
    /* Process with various OpenMP constructs */
    process_with_openmp(data, N);
    complex_flow_openmp(data, N);
    
    /* Final computation to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) schedule(static, 16)
    for (int i = 0; i < N * M; i++) {
        checksum += data[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Additional taskgroup usage for coverage */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Force diagnostic for sections clause */
                    #pragma omp error severity(warning) message("sections clause")
                }
                #pragma omp task
                {
                    /* Force diagnostic for taskgroup clause */
                    #pragma omp error severity(warning) message("taskgroup clause")
                }
            }
        }
    }
    
    free(data);
    return 0;
}
