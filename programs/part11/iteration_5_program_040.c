#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define SIZE 1000
#define NESTED_LOOP_SIZE 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_for_clause(int *arr) {
    int i, j;
    
    /* Complex directive with 'for' clause and multiple arguments */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) num_teams(2) thread_limit(64) \
        map(tofrom: arr[0:SIZE*NESTED_LOOP_SIZE])
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < NESTED_LOOP_SIZE; j++) {
            arr[i * NESTED_LOOP_SIZE + j] = i * j + omp_get_thread_num();
        }
    }
    
    /* Nested directive with 'for' clause */
    #pragma omp parallel
    {
        #pragma omp for nowait schedule(dynamic, 8) ordered
        for (i = 0; i < SIZE; i++) {
            #pragma omp ordered
            arr[i] *= 2;
        }
    }
}

/* Function using 'parallel' and 'sections' clauses */
int __attribute__((optimize("O0"))) compute_with_sections(int *arr) {
    int sum = 0, max_val = INT_MIN;
    
    /* Combined 'parallel' and 'sections' clauses */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(arr) if(SIZE > 500)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < SIZE/2; i++) {
                sum += arr[i];
            }
            /* Force diagnostic with clause name in message */
            #pragma omp error severity(warning) message("Processing section with for clause")
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = SIZE/2; i < SIZE; i++) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
        }
        
        /* Third section with nested parallel region */
        #pragma omp section
        {
            #pragma omp parallel for simd reduction(+:sum) if(omp_get_num_threads() > 1)
            for (int i = 0; i < SIZE; i += 2) {
                sum += arr[i] % 100;
            }
        }
    }
    
    return sum + max_val;
}

/* Complex function using 'taskgroup' clause */
void __attribute__((optimize("O0"))) task_based_computation(int *arr, int *result) {
    int task_sum = 0;
    
    /* Taskgroup with explicit task_reduction clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                /* Multiple tasks within taskgroup */
                for (int i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i) \
                        depend(out: arr[i*100]) if(i < 5)
                    {
                        int local_sum = 0;
                        for (int j = i * 100; j < (i + 1) * 100; j++) {
                            local_sum += arr[j];
                        }
                        task_sum += local_sum;
                        
                        /* Nested task with error directive */
                        if (i == 3) {
                            #pragma omp task
                            {
                                /* Force pretty-printing of 'for' clause */
                                _Pragma("omp error severity(message) message(\"Inside task with for clause\")")
                                for (int k = 0; k < 10; k++) {
                                    arr[k] = k;
                                }
                            }
                        }
                    }
                }
                
                /* Additional task with sections */
                #pragma omp task untied mergeable
                {
                    #pragma omp parallel sections
                    {
                        #pragma omp section
                        { *result = task_sum; }
                        
                        #pragma omp section
                        { 
                            /* Macro expansion with clause */
                            #define OMP_PARALLEL_FOR _Pragma("omp parallel for")
                            OMP_PARALLEL_FOR
                            for (int i = 0; i < 100; i++) {
                                arr[i] = i;
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Function with switch statement containing OpenMP directives */
void __attribute__((optimize("O0"))) switch_with_omp(int mode, int *arr, int *out) {
    switch (mode) {
        case 1:
            /* 'for' clause in switch case */
            #pragma omp parallel for simd safelen(16) linear(i:1)
            for (int i = 0; i < SIZE; i++) {
                arr[i] = arr[i] * 2 + 1;
            }
            break;
            
        case 2:
            /* 'parallel' clause in switch case */
            #pragma omp parallel if(SIZE > 100) proc_bind(close)
            {
                #pragma omp for schedule(guided)
                for (int i = 0; i < SIZE; i++) {
                    out[i] = arr[i] / 2;
                }
            }
            break;
            
        case 3:
            /* 'sections' clause in switch case */
            #pragma omp parallel sections
            {
                #pragma omp section
                { out[0] = arr[0]; }
                
                #pragma omp section
                { 
                    #pragma omp parallel for
                    for (int i = 1; i < SIZE; i++) {
                        out[i] = arr[i] + out[i-1];
                    }
                }
            }
            break;
            
        default:
            /* 'taskgroup' clause in default case */
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    for (int i = 0; i < SIZE; i++) {
                        out[i] = -arr[i];
                    }
                }
            }
            break;
    }
}

int main() {
    int *array = (int*)malloc(SIZE * NESTED_LOOP_SIZE * sizeof(int));
    int *result = (int*)malloc(SIZE * sizeof(int));
    int checksum = 0;
    
    if (!array || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with parallel for (triggers 'for' clause) */
    #pragma omp parallel for schedule(dynamic) num_threads(4) \
        default(none) shared(array) private(checksum)
    for (int i = 0; i < SIZE * NESTED_LOOP_SIZE; i++) {
        array[i] = i % 100;
    }
    
    /* Process with complex for clause */
    process_with_for_clause(array);
    
    /* Compute with sections (triggers 'parallel' and 'sections' clauses) */
    checksum += compute_with_sections(array);
    
    /* Task-based computation (triggers 'taskgroup' clause) */
    int task_result = 0;
    task_based_computation(array, &task_result);
    checksum += task_result;
    
    /* Switch with different OpenMP modes */
    for (int mode = 1; mode <= 4; mode++) {
        switch_with_omp(mode, array, result);
        
        /* Verify results to prevent optimization */
        #pragma omp parallel for reduction(+:checksum)
        for (int i = 0; i < SIZE; i++) {
            checksum += result[i] % 1000;
        }
    }
    
    /* Final parallel region with combined clauses */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Force diagnostic that should trigger pretty-printing */
            #pragma omp error severity(warning) \
                message("Final check: for, parallel, sections, taskgroup clauses processed")
        }
        
        #pragma omp barrier
        
        #pragma omp for nowait
        for (int i = 0; i < SIZE; i++) {
            result[i] = array[i] + checksum;
        }
    }
    
    /* Final reduction */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) lastprivate(checksum)
    for (int i = 0; i < SIZE; i++) {
        final_sum += result[i];
        checksum = i;  /* Use lastprivate variable */
    }
    
    printf("Checksum: %d, Final sum: %d, Last index: %d\n", 
           checksum, final_sum, SIZE-1);
    
    free(array);
    free(result);
    
    return 0;
}
