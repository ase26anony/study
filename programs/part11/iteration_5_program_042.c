/* test_omp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function attribute to prevent optimization from removing OpenMP constructs */
__attribute__((optimize("O0")))
void process_array(double* arr, int n, double* results) {
    double sum = 0.0;
    double max_val = -1e30;
    
    /* 1. PARALLEL and FOR clauses - combined directive */
    #pragma omp parallel for schedule(dynamic) reduction(+:sum) \
            if(n > 1000) num_threads(omp_get_max_threads())
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i * 1.5;
        sum += arr[i];
    }
    
    /* 2. PARALLEL and SECTIONS clauses - combined directive */
    #pragma omp parallel sections private(max_val) \
            if(omp_get_num_threads() > 1)
    {
        #pragma omp section
        {
            max_val = arr[0];
            for (int i = 1; i < n/2; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            double local_max = arr[n/2];
            for (int i = n/2 + 1; i < n; i++) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            #pragma omp critical
            {
                if (local_max > max_val) max_val = local_max;
            }
        }
    }
    
    results[0] = sum;
    results[1] = max_val;
}

/* Complex directive with FOR clause and arguments */
__attribute__((optimize("O0")))
void distribute_computation(double* arr, int n, int m, double* result) {
    /* FOR clause with explicit arguments in a combined directive */
    #pragma omp target teams distribute parallel for simd \
            schedule(static, 4) collapse(2) map(tofrom: arr[0:n*m]) \
            if(n*m > 10000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            arr[idx] = arr[idx] * 2.0 + (double)(i + j);
        }
    }
    
    /* Force diagnostic with clause name in message */
    #pragma omp error severity(warning) message("Processing FOR clause in distributed loop")
    
    double total = 0.0;
    #pragma omp parallel for reduction(+:total) schedule(guided)
    for (int i = 0; i < n * m; i++) {
        total += arr[i];
    }
    
    *result = total;
}

/* Function using TASKGROUP clause */
__attribute__((optimize("O0")))
double taskgroup_reduction(double* arr, int n) {
    double sum = 0.0;
    
    /* TASKGROUP clause with task_reduction argument */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:sum)
            {
                for (int i = 0; i < n; i += n/10) {
                    #pragma omp task in_reduction(+:sum) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        int end = (i + n/10) < n ? (i + n/10) : n;
                        for (int j = i; j < end; j++) {
                            local_sum += arr[j];
                        }
                        sum += local_sum;
                        
                        /* Nested directive inside task */
                        #pragma omp error severity(message) \
                                message("Task processing section with TASKGROUP clause")
                    }
                }
            }
        }
    }
    
    return sum;
}

/* Macro expansion to create complex patterns for pretty-printer */
#define CREATE_PARALLEL_FOR(init, cond, incr, body) \
    _Pragma("omp parallel for schedule(static)") \
    for(init; cond; incr) { body }

#define CREATE_SECTIONS(section1, section2) \
    _Pragma("omp parallel sections") \
    { \
        _Pragma("omp section") \
        { section1 } \
        _Pragma("omp section") \
        { section2 } \
    }

/* Function using macro expansions */
__attribute__((optimize("O0")))
void macro_based_computation(double* arr, int n, double* out1, double* out2) {
    double sum1 = 0.0, sum2 = 0.0;
    
    /* FOR clause via macro expansion */
    CREATE_PARALLEL_FOR(int i = 0, i < n/2, i++,
        sum1 += arr[i] * 0.5;
    )
    
    /* SECTIONS clause via macro expansion */
    CREATE_SECTIONS(
        {
            for (int i = n/2; i < n; i++) {
                sum2 += arr[i] * 2.0;
            }
        },
        {
            /* Force diagnostic with PARALLEL clause name */
            #pragma omp error severity(warning) \
                    message("In parallel section with SECTIONS clause")
            *out1 = sum1;
        }
    )
    
    *out2 = sum2;
}

/* Main function with complex control flow */
int main(int argc, char** argv) {
    const int N = 10000;
    const int M = 100;
    double* array = (double*)malloc(N * M * sizeof(double));
    double results[4];
    
    /* Switch statement embedding OpenMP directives */
    int mode = 2;  /* Can be changed for different execution paths */
    switch (mode) {
        case 1:
            /* FOR clause in switch case */
            #pragma omp parallel for schedule(runtime) \
                    if(omp_get_num_procs() > 1)
            for (int i = 0; i < N * M; i++) {
                array[i] = 1.0;
            }
            break;
            
        case 2:
            /* Combined PARALLEL FOR with collapse */
            #pragma omp parallel for collapse(2) \
                    ordered(2) lastprivate(i, j)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    array[i * M + j] = (double)(i * M + j);
                    #pragma omp ordered depend(sink: i-1, j) depend(sink: i, j-1)
                    /* Some ordered computation */
                    #pragma omp ordered depend(source)
                }
            }
            break;
            
        default:
            /* SECTIONS clause in default case */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    for (int i = 0; i < N * M; i++) {
                        array[i] = -1.0;
                    }
                }
                #pragma omp section
                {
                    /* Empty section but forces clause representation */
                }
            }
            break;
    }
    
    /* Call functions with different OpenMP clause patterns */
    process_array(array, N * M, results);
    
    double distributed_result;
    distribute_computation(array, N, M, &distributed_result);
    
    double taskgroup_sum = taskgroup_reduction(array, N * M);
    
    double macro_out1, macro_out2;
    macro_based_computation(array, N * M, &macro_out1, &macro_out2);
    
    /* Compute final checksum to prevent dead code elimination */
    double checksum = results[0] + results[1] + distributed_result + 
                     taskgroup_sum + macro_out1 + macro_out2;
    
    printf("Final checksum: %f\n", checksum);
    printf("Array[0] = %f, Array[%d] = %f\n", 
           array[0], N * M - 1, array[N * M - 1]);
    
    free(array);
    return 0;
}
