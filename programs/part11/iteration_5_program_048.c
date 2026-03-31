/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define M 100
#define CHUNK_SIZE 64

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_omp_for(int *arr, int n) {
    int i, j;
    
    /* Combined parallel and for clause - triggers both uncovered cases */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) private(j) \
        if(n > 100) num_threads(4)
    for (i = 0; i < n; i++) {
        for (j = 0; j < M; j++) {
            arr[i] += (i * j) % 7;
        }
    }
    
    /* Complex for clause with multiple arguments in distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom: arr[0:n*M]) \
        num_teams(2) thread_limit(8)
    for (i = 0; i < n; i++) {
        for (j = 0; j < M; j++) {
            arr[i * M + j] *= 2;
        }
    }
}

/* Function using sections clause */
int __attribute__((optimize("O0"))) compute_with_omp_sections(int *arr, int n) {
    int sum = 0, max_val = arr[0];
    
    /* Combined parallel and sections clause */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(n) if(n > 50)
    {
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            /* Nested directive with for clause */
            #pragma omp parallel for simd schedule(guided)
            for (int i = 0; i < n/4; i++) {
                arr[i] = sqrt(arr[i]);
            }
        }
        
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
                sum += arr[i];
            }
        }
        
        #pragma omp section
        {
            /* Empty section to test sections clause representation */
            #pragma omp critical
            {
                printf("Section 3 executing with %d threads\n", omp_get_num_threads());
            }
        }
    }
    
    return sum + max_val;
}

/* Complex taskgroup usage */
long __attribute__((optimize("O0"))) taskgroup_reduction_example(int *arr, int n) {
    long total = 0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel master taskloop reduction(+:total) \
        task_reduction(+:total) num_tasks(10)
    for (int i = 0; i < n; i++) {
        #pragma omp taskgroup task_reduction(+:total)
        {
            #pragma omp task in_reduction(+:total) firstprivate(i)
            {
                int local_sum = 0;
                for (int j = 0; j < M; j++) {
                    local_sum += arr[i * M + j];
                }
                total += local_sum;
                
                /* Force diagnostic with clause name in message */
                #pragma omp error severity(warning) message("Processing with for clause")
            }
            
            #pragma omp task in_reduction(+:total) firstprivate(i)
            {
                /* Another task with nested parallel for */
                #pragma omp parallel for schedule(static) if(M > 50)
                for (int j = 0; j < M/2; j++) {
                    arr[i * M + j] = arr[i * M + j] % 100;
                }
                total += i;
            }
        }
    }
    
    return total;
}

/* Macro expansion with _Pragma to create complex patterns */
#define CREATE_PARALLEL_REGION(iterations) \
    _Pragma("omp parallel for schedule(static)") \
    for (int _i = 0; _i < (iterations); _i++)

#define CREATE_SECTIONS_REGION() \
    _Pragma("omp parallel sections") \
    { \
        _Pragma("omp section") \
        { /* code */ } \
        _Pragma("omp section") \
        { /* code */ } \
    }

/* Function using macro expansions */
void __attribute__((optimize("O0"))) macro_based_omp(int *arr, int n) {
    /* This expands to parallel for clause */
    CREATE_PARALLEL_REGION(n) {
        arr[_i] = _i * 2;
    }
    
    /* This expands to parallel sections clause */
    CREATE_SECTIONS_REGION();
}

/* Main function with mixed OpenMP and C constructs */
int main(int argc, char **argv) {
    int *array = (int*)malloc(N * M * sizeof(int));
    long checksum = 0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with OpenMP parallel for */
    #pragma omp parallel for schedule(static) if(N > 500) \
        ordered proc_bind(spread)
    for (int i = 0; i < N * M; i++) {
        array[i] = i % 97;
    }
    
    /* Switch statement with OpenMP directives inside */
    int mode = 2;
    switch (mode) {
        case 1:
            /* Nested parallel for inside switch */
            #pragma omp parallel for collapse(2) schedule(dynamic, 16)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    array[i * M + j] += omp_get_thread_num();
                }
            }
            break;
            
        case 2:
            /* Process with for clause function */
            process_with_omp_for(array, N);
            
            /* Compute with sections clause */
            checksum += compute_with_omp_sections(array, N);
            
            /* Taskgroup reduction */
            checksum += taskgroup_reduction_example(array, N);
            break;
            
        default:
            /* Macro-based OpenMP */
            macro_based_omp(array, N);
            break;
    }
    
    /* Final computation with complex directive combining multiple clauses */
    #pragma omp target parallel for simd schedule(nonmonotonic:dynamic) \
        device(0) map(tofrom: array[0:N*M], checksum) \
        depend(inout: array) nowait
    for (int i = 0; i < N * M; i++) {
        checksum += array[i];
    }
    
    /* Additional taskgroup in loop context */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int iter = 0; iter < 5; iter++) {
                #pragma omp taskgroup task_reduction(+:checksum)
                {
                    #pragma omp task in_reduction(+:checksum)
                    {
                        checksum += iter;
                        
                        /* Trigger diagnostic during compilation */
                        #pragma omp error severity(message) \
                            message("Taskgroup clause in iteration")
                    }
                }
            }
        }
    }
    
    printf("Final checksum: %ld\n", checksum);
    
    /* Prevent dead code elimination */
    volatile long result = checksum;
    
    free(array);
    return (int)(result % 1000);
}
