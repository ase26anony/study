/* test_omp_clauses.c - Targeting uncovered pretty-print cases for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 10000
#define NESTED_LOOP_SIZE 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_with_for_clause(int *arr, int n) {
    int i, j;
    
    /* Complex directive with 'for' clause - will trigger pretty-print of OMP_CLAUSE_FOR */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(i, j) if(n > 1000)
    for (i = 0; i < NESTED_LOOP_SIZE; i++) {
        for (j = 0; j < NESTED_LOOP_SIZE; j++) {
            if (i * j < n) {
                arr[i * NESTED_LOOP_SIZE + j] = i + j;
            }
        }
    }
    
    /* Nested directive with diagnostic - forces pretty-printing during compilation */
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < n; i++) {
        if (arr[i] % 7 == 0) {
            /* This will trigger pretty-print when compiler processes the error directive */
            #pragma omp error severity(warning) message("Processing for clause iteration")
            arr[i] *= 2;
        }
    }
}

/* Function using sections clause */
int __attribute__((optimize("O0"))) process_with_sections_clause(int *arr, int n) {
    int sum = 0, max_val = arr[0];
    
    /* Combined parallel and sections clauses - triggers both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        num_threads(4) if(n > 500)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            /* Runtime call inside section */
            int threads = omp_get_num_threads();
            if (threads > 2) {
                /* Nested diagnostic with clause name */
                _Pragma("omp error severity(message) message(\"sections clause active\")")
            }
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
        
        /* Third section with nested control flow */
        #pragma omp section
        {
            switch (n % 3) {
                case 0:
                    /* Complex macro expansion with _Pragma */
                    #define PROCESS_CHUNK(start, end) \
                        _Pragma("omp parallel for if(end-start > 100)") \
                        for (int k = start; k < end; k++) { \
                            arr[k] = arr[k] % 256; \
                        }
                    
                    PROCESS_CHUNK(0, n/3)
                    break;
                default:
                    break;
            }
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
int __attribute__((optimize("O0"))) process_with_taskgroup_clause(int *arr, int n) {
    int total = 0;
    
    /* Taskgroup with reduction - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:total)
        {
            for (int i = 0; i < n; i += n/10) {
                int start = i;
                int end = (i + n/10 < n) ? i + n/10 : n;
                
                #pragma omp task in_reduction(+:total) firstprivate(start, end)
                {
                    int local_sum = 0;
                    for (int j = start; j < end; j++) {
                        local_sum += arr[j];
                    }
                    total += local_sum;
                    
                    /* Nested task inside taskgroup */
                    #pragma omp task if(end - start > 5)
                    {
                        /* Diagnostic with clause reference */
                        #pragma omp error severity(warning) \
                            message("taskgroup clause processing subtask")
                    }
                }
            }
        }
    }
    
    return total;
}

/* Main function with mixed OpenMP constructs */
int main() {
    int *array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int checksum = 0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Phase 1: Initialize array with parallel for */
    printf("Phase 1: Initializing array with parallel for...\n");
    #pragma omp parallel for schedule(static) if(ARRAY_SIZE > 1000)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i * 17) % 7919;  /* Prime number modulus for distribution */
    }
    
    /* Phase 2: Process with for clause (distribute parallel for simd) */
    printf("Phase 2: Processing with for clause...\n");
    process_with_for_clause(array, ARRAY_SIZE);
    
    /* Phase 3: Process with sections clause */
    printf("Phase 3: Processing with sections clause...\n");
    checksum += process_with_sections_clause(array, ARRAY_SIZE);
    
    /* Phase 4: Process with taskgroup clause */
    printf("Phase 4: Processing with taskgroup clause...\n");
    checksum += process_with_taskgroup_clause(array, ARRAY_SIZE);
    
    /* Additional complex nested directive combining multiple clauses */
    printf("Phase 5: Complex nested directives...\n");
    {
        int chunk_results[4] = {0};
        
        /* This complex directive should trigger pretty-print for multiple clauses */
        #pragma omp parallel num_threads(2) if(ARRAY_SIZE > 100)
        {
            #pragma omp for schedule(guided) nowait
            for (int i = 0; i < 4; i++) {
                int start = i * (ARRAY_SIZE / 4);
                int end = (i == 3) ? ARRAY_SIZE : (i + 1) * (ARRAY_SIZE / 4);
                
                /* Nested sections inside parallel region */
                #pragma omp sections private(start, end)
                {
                    #pragma omp section
                    {
                        for (int j = start; j < end; j += 2) {
                            chunk_results[i] += array[j];
                        }
                    }
                    #pragma omp section
                    {
                        for (int j = start + 1; j < end; j += 2) {
                            chunk_results[i] -= array[j];
                        }
                    }
                }
            }
        }
        
        for (int i = 0; i < 4; i++) {
            checksum += chunk_results[i];
        }
    }
    
    /* Final computation to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    free(array);
    return 0;
}
