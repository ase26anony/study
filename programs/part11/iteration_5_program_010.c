/* test_openmp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define CHUNK_SIZE 64

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(double *data, int n, double *result_sum, double *result_max) {
    double local_sum = 0.0;
    double local_max = -INFINITY;
    
    /* 1. Use 'for' clause with arguments - triggers OMP_CLAUSE_FOR */
    #pragma omp distribute parallel for simd \
        schedule(static, CHUNK_SIZE) collapse(2) \
        reduction(+:local_sum) reduction(max:local_max) \
        if(n > 100)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 2; j++) {
            data[i] = sin(i * 0.01) * cos(j * 0.01);
            local_sum += data[i];
            if (data[i] > local_max) local_max = data[i];
        }
    }
    
    /* 2. Combined 'parallel' and 'sections' - triggers both clauses */
    #pragma omp parallel sections \
        private(local_sum, local_max) \
        num_threads(4)
    {
        #pragma omp section
        {
            /* First section computes sum of squares */
            double sum_sq = 0.0;
            #pragma omp parallel for reduction(+:sum_sq) \
                schedule(dynamic, 16)
            for (int i = 0; i < n/2; i++) {
                sum_sq += data[i] * data[i];
            }
            result_sum[0] = sum_sq;
            
            /* Force diagnostic with clause name in message */
            #pragma omp error severity(warning) \
                message("OpenMP clause 'for' used in section 1")
        }
        
        #pragma omp section
        {
            /* Second section finds maximum absolute value */
            double max_abs = 0.0;
            #pragma omp parallel for reduction(max:max_abs) \
                schedule(guided)
            for (int i = n/2; i < n; i++) {
                double abs_val = fabs(data[i]);
                if (abs_val > max_abs) max_abs = abs_val;
            }
            result_max[0] = max_abs;
            
            /* Alternative way to trigger pretty-printing */
            _Pragma("omp error severity(message) message(\"sections clause active\")")
        }
    }
    
    /* 3. 'taskgroup' clause with task_reduction */
    #pragma omp parallel
    #pragma omp single
    {
        double task_sum = 0.0;
        
        #pragma omp taskgroup task_reduction(+:task_sum) \
            allocate(task_sum: omp_pteam_mem_alloc)
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task in_reduction(+:task_sum) \
                    firstprivate(i)
                {
                    double partial = 0.0;
                    int start = i * (n/10);
                    int end = (i + 1) * (n/10);
                    for (int j = start; j < end; j++) {
                        partial += data[j] * log(fabs(data[j]) + 1.0);
                    }
                    task_sum += partial;
                    
                    /* Nested task with error directive */
                    #pragma omp task
                    {
                        #pragma omp error severity(warning) \
                            message("Inside taskgroup, clause: taskgroup")
                    }
                }
            }
        }
        
        result_sum[1] = task_sum;
    }
}

/* Complex control flow with OpenMP directives */
void __attribute__((optimize("O0")))
nested_function_with_omp(int mode) {
    static int counter = 0;
    
    switch (mode) {
        case 1: {
            /* 'parallel' clause in switch case */
            #pragma omp parallel for ordered \
                if(omp_get_num_procs() > 2)
            for (int i = 0; i < 100; i++) {
                #pragma omp ordered
                {
                    counter++;
                }
            }
            break;
        }
        case 2: {
            /* 'sections' clause with multiple section blocks */
            #pragma omp parallel sections \
                default(none) shared(counter)
            {
                #pragma omp section
                {
                    #pragma omp atomic
                    counter += 10;
                }
                #pragma omp section
                {
                    #pragma omp atomic
                    counter -= 5;
                }
                #pragma omp section
                {
                    #pragma omp atomic
                    counter *= 2;
                }
            }
            break;
        }
        case 3: {
            /* Complex nested directives */
            #pragma omp target teams distribute parallel for simd \
                map(tofrom:counter) \
                device(omp_get_default_device()) \
                thread_limit(omp_get_max_threads())
            for (int i = 0; i < 50; i++) {
                counter += i % 3;
            }
            break;
        }
    }
}

/* Macro that expands to include clause names */
#define DIAGNOSTIC_MSG(clause) \
    _Pragma("omp error severity(message) message(\"Clause: " #clause "\")")

void trigger_pretty_print_diagnostics() {
    /* Force pretty-printing of clause names through diagnostics */
    DIAGNOSTIC_MSG(for);
    DIAGNOSTIC_MSG(parallel);
    DIAGNOSTIC_MSG(sections);
    DIAGNOSTIC_MSG(taskgroup);
    
    /* Combined directive that will be pretty-printed */
    #pragma omp parallel master taskloop simd \
        grainsize(8) nogroup \
        if(omp_in_parallel())
    for (int i = 0; i < 20; i++) {
        volatile int dummy = i * 2;
        (void)dummy;
    }
}

int main() {
    double *data = (double*)malloc(N * sizeof(double));
    double result_sum[2] = {0.0, 0.0};
    double result_max[2] = {0.0, 0.0};
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    omp_set_dynamic(0);
    
    /* 1. Initial parallel for with schedule(dynamic) */
    #pragma omp parallel for schedule(dynamic) \
        default(none) shared(data) \
        if(N > 500)
    for (int i = 0; i < N; i++) {
        data[i] = (double)i / N;
    }
    
    /* 2. Process with all target clauses */
    process_with_openmp(data, N, result_sum, result_max);
    
    /* 3. Trigger diagnostics for pretty-printing */
    trigger_pretty_print_diagnostics();
    
    /* 4. Nested function calls with different modes */
    for (int mode = 1; mode <= 3; mode++) {
        nested_function_with_omp(mode);
    }
    
    /* 5. Final computation with combined clauses */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) \
        schedule(runtime) \
        proc_bind(spread)
    for (int i = 0; i < N; i++) {
        checksum += data[i] + result_sum[0] + result_sum[1] + 
                   result_max[0] + result_max[1];
    }
    
    /* 6. Taskgroup with explicit task_reduction */
    double final_reduction = 0.0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup task_reduction(+:final_reduction)
        {
            #pragma omp task in_reduction(+:final_reduction)
            { final_reduction += checksum; }
            
            #pragma omp task in_reduction(+:final_reduction)
            { final_reduction += omp_get_wtime(); }
        }
    }
    
    printf("Final checksum: %f\n", final_reduction);
    printf("Results: sum1=%f, sum2=%f, max1=%f, max2=%f\n",
           result_sum[0], result_sum[1], result_max[0], result_max[1]);
    
    free(data);
    return 0;
}
