/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define CHUNK_SIZE 64

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_omp_for(int *arr, int n) {
    int i, j;
    
    /* Complex directive combining 'for' clause with arguments */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) num_teams(2) thread_limit(32) \
        map(tofrom: arr[0:n*n])
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            arr[i * n + j] = (i * j) % 256;
        }
    }
    
    /* Nested directive with 'for' clause */
    #pragma omp parallel for ordered schedule(dynamic, CHUNK_SIZE) \
        private(i, j) shared(arr, n)
    for (i = 0; i < n; i++) {
        #pragma omp ordered
        {
            for (j = 0; j < n; j++) {
                arr[i * n + j] += omp_get_thread_num();
            }
        }
    }
}

/* Function using 'parallel' and 'sections' clauses */
int __attribute__((optimize("O0"))) compute_with_parallel_sections(int *arr, int n) {
    int sum = 0, max_val = 0;
    
    /* Combined 'parallel' and 'sections' clauses */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        num_threads(4) if(n > 500)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            /* Trigger diagnostic with clause name in message */
            #pragma omp error severity(warning) message("Processing section with for clause")
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
        }
        
        /* Third section with nested parallel region */
        #pragma omp section
        {
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum) if(omp_in_parallel())
            for (int i = 0; i < n; i += 2) {
                local_sum += arr[i];
            }
            sum += local_sum;
        }
    }
    
    return sum + max_val;
}

/* Function using 'taskgroup' clause */
void __attribute__((optimize("O0"))) process_with_taskgroup(int *arr, int n, int *result) {
    int sum = 0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:sum)
        {
            for (int i = 0; i < n; i++) {
                #pragma omp task in_reduction(+:sum) firstprivate(i) shared(arr)
                {
                    int val = arr[i];
                    /* Complex expression to prevent optimization */
                    val = (val * 1103515245 + 12345) % 256;
                    sum += val;
                    
                    /* Nested task with error directive */
                    if (val % 37 == 0) {
                        #pragma omp task
                        {
                            /* Force pretty-printing of 'for' clause */
                            _Pragma("omp error severity(message) message(\"Found value requiring for clause processing\")")
                            arr[i] = val * 2;
                        }
                    }
                }
            }
        }
        
        /* Additional taskgroup without reduction for coverage */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                /* Empty task but clause still present */
            }
        }
    }
    
    *result = sum;
}

/* Macro that expands to include clause names */
#define DIAGNOSTIC_MESSAGE(clause) \
    _Pragma("omp error severity(warning) message(\"Testing pretty print for clause: " #clause "\")")

/* Function with mixed OpenMP and C constructs */
void __attribute__((optimize("O0"))) complex_control_flow(int *arr, int n) {
    int i = 0;
    
    /* OpenMP inside switch statement */
    switch (n % 3) {
        case 0:
            #pragma omp parallel for simd simdlen(8) if(n > 100)
            for (i = 0; i < n; i++) {
                arr[i] = arr[i] * arr[i];
            }
            break;
            
        case 1:
            /* Trigger diagnostic for 'for' clause */
            DIAGNOSTIC_MESSAGE(for);
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    for (i = 0; i < n/2; i++) {
                        arr[i] = sqrt(arr[i]);
                    }
                }
                #pragma omp section
                {
                    for (i = n/2; i < n; i++) {
                        arr[i] = log(arr[i] + 1);
                    }
                }
            }
            break;
            
        case 2:
            /* Nested parallel regions */
            #pragma omp parallel num_threads(2)
            {
                #pragma omp single
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            /* Force 'parallel' clause pretty-printing */
                            _Pragma("omp error severity(message) message(\"Task inside parallel region\")")
                        }
                    }
                }
            }
            break;
    }
    
    /* OpenMP inside loop */
    for (int iter = 0; iter < 2; iter++) {
        #pragma omp parallel for schedule(guided) if(iter == 1)
        for (i = 0; i < n; i++) {
            arr[i] += iter;
        }
    }
}

int main() {
    int *array = (int*)malloc(N * N * sizeof(int));
    int checksum = 0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with OpenMP parallel for */
    #pragma omp parallel for schedule(dynamic) shared(array) \
        default(none) firstprivate(N)
    for (int i = 0; i < N * N; i++) {
        array[i] = i % 100;
    }
    
    /* Process with various OpenMP constructs */
    process_with_omp_for(array, N);
    
    checksum += compute_with_parallel_sections(array, N * N);
    
    int task_result = 0;
    process_with_taskgroup(array, N * N, &task_result);
    checksum += task_result;
    
    complex_control_flow(array, N * N);
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum) \
        if(omp_get_max_threads() > 1)
    for (int i = 0; i < N * N; i++) {
        checksum += array[i] % 17;
    }
    
    /* Additional directive to ensure all clauses are processed */
    #pragma omp parallel master taskloop simd \
        num_tasks(4) simdlen(4) shared(array, checksum)
    for (int i = 0; i < 100; i++) {
        checksum += i;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
