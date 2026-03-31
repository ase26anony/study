/* test_openmp_clauses.c - Targeting GCC tree-pretty-print.cc lines 1434-1445 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function attribute to prevent optimization from removing OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(int *data, int n, int *result_sum, int *result_max) {
    int local_sum = 0;
    int local_max = 0;
    
    /* 1. TARGET: OMP_CLAUSE_FOR clause */
    /* Combined parallel for directive - will trigger pretty-print for 'for' clause */
    #pragma omp parallel for schedule(static, 4) collapse(2) reduction(+:local_sum) \
        if(n > 1000) num_threads(omp_get_num_procs() / 2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            data[i] = i * j + omp_get_thread_num();
            local_sum += data[i];
        }
    }
    
    /* 2. TARGET: OMP_CLAUSE_PARALLEL clause */
    /* Explicit parallel region - will trigger pretty-print for 'parallel' clause */
    #pragma omp parallel default(none) shared(data, n, local_max) \
        private(local_sum) if(omp_in_parallel())
    {
        int thread_max = 0;
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < n; i++) {
            if (data[i] > thread_max) thread_max = data[i];
        }
        
        #pragma omp critical
        {
            if (thread_max > local_max) local_max = thread_max;
        }
        
        /* Nested directive with 'for' clause inside 'parallel' region */
        #pragma omp master
        {
            /* Force diagnostic with clause name in message */
            _Pragma("omp error severity(warning) message(\"Processing for clause\")")
        }
    }
    
    /* 3. TARGET: OMP_CLAUSE_SECTIONS clause */
    /* Combined parallel sections - triggers both 'parallel' and 'sections' clauses */
    #pragma omp parallel sections reduction(+:local_sum) reduction(max:local_max) \
        num_threads(4)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                local_sum += data[i] * 2;
            }
            /* Diagnostic with sections clause name */
            #pragma omp error severity(message) message("In sections clause region")
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (data[i] > local_max) local_max = data[i];
            }
        }
        
        /* Third section with nested for */
        #pragma omp section
        {
            #pragma omp parallel for simd schedule(guided)
            for (int i = 0; i < n; i += 2) {
                data[i] = data[i] / 2;
            }
        }
    }
    
    *result_sum = local_sum;
    *result_max = local_max;
}

/* Another function to test taskgroup clause */
void __attribute__((optimize("O0"), noinline))
process_with_taskgroup(int *data, int n, int *final_sum) {
    int task_sum = 0;
    
    /* 4. TARGET: OMP_CLAUSE_TASKGROUP clause */
    /* Taskgroup with task_reduction - triggers 'taskgroup' clause pretty-print */
    #pragma omp parallel master taskloop reduction(+:task_sum) \
        num_tasks(omp_get_num_threads() * 2)
    {
        #pragma omp taskgroup task_reduction(+:task_sum) \
            allocate(omp_default_mem_alloc: task_sum)
        {
            for (int t = 0; t < omp_get_num_threads(); t++) {
                #pragma omp task in_reduction(+:task_sum) \
                    depend(out: data[t]) if(t > 0)
                {
                    int start = t * (n / omp_get_num_threads());
                    int end = (t + 1) * (n / omp_get_num_threads());
                    for (int i = start; i < end && i < n; i++) {
                        task_sum += data[i] % 100;
                    }
                    
                    /* Diagnostic inside task with clause reference */
                    if (t == 0) {
                        #pragma omp error severity(warning) \
                            message("Taskgroup clause active with for computation")
                    }
                }
            }
            
            /* Additional task with nested for */
            #pragma omp task untied mergeable
            {
                #pragma omp parallel for simd simdlen(4) \
                    schedule(nonmonotonic:static)
                for (int i = 0; i < n; i++) {
                    data[i] = data[i] ^ 0x55;
                }
            }
        }
    }
    
    *final_sum = task_sum;
}

/* Complex control flow to ensure pretty-printer sees clauses in various contexts */
int __attribute__((optimize("O0")))
main(int argc, char **argv) {
    const int N = 10000;
    int *data = (int*)malloc(N * sizeof(int));
    int result_sum = 0, result_max = 0, task_sum = 0;
    
    /* Initialize data with OpenMP */
    #pragma omp parallel for simd schedule(static) \
        if(N > 100) aligned(data:64)
    for (int i = 0; i < N; i++) {
        data[i] = i % 256;
    }
    
    /* Switch statement with OpenMP in different cases */
    for (int iteration = 0; iteration < 3; iteration++) {
        switch (iteration) {
            case 0:
                /* Directives in switch case 0 */
                #pragma omp parallel for ordered(1) \
                    linear(i:1) lastprivate(result_max)
                for (int i = 0; i < N/10; i++) {
                    #pragma omp ordered depend(source)
                    data[i] += iteration;
                    #pragma omp ordered depend(sink)
                    if (data[i] > result_max) result_max = data[i];
                }
                break;
                
            case 1:
                /* Mixed directive with sections */
                #pragma omp parallel sections private(result_sum) \
                    copyin(result_max)
                {
                    #pragma omp section
                    {
                        #pragma omp taskloop grainsize(64) \
                            nogroup reduction(+:result_sum)
                        for (int i = 0; i < N; i += 2) {
                            result_sum += data[i];
                        }
                    }
                    
                    #pragma omp section
                    {
                        #pragma omp distribute parallel for simd \
                            dist_schedule(static, 256)
                        for (int i = 1; i < N; i += 2) {
                            data[i] = data[i] * 2;
                        }
                    }
                }
                break;
                
            case 2:
                /* Taskgroup in final iteration */
                process_with_taskgroup(data, N, &task_sum);
                break;
        }
    }
    
    /* Final processing with all target clauses */
    process_with_openmp(data, N, &result_sum, &result_max);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    #pragma omp parallel for reduction(+:checksum) \
        schedule(runtime)
    for (int i = 0; i < N; i++) {
        checksum += (unsigned long long)data[i];
    }
    checksum += result_sum + result_max + task_sum;
    
    printf("Checksum: %llu\n", checksum);
    
    /* Force compiler to generate diagnostics with clause names */
    #ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic warning "-Wopenmp-clauses"
    
    /* These will trigger pretty-printing of clause names during compilation */
    #pragma omp parallel for if(0)
    for (int i = 0; i < 1; i++) { /* empty */ }
    
    #pragma omp parallel sections if(0)
    {
        #pragma omp section
        { /* empty */ }
    }
    
    #pragma omp taskgroup if(0)
    { /* empty */ }
    
    #pragma GCC diagnostic pop
    #endif
    
    free(data);
    return 0;
}
