/* test_openmp_clauses.c
 * Targets uncovered lines in tree-pretty-print.cc:
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function attribute to prevent optimization from removing OpenMP constructs */
void __attribute__((optimize("O0"), noinline))
process_with_openmp(int *data, int n, int *results) {
    int i, j;
    int local_sum = 0;
    int local_max = 0;
    
    /* 1. OMP_CLAUSE_FOR - in combined directive with schedule and collapse */
    #pragma omp distribute parallel for simd \
        schedule(static, 4) collapse(2) \
        private(i, j) shared(data) reduction(+:local_sum)
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            data[i] = i * 10 + j;
            local_sum += data[i];
        }
    }
    results[0] = local_sum;
    
    /* 2. OMP_CLAUSE_PARALLEL - standalone and combined */
    #pragma omp parallel default(none) shared(data, n, results, local_max) \
        num_threads(4) if(n > 1000)
    {
        int tid = omp_get_thread_num();
        #pragma omp single
        {
            /* Force pretty-printing of 'parallel' clause */
            _Pragma("omp error severity(warning) message(\"Testing parallel clause pretty-print\")")
        }
        
        #pragma omp for reduction(max:local_max)
        for (i = 0; i < n; i++) {
            if (data[i] > local_max) local_max = data[i];
        }
    }
    results[1] = local_max;
    
    /* 3. OMP_CLAUSE_SECTIONS - with multiple section blocks */
    int section_sum = 0, section_product = 1;
    #pragma omp parallel sections private(i) \
        reduction(+:section_sum) reduction(*:section_product)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                section_sum += data[i];
            }
            /* Nested directive to trigger 'sections' pretty-print */
            #pragma omp error severity(message) \
                message("Processing in sections clause region")
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                if (data[i] != 0) section_product *= data[i];
            }
        }
        
        #pragma omp section
        {
            /* Empty section to test minimal case */
        }
    }
    results[2] = section_sum;
    results[3] = section_product % 1000000; /* Prevent overflow */
    
    /* 4. OMP_CLAUSE_TASKGROUP - with task_reduction clause */
    int task_reduction_sum = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_reduction_sum)
            {
                for (i = 0; i < 8; i++) {
                    #pragma omp task in_reduction(+:task_reduction_sum) \
                        firstprivate(i) shared(data, n)
                    {
                        int chunk = n / 8;
                        int start = i * chunk;
                        int end = (i == 7) ? n : start + chunk;
                        for (j = start; j < end; j++) {
                            task_reduction_sum += data[j] % 100;
                        }
                        
                        /* Trigger 'taskgroup' clause pretty-print */
                        if (i == 0) {
                            #pragma omp error severity(warning) \
                                message("Taskgroup clause in use")
                        }
                    }
                }
            }
        }
    }
    results[4] = task_reduction_sum;
}

/* Complex control flow to ensure pretty-printer sees various contexts */
void __attribute__((optimize("O0")))
nested_function_with_omp(int *data, int n) {
    int mode = n % 3;
    
    switch (mode) {
        case 0:
            /* Combined parallel for directive */
            #pragma omp parallel for ordered schedule(dynamic, 2)
            for (int i = 0; i < n; i++) {
                #pragma omp ordered
                {
                    data[i] = data[i] * 2;
                }
            }
            break;
            
        case 1:
            /* Nested sections */
            #pragma omp parallel
            {
                #pragma omp sections nowait
                {
                    #pragma omp section
                    { data[0] = 1; }
                    #pragma omp section  
                    { data[1] = 2; }
                }
                
                /* Another directive in same parallel region */
                #pragma omp for
                for (int i = 2; i < n; i++) {
                    data[i] = data[i-1] + data[i-2];
                }
            }
            break;
            
        case 2:
            /* Task with taskgroup */
            #pragma omp parallel
            {
                #pragma omp single
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            /* Force 'for' clause pretty-print via error directive */
                            #pragma omp error severity(message) \
                                message("Testing for clause pretty-print in task context")
                            data[0] = omp_get_num_threads();
                        }
                    }
                }
            }
            break;
    }
}

/* Main function with OpenMP runtime calls */
int main() {
    const int N = 10000;
    int *data = (int*)malloc(N * sizeof(int));
    int results[5] = {0};
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    omp_set_dynamic(0);
    
    /* Process with various OpenMP clauses */
    process_with_openmp(data, N, results);
    
    /* Additional processing with nested control flow */
    for (int iter = 0; iter < 3; iter++) {
        nested_function_with_omp(data, N / (iter + 1));
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    #pragma omp parallel for reduction(+:checksum) \
        schedule(guided) if(N > 5000)
    for (int i = 0; i < N; i++) {
        checksum += data[i];
    }
    
    /* Add results to checksum */
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Final directive combining multiple target clauses */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            #pragma omp parallel for simd schedule(static)
            for (int i = 0; i < 100; i++) {
                data[i] = i;
            }
        }
        #pragma omp section
        {
            #pragma omp taskgroup task_reduction(+:checksum)
            {
                #pragma omp task in_reduction(+:checksum)
                {
                    checksum += 1;
                }
            }
        }
    }
    
    free(data);
    return 0;
}
