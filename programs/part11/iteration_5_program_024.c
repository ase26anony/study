/* test_omp_clauses.c - Targeting GCC tree-pretty-print.cc lines 1434-1445 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_omp_for(int *arr, int n) {
    int i, j;
    
    /* Complex directive with 'for' clause - triggers OMP_CLAUSE_FOR */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
            private(i, j) if(n > 1000)
    for (i = 0; i < n; i += 100) {
        for (j = 0; j < 100 && (i + j) < n; j++) {
            arr[i + j] = (arr[i + j] * 3) / 2;
        }
    }
    
    /* Nested directive with diagnostic */
    #pragma omp parallel
    {
        #pragma omp single
        {
            /* Force pretty-printing of 'for' clause in error message */
            #pragma omp error severity(warning) message("Directive with for clause")
        }
    }
}

/* Function using 'parallel' and 'sections' clauses */
int __attribute__((optimize("O0")))
compute_with_parallel_sections(int *arr, int n) {
    int sum = 0, max_val = arr[0];
    
    /* Combined 'parallel' and 'sections' clauses */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
            num_threads(4) if(n > 500)
    {
        /* OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS should be printed */
        #pragma omp section
        {
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum) if(omp_get_num_threads() > 1)
            for (int i = 0; i < n/2; i++) {
                local_sum += arr[i];
            }
            sum += local_sum;
            
            /* Nested error directive */
            #pragma omp error severity(message) message("In sections: parallel")
        }
        
        #pragma omp section
        {
            #pragma omp parallel for reduction(max:max_val)
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
            
            /* Macro expansion with _Pragma for complex pretty-printing */
            #define EMIT_SECTIONS_WARNING _Pragma("omp error severity(warning) message(\"sections clause\")")
            EMIT_SECTIONS_WARNING;
        }
    }
    
    return sum + max_val;
}

/* Function using 'taskgroup' clause */
void __attribute__((optimize("O0")))
process_with_taskgroup(int *arr, int n, int *result) {
    int task_sum = 0;
    
    /* Taskgroup with task_reduction - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < n; i += n/10) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        int local_sum = 0;
                        int end = (i + n/10) < n ? (i + n/10) : n;
                        for (int j = i; j < end; j++) {
                            local_sum += arr[j];
                        }
                        task_sum += local_sum;
                        
                        /* Nested directive inside task */
                        if (local_sum > 1000) {
                            #pragma omp error severity(warning) \
                                    message("taskgroup clause with large sum")
                        }
                    }
                }
            }
            
            /* Another taskgroup with different structure */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    /* Force pretty-printing through macro in switch */
                    switch (omp_get_thread_num()) {
                        case 0:
                            #define TASKGROUP_PRAGMA _Pragma("omp error message(\"taskgroup in switch\")")
                            TASKGROUP_PRAGMA;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    
    *result = task_sum;
}

/* Main function with mixed OpenMP constructs */
int main() {
    int *array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int checksum = 0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Phase 1: Initialize with parallel for - triggers OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic) if(ARRAY_SIZE > 1000) \
            shared(array) default(none)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i * 3) % 97;
    }
    
    /* Phase 2: Process with complex for clause */
    process_with_omp_for(array, ARRAY_SIZE);
    
    /* Phase 3: Compute with parallel sections */
    checksum += compute_with_parallel_sections(array, ARRAY_SIZE);
    
    /* Phase 4: Taskgroup processing */
    int task_result = 0;
    process_with_taskgroup(array, ARRAY_SIZE, &task_result);
    checksum += task_result;
    
    /* Additional mixed constructs in main */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Combined directive in loop */
            for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
                #pragma omp parallel for simd schedule(guided) \
                        if(iter % 10 == 0)
                for (int i = 0; i < ARRAY_SIZE/10; i++) {
                    array[i] += iter;
                }
                
                /* Sections inside parallel region */
                if (iter % 20 == 0) {
                    #pragma omp sections private(iter)
                    {
                        #pragma omp section
                        { array[0]++; }
                        
                        #pragma omp section  
                        { array[1]--; }
                    }
                }
            }
        }
    }
    
    /* Final computation to prevent dead code elimination */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) \
            if(ARRAY_SIZE > 100) schedule(static)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += array[i];
    }
    
    checksum += final_sum;
    
    printf("Final checksum: %d\n", checksum);
    
    free(array);
    return 0;
}
