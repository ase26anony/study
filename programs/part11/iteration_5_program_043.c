/* test_omp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define M 100
#define CHUNK_SIZE 64

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_parallel_for(double *arr, int n) {
    int i;
    /* Combined parallel and for clause - will trigger both pretty-print cases */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) private(i) \
        if(n > 100) num_threads(4)
    for (i = 0; i < n; i++) {
        arr[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Complex for clause with multiple arguments */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom: arr[0:n*n])
    for (i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            arr[i * n + j] *= 1.1;
        }
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) compute_with_sections(double *arr, int n) {
    double sum = 0.0, max_val = -1e30;
    
    /* Combined parallel and sections clause */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        private(arr) if(n > 50)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                sum += arr[i];
            }
            /* Nested directive with diagnostic */
            #pragma omp error message("Processing section 1 with for clause")
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        /* Third section using macro expansion with _Pragma */
        #pragma omp section
        {
            #define EMIT_FOR_CLAUSE _Pragma("omp parallel for")
            EMIT_FOR_CLAUSE
            for (int i = 0; i < n; i += 2) {
                arr[i] = sqrt(fabs(arr[i]));
            }
        }
    }
    
    return sum + max_val;
}

/* Function using taskgroup clause */
void __attribute__((optimize("O0"))) process_with_taskgroup(double *arr, int n, double *result) {
    double task_sum = 0.0;
    
    /* Taskgroup with task_reduction clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < n; i += M) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        int end = (i + M < n) ? i + M : n;
                        for (int j = i; j < end; j++) {
                            local_sum += arr[j] * arr[j];
                        }
                        task_sum += local_sum;
                        
                        /* Force diagnostic with clause name in message */
                        if (local_sum > 1000.0) {
                            #pragma omp error severity(warning) \
                                message("Large task reduction with for clause")
                        }
                    }
                }
            }
        }
    }
    
    *result = task_sum;
}

/* Complex control flow with mixed OpenMP directives */
double __attribute__((optimize("O0"))) nested_omp_in_switch(int mode, double *arr, int n) {
    double val = 0.0;
    
    switch (mode) {
        case 1: {
            /* Sections inside switch case */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    val = arr[0];
                }
                #pragma omp section
                {
                    for (int i = 1; i < n; i++) {
                        val += arr[i];
                    }
                }
            }
            break;
        }
        case 2: {
            /* Parallel for with runtime calls */
            int num_threads = 0;
            #pragma omp parallel for schedule(guided) \
                if(n > 1000) default(none) shared(arr, n, num_threads)
            for (int i = 0; i < n; i++) {
                #pragma omp critical
                {
                    if (omp_get_thread_num() == 0) {
                        num_threads = omp_get_num_threads();
                    }
                }
                arr[i] *= (1.0 + 0.001 * omp_get_thread_num());
            }
            val = num_threads;
            break;
        }
        case 3: {
            /* Taskgroup in switch with macro */
            #define TASKGROUP_REDUCTION _Pragma("omp taskgroup task_reduction(+:val)")
            TASKGROUP_REDUCTION
            {
                #pragma omp task in_reduction(+:val)
                {
                    for (int i = 0; i < n; i++) {
                        val += sin(arr[i]);
                    }
                }
            }
            break;
        }
        default:
            /* Simple parallel for */
            #pragma omp parallel for reduction(+:val)
            for (int i = 0; i < n; i++) {
                val += arr[i];
            }
    }
    
    return val;
}

/* Main function with multiple OpenMP regions */
int main() {
    double *array = (double *)malloc(N * N * sizeof(double));
    double checksum = 0.0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with parallel for */
    #pragma omp parallel for schedule(static) \
        if(N*N > 10000) num_threads(omp_get_max_threads())
    for (int i = 0; i < N * N; i++) {
        array[i] = (i % 100) * 0.01;
    }
    
    /* Process with various OpenMP constructs */
    process_with_parallel_for(array, N);
    
    double section_result = compute_with_sections(array, N);
    checksum += section_result;
    
    double taskgroup_result;
    process_with_taskgroup(array, N * N, &taskgroup_result);
    checksum += taskgroup_result;
    
    /* Test nested constructs */
    for (int mode = 1; mode <= 4; mode++) {
        double nested_result = nested_omp_in_switch(mode, array, 100);
        checksum += nested_result;
        
        /* Additional combined directive */
        if (mode % 2 == 0) {
            #pragma omp parallel for simd schedule(nonmonotonic:dynamic) \
                aligned(array:64) if(mode==2)
            for (int i = 0; i < 100; i++) {
                array[i] = log1p(fabs(array[i]));
            }
        }
    }
    
    /* Final reduction with complex clause combination */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        schedule(runtime) order(concurrent)
    for (int i = 0; i < N * N; i++) {
        final_sum += array[i];
    }
    
    checksum += final_sum;
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %f\n", checksum);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    free(array);
    return 0;
}
