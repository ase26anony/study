/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define SIZE 1000
#define NESTED_LOOP_SIZE 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_omp_for(int *arr, int n) {
    int i, j;
    
    /* Combined parallel and for clause - triggers both uncovered cases */
    #pragma omp parallel for schedule(static, 4) collapse(2) \
        private(i, j) shared(arr) if(n > 100)
    for (i = 0; i < NESTED_LOOP_SIZE; i++) {
        for (j = 0; j < NESTED_LOOP_SIZE; j++) {
            arr[(i * NESTED_LOOP_SIZE + j) % n] += i * j;
        }
    }
    
    /* Complex directive with for clause in different context */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:n]) schedule(dynamic) num_teams(4) \
        thread_limit(8) if(omp_get_num_threads() > 1)
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + 1;
    }
}

/* Function using sections clause */
int __attribute__((optimize("O0"), noinline))
process_with_omp_sections(int *arr, int n) {
    int sum = 0;
    int max_val = INT_MIN;
    
    /* Combined parallel and sections clause */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(arr) shared(n) num_threads(2)
    {
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            /* Nested directive inside section */
            #pragma omp parallel for simd schedule(guided)
            for (int i = 0; i < n/4; i++) {
                arr[i] = sum % (i + 1);
            }
        }
        
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
            /* Another directive with error for diagnostic */
            #pragma omp error severity(warning) message("Processing sections clause")
            for (int i = n/2; i < n; i++) {
                arr[i] = max_val - arr[i];
            }
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
void __attribute__((optimize("O0"), noinline))
process_with_omp_taskgroup(int *arr, int n, int *result) {
    int sum = 0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:sum) \
            allocate(omp_default_mem_alloc: sum)
        {
            /* Spawn multiple tasks */
            for (int i = 0; i < 4; i++) {
                #pragma omp task in_reduction(+:sum) shared(arr, n) \
                    firstprivate(i) mergeable
                {
                    int local_sum = 0;
                    int start = i * (n / 4);
                    int end = (i + 1) * (n / 4);
                    
                    for (int j = start; j < end; j++) {
                        local_sum += arr[j];
                    }
                    
                    sum += local_sum;
                    
                    /* Nested task with error directive */
                    if (i == 0) {
                        #pragma omp task
                        {
                            /* Force diagnostic with clause name */
                            _Pragma("omp error severity(message) message(\"for clause in task\")")
                            for (int k = 0; k < 10; k++) {
                                arr[k] = local_sum % (k + 1);
                            }
                        }
                    }
                }
            }
        }
        
        /* Another taskgroup with different structure */
        #pragma omp taskgroup
        {
            #pragma omp task untied
            {
                /* Complex macro expansion with clause */
                #define PROCESS_CHUNK(start, end) \
                    _Pragma("omp parallel for schedule(runtime)") \
                    for (int idx = start; idx < end; idx++) { \
                        arr[idx] = arr[idx] * 3 / 2; \
                    }
                
                PROCESS_CHUNK(0, n/2)
                #undef PROCESS_CHUNK
            }
        }
    }
    
    *result = sum;
}

/* Main function with mixed OpenMP and C constructs */
int main(int argc, char *argv[]) {
    int *array = (int *)malloc(SIZE * sizeof(int));
    int checksum = 0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with parallel for */
    #pragma omp parallel for schedule(dynamic) \
        shared(array) if(SIZE > 500)
    for (int i = 0; i < SIZE; i++) {
        array[i] = i % 100;
    }
    
    /* Switch statement with different OpenMP directives */
    int mode = 1;
    switch (mode) {
        case 1:
            /* Process with for clause */
            process_with_omp_for(array, SIZE);
            break;
        case 2:
            /* Process with sections clause */
            checksum += process_with_omp_sections(array, SIZE);
            break;
        default:
            /* Taskgroup processing */
            int task_result;
            process_with_omp_taskgroup(array, SIZE, &task_result);
            checksum += task_result;
            break;
    }
    
    /* Nested loop with OpenMP directive */
    for (int outer = 0; outer < 3; outer++) {
        /* Combined parallel sections */
        #pragma omp parallel sections private(outer) \
            shared(array, checksum) if(omp_in_parallel())
        {
            #pragma omp section
            {
                /* Directive with error for diagnostic */
                #pragma omp error severity(warning) \
                    message("parallel sections clause in loop")
                for (int i = 0; i < SIZE/3; i++) {
                    checksum += array[i];
                }
            }
            
            #pragma omp section
            {
                /* Another directive */
                #pragma omp parallel for simd schedule(static) \
                    safelen(8) linear(i:1)
                for (int i = SIZE/3; i < 2*SIZE/3; i++) {
                    array[i] = checksum % (i + 1);
                }
            }
        }
    }
    
    /* Final computation with complex directive */
    int final_sum = 0;
    #pragma omp target enter data map(to: array[0:SIZE])
    
    #pragma omp target teams distribute parallel for \
        reduction(+:final_sum) map(tofrom: final_sum) \
        is_device_ptr(array) depend(inout: array) nowait
    for (int i = 0; i < SIZE; i++) {
        final_sum += array[i];
    }
    
    #pragma omp taskwait
    
    checksum += final_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
