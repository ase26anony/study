/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 1000
#define M 100
#define CHUNK_SIZE 64

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_with_parallel_for(double *arr, int n) {
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) default(none) shared(arr, n)
    for (int i = 0; i < n; i++) {
        arr[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Nested directive combining parallel and for clauses */
    #pragma omp parallel for simd collapse(2) schedule(static, 4) if(n > 500)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            arr[i*2 + j] += sqrt(fabs(arr[i*2 + j]));
        }
    }
}

/* Function using parallel sections */
double __attribute__((optimize("O0"))) compute_with_sections(double *arr, int n) {
    double sum = 0.0, max_val = -INFINITY;
    
    #pragma omp parallel sections default(none) shared(arr, n, sum, max_val)
    {
        #pragma omp section
        {
            double local_sum = 0.0;
            #pragma omp parallel for reduction(+:local_sum) if(n > 100)
            for (int i = 0; i < n; i += 2) {
                local_sum += arr[i];
            }
            #pragma omp atomic
            sum += local_sum;
        }
        
        #pragma omp section
        {
            double local_max = -INFINITY;
            #pragma omp parallel for reduction(max:local_max)
            for (int i = 1; i < n; i += 2) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            #pragma omp atomic
            if (local_max > max_val) max_val = local_max;
        }
    }
    
    /* Force diagnostic with sections clause name */
    #pragma omp error severity(warning) message("Processing completed for sections clause")
    
    return sum + max_val;
}

/* Complex taskgroup usage with task_reduction */
double __attribute__((optimize("O0"))) task_based_computation(double *arr, int n) {
    double task_sum = 0.0;
    
    #pragma omp parallel default(none) shared(arr, n, task_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < n; i += M) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double block_sum = 0.0;
                        int end = (i + M < n) ? i + M : n;
                        for (int j = i; j < end; j++) {
                            block_sum += arr[j] * arr[j];
                        }
                        #pragma omp atomic
                        task_sum += block_sum;
                        
                        /* Nested task with error directive containing clause name */
                        #pragma omp task if(block_sum > 0)
                        {
                            /* This will trigger pretty-printing of 'for' clause */
                            #pragma omp error severity(message) \
                                message("Task completed: for clause was used in parent context")
                        }
                    }
                }
            }
        }
    }
    
    return task_sum;
}

/* Function with mixed OpenMP and C control flow */
void __attribute__((optimize("O0"))) complex_control_flow(double *arr, int n) {
    int thread_count = 0;
    
    /* Switch statement with OpenMP inside cases */
    switch (n % 4) {
        case 0:
            #pragma omp parallel for ordered schedule(guided)
            for (int i = 0; i < n; i++) {
                #pragma omp ordered
                arr[i] = log(fabs(arr[i]) + 1.0);
            }
            break;
            
        case 1:
            {
                /* Nested loops with OpenMP directives */
                #pragma omp target teams distribute parallel for simd \
                    map(tofrom:arr[0:n]) if(omp_get_num_devices() > 0)
                for (int i = 0; i < n; i++) {
                    arr[i] = exp(-arr[i]);
                }
            }
            break;
            
        case 2:
            /* Using _Pragma for macro expansion */
            #define OMP_PARALLEL_FOR_SCHEDULE _Pragma("omp parallel for schedule(static)")
            OMP_PARALLEL_FOR_SCHEDULE
            for (int i = 0; i < n; i++) {
                arr[i] = arr[i] * arr[i];
            }
            break;
            
        default:
            #pragma omp parallel default(none) shared(arr, n, thread_count)
            {
                #pragma omp single
                thread_count = omp_get_num_threads();
                
                #pragma omp for nowait
                for (int i = 0; i < n; i++) {
                    arr[i] = sqrt(fabs(arr[i]));
                }
            }
            break;
    }
}

/* Main function with comprehensive OpenMP usage */
int main() {
    double *data = (double*)malloc(N * sizeof(double));
    if (!data) return 1;
    
    double checksum = 0.0;
    
    /* 1. Parallel for with schedule clause */
    process_with_parallel_for(data, N);
    
    /* 2. Parallel sections with reductions */
    checksum += compute_with_sections(data, N);
    
    /* 3. Taskgroup with task_reduction */
    checksum += task_based_computation(data, N);
    
    /* 4. Complex control flow with various OpenMP constructs */
    complex_control_flow(data, N);
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum) if(N > 100)
    for (int i = 0; i < N; i++) {
        checksum += data[i];
    }
    
    /* Additional directive to ensure pretty-printing of all target clauses */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* This complex directive should trigger pretty-printing */
            #pragma omp taskloop simd in_reduction(+:checksum) \
                grainsize(16) if(checksum > 0) \
                error severity(warning) message("Finalizing: for parallel sections taskgroup")
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Using %d OpenMP threads\n", omp_get_max_threads());
    
    free(data);
    return 0;
}
