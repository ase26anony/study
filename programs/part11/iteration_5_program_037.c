/* test_omp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define N 10000
#define CHUNK_SIZE 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_parallel_for(double *arr) {
    int i;
    /* Combined parallel and for clause - will trigger both pretty-print cases */
    #pragma omp parallel for schedule(dynamic, CHUNK_SIZE) private(i) \
        if(N > 1000)
    for (i = 0; i < N; i++) {
        arr[i] = sin(i * 0.01) * cos(i * 0.005);
    }
    
    /* Complex for clause with multiple arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        aligned(arr:64) linear(i:1)
    for (i = 0; i < sqrt(N); i++) {
        for (int j = 0; j < sqrt(N); j++) {
            int idx = i * (int)sqrt(N) + j;
            if (idx < N) {
                arr[idx] *= 1.0001;
            }
        }
    }
}

/* Function using sections clause */
double __attribute__((optimize("O0"))) compute_with_sections(double *arr) {
    double sum = 0.0, max_val = -1e30;
    
    /* Combined parallel and sections clause */
    #pragma omp parallel sections reduction(+:sum) reduction(max:max_val) \
        num_threads(4)
    {
        /* First section */
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) {
                sum += arr[i];
            }
            /* Nested directive inside section */
            #pragma omp parallel for simd
            for (int i = 0; i < N/4; i++) {
                arr[i] = fabs(arr[i]);
            }
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = N/2; i < N; i++) {
                if (arr[i] > max_val) {
                    max_val = arr[i];
                }
            }
            /* Trigger diagnostic with sections clause name */
            #pragma omp error severity(warning) message("Processing sections clause")
        }
    }
    
    return sum + max_val;
}

/* Complex function with taskgroup clause */
double __attribute__((optimize("O0"))) process_with_taskgroup(double *arr) {
    double total = 0.0;
    int i;
    
    /* Taskgroup with explicit task_reduction clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:total)
            {
                for (i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:total) firstprivate(i) \
                        depend(out: arr[i*1000])
                    {
                        double local_sum = 0.0;
                        int start = i * 1000;
                        int end = (i + 1) * 1000;
                        if (end > N) end = N;
                        
                        for (int j = start; j < end; j++) {
                            local_sum += arr[j] * arr[j];
                        }
                        total += sqrt(local_sum);
                        
                        /* Nested directive inside task */
                        #pragma omp parallel for
                        for (int j = start; j < end; j += 100) {
                            arr[j] = local_sum / (end - start);
                        }
                    }
                }
            }
        }
    }
    
    /* Trigger diagnostic with taskgroup clause name using _Pragma */
    #define EMIT_TASKGROUP_WARNING _Pragma("omp error severity(message) message(\"taskgroup clause processed\")")
    EMIT_TASKGROUP_WARNING;
    
    return total;
}

/* Function with mixed control flow and OpenMP */
void __attribute__((optimize("O0"))) complex_control_flow(double *arr) {
    int mode = omp_get_thread_num() % 3;
    
    switch (mode) {
        case 0: {
            /* For clause in switch case */
            #pragma omp for nowait
            for (int i = 0; i < N; i += 2) {
                arr[i] = arr[i] * 2.0;
            }
            break;
        }
        case 1: {
            /* Sections in switch case */
            #pragma omp sections
            {
                #pragma omp section
                { arr[0] = 1.0; }
                #pragma omp section  
                { arr[1] = 2.0; }
            }
            break;
        }
        case 2: {
            /* Trigger diagnostic with for clause name */
            #pragma omp error severity(warning) message("for clause in switch case")
            break;
        }
    }
}

/* Main function with execution flow as specified */
int main() {
    double *array = (double*)malloc(N * sizeof(double));
    if (!array) return 1;
    
    double checksum = 0.0;
    
    /* 1. Parallel for initialization */
    #pragma omp parallel for schedule(dynamic) shared(array) \
        if(N > 500) default(none)
    for (int i = 0; i < N; i++) {
        array[i] = i * 0.001;
    }
    
    /* Process with parallel for (triggers for and parallel clauses) */
    process_with_parallel_for(array);
    
    /* 2. Parallel sections with reductions */
    double section_result = compute_with_sections(array);
    checksum += section_result;
    
    /* 3. Taskgroup with task_reduction */
    double task_result = process_with_taskgroup(array);
    checksum += task_result;
    
    /* 4. Complex control flow with mixed directives */
    #pragma omp parallel
    {
        complex_control_flow(array);
    }
    
    /* Final computation to prevent dead code elimination */
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < N; i++) {
        checksum += array[i];
    }
    
    /* Use runtime API calls */
    int num_threads = omp_get_max_threads();
    checksum /= num_threads;
    
    printf("Final checksum: %f\n", checksum);
    printf("Using %d OpenMP threads\n", num_threads);
    
    free(array);
    return 0;
}
