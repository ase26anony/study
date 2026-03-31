/* test_omp_clauses.c - Target coverage for tree-pretty-print.cc lines 1434-1445 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define SIZE 10000
#define CHUNK 64

/* Function attribute to prevent optimization */
void __attribute__((optimize("O0"), noinline)) 
process_with_parallel_for(double *arr, int n) {
    int i;
    
    /* Combined parallel and for clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for schedule(dynamic, CHUNK) private(i) \
        if(n > 1000) num_threads(omp_get_max_threads())
    for (i = 0; i < n; i++) {
        arr[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Complex for clause with multiple arguments */
    #pragma omp target teams distribute parallel for simd \
        schedule(static, 4) collapse(2) map(tofrom: arr[0:n])
    for (i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            arr[i*2 + j] *= 1.1;
        }
    }
}

void __attribute__((optimize("O0"), noinline))
process_with_parallel_sections(double *arr, int n, double *sum, double *max) {
    *sum = 0.0;
    *max = -1e30;
    
    /* Combined parallel and sections clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections reduction(+:*sum) reduction(max:*max) \
        if(n > 500)
    {
        /* First section */
        #pragma omp section
        {
            double local_sum = 0.0;
            for (int i = 0; i < n/2; i++) {
                local_sum += arr[i];
            }
            *sum = local_sum;
        }
        
        /* Second section */
        #pragma omp section
        {
            double local_max = -1e30;
            for (int i = n/2; i < n; i++) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            *max = local_max;
        }
        
        /* Third section with nested directive */
        #pragma omp section
        {
            /* Nested for clause inside sections */
            #pragma omp parallel for simd schedule(guided)
            for (int i = 0; i < n; i += 10) {
                arr[i] = sqrt(fabs(arr[i]));
            }
        }
    }
}

void __attribute__((optimize("O0"), noinline))
process_with_taskgroup(double *arr, int n, double *result) {
    double sum = 0.0;
    int count = 0;
    
    /* Taskgroup clause with task_reduction argument - triggers OMP_CLAUSE_TASKGROUP */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:sum, count)
        {
            for (int i = 0; i < n; i += n/8) {
                int start = i;
                int end = (i + n/8 < n) ? i + n/8 : n;
                
                #pragma omp task in_reduction(+:sum, count) \
                    firstprivate(start, end, arr)
                {
                    double local_sum = 0.0;
                    int local_count = 0;
                    for (int j = start; j < end; j++) {
                        if (arr[j] > 0.5) {
                            local_sum += arr[j];
                            local_count++;
                        }
                    }
                    sum += local_sum;
                    count += local_count;
                }
            }
            
            /* Force diagnostic with clause name in message */
            #pragma omp task
            {
                /* This will trigger pretty-printing of the 'for' clause */
                #pragma omp error message("Processing with for clause in task") severity(warning)
                
                /* Complex macro expansion with _Pragma */
                #define EMIT_FOR_CLAUSE _Pragma("omp parallel for schedule(static)")
                EMIT_FOR_CLAUSE
                for (int i = 0; i < 10; i++) {
                    arr[i % n] += 0.001;
                }
            }
        }
    }
    
    *result = (count > 0) ? sum / count : 0.0;
}

/* Function with mixed control flow and OpenMP */
void __attribute__((optimize("O0")))
complex_control_flow(double *arr, int n) {
    int mode = 3;
    
    switch (mode) {
        case 1: {
            /* For clause in switch case */
            #pragma omp for schedule(runtime) nowait
            for (int i = 0; i < n; i++) {
                arr[i] = 1.0;
            }
            break;
        }
        case 2: {
            /* Sections clause in switch case */
            #pragma omp sections
            {
                #pragma omp section
                { arr[0] = 2.0; }
                #pragma omp section  
                { arr[1] = 3.0; }
            }
            break;
        }
        case 3: {
            /* Nested loops with OpenMP directives */
            for (int outer = 0; outer < 5; outer++) {
                if (outer % 2 == 0) {
                    /* Parallel for inside if statement */
                    #pragma omp parallel for ordered
                    for (int i = 0; i < n/5; i++) {
                        #pragma omp ordered
                        arr[outer * (n/5) + i] += outer * 0.1;
                    }
                } else {
                    /* Taskgroup inside else branch */
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        { arr[outer] *= 0.9; }
                    }
                }
            }
            break;
        }
    }
}

int main() {
    double *array = (double*)malloc(SIZE * sizeof(double));
    double sum, max, task_result;
    double checksum = 0.0;
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize OpenMP */
    omp_set_dynamic(0);
    omp_set_num_threads(4);
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* 1. Use parallel for clause */
    process_with_parallel_for(array, SIZE);
    
    /* 2. Use parallel sections clause */
    process_with_parallel_sections(array, SIZE, &sum, &max);
    checksum += sum + max;
    
    /* 3. Use taskgroup clause */
    process_with_taskgroup(array, SIZE, &task_result);
    checksum += task_result;
    
    /* 4. Mixed control flow with various clauses */
    complex_control_flow(array, SIZE);
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < SIZE; i++) {
        checksum += array[i] * 0.0001;
    }
    
    /* Additional diagnostic trigger */
    #ifdef _OPENMP
    #pragma omp error message("Final check: for, parallel, sections, taskgroup clauses processed") severity(message)
    #endif
    
    printf("Final checksum: %f\n", checksum);
    
    free(array);
    return 0;
}
