/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <limits.h>

#define SIZE 1000
#define CHUNK 64

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_with_omp_for(int *arr, int n) {
    int i, j;
    
    /* Combined parallel and for clause - will trigger 'parallel' and 'for' pretty-printing */
    #pragma omp parallel for schedule(dynamic, CHUNK) private(i) \
        if(n > 100) num_threads(omp_get_max_threads())
    for (i = 0; i < n; i++) {
        arr[i] = i * 2;
    }
    
    /* Nested directive with for clause in distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom: arr[0:n*n])
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i + j < n) {
                arr[i * n + j] += j;
            }
        }
    }
}

/* Function using sections clause */
int __attribute__((optimize("O0"))) compute_with_omp_sections(int *arr, int n) {
    int sum = 0, max_val = INT_MIN;
    
    /* Combined parallel and sections clause */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(n) if(n > 50)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            /* Force diagnostic with clause name in message */
            #pragma omp error severity(warning) message("Processing in sections clause, part 1")
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
            /* Another diagnostic */
            #pragma omp error severity(message) message("Processing in sections clause, part 2")
        }
        
        /* Third section using _Pragma for complex pattern */
        #pragma omp section
        {
            #define SECTION_MACRO(x) _Pragma("omp critical") { x; }
            SECTION_MACRO(sum += 1);
            #undef SECTION_MACRO
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
int __attribute__((optimize("O0"))) process_with_taskgroup(int *arr, int n) {
    int total = 0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:total)
        {
            for (int i = 0; i < n; i++) {
                #pragma omp task in_reduction(+:total) firstprivate(i) \
                    if(i % 2 == 0)
                {
                    total += arr[i];
                    
                    /* Nested task with error directive containing clause name */
                    if (i == n/2) {
                        #pragma omp task
                        {
                            /* This should trigger pretty-printing of 'for' clause */
                            #pragma omp error severity(warning) \
                                message("Inside taskgroup, simulating for clause behavior")
                        }
                    }
                }
            }
        }
    }
    
    return total;
}

/* Complex control flow with embedded OpenMP */
void __attribute__((optimize("O0"))) mixed_control_flow(int *arr, int n) {
    int i = 0;
    
    switch (n % 3) {
        case 0:
            /* Directives inside switch case */
            #pragma omp parallel for ordered schedule(guided)
            for (i = 0; i < n; i++) {
                #pragma omp ordered
                arr[i] = arr[i] * 3;
            }
            break;
            
        case 1:
            {
                /* Nested function-like block */
                auto void process_chunk(int start, int end) {
                    #pragma omp taskloop grainsize(16) \
                        if(end-start > 32) shared(arr)
                    for (int j = start; j < end; j++) {
                        arr[j] = arr[j] / 2;
                    }
                }
                
                #pragma omp parallel
                {
                    #pragma omp single
                    {
                        process_chunk(0, n/2);
                        process_chunk(n/2, n);
                    }
                }
            }
            break;
            
        case 2:
            /* Loop with embedded directive */
            for (int iter = 0; iter < 3; iter++) {
                #pragma omp parallel sections if(iter > 0) \
                    num_threads(omp_get_num_procs() / 2)
                {
                    #pragma omp section
                    {
                        #pragma omp simd safelen(8)
                        for (i = 0; i < n; i++) {
                            arr[i] += iter;
                        }
                    }
                    #pragma omp section
                    {
                        #pragma omp simd
                        for (i = 0; i < n; i++) {
                            arr[i] -= 1;
                        }
                    }
                }
            }
            break;
    }
}

/* Main function with execution flow as specified */
int main() {
    int *array = (int*)malloc(SIZE * SIZE * sizeof(int));
    int checksum = 0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* 1. Parallel for with schedule(dynamic) */
    #pragma omp parallel for schedule(dynamic) \
        shared(array) default(none) if(SIZE > 100)
    for (int i = 0; i < SIZE * SIZE; i++) {
        array[i] = i % 100;
    }
    
    /* 2. Process with combined parallel for clause */
    process_with_omp_for(array, SIZE);
    
    /* 3. Parallel sections with reductions */
    checksum += compute_with_omp_sections(array, SIZE);
    
    /* 4. Taskgroup with task_reduction */
    checksum += process_with_taskgroup(array, SIZE);
    
    /* 5. Mixed control flow */
    mixed_control_flow(array, SIZE);
    
    /* Final computation to prevent dead code elimination */
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum) \
        schedule(static) collapse(2)
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            final_sum += array[i * SIZE + j];
        }
    }
    
    checksum += final_sum;
    
    /* Use runtime calls in conditional directives */
    #pragma omp parallel if(omp_in_parallel() == 0) \
        num_threads(omp_get_max_threads())
    {
        int tid = omp_get_thread_num();
        #pragma omp critical
        {
            printf("Thread %d: checksum contribution processed\n", tid);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    free(array);
    return 0;
}
