/* test_openmp_clauses.c - Targeting uncovered pretty-print cases in tree-pretty-print.cc */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function attribute to prevent optimization from removing OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(double *data, int n, double *results) {
    double sum = 0.0;
    double max_val = -INFINITY;
    
    /* TARGET 1: OMP_CLAUSE_FOR - in combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(sum) shared(data, results) if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            data[i] = sin(i * 0.01) * cos(j * 0.01);
        }
        sum += data[i];
    }
    
    results[0] = sum;
    
    /* TARGET 2: OMP_CLAUSE_PARALLEL - combined with sections */
    #pragma omp parallel sections private(max_val) shared(data, results) \
        num_threads(4) if(omp_get_num_procs() > 2)
    {
        /* TARGET 3: OMP_CLAUSE_SECTIONS - with multiple section blocks */
        #pragma omp section
        {
            max_val = data[0];
            for (int i = 1; i < n/2; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
            results[1] = max_val;
        }
        
        #pragma omp section
        {
            double local_sum = 0.0;
            for (int i = n/2; i < n; i++) {
                local_sum += data[i] * data[i];
            }
            results[2] = sqrt(local_sum);
        }
    }
}

/* Another function to test taskgroup clause */
void __attribute__((optimize("O0"), noinline))
taskgroup_example(double *data, int n, double *result) {
    double task_sum = 0.0;
    
    /* TARGET 4: OMP_CLAUSE_TASKGROUP - with task_reduction clause */
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            for (int i = 0; i < n; i += n/10) {
                #pragma omp task in_reduction(+:task_sum) firstprivate(i) \
                    shared(data, n)
                {
                    double local = 0.0;
                    int end = (i + n/10) < n ? (i + n/10) : n;
                    for (int j = i; j < end; j++) {
                        local += data[j];
                    }
                    task_sum += local;
                    
                    /* Force diagnostic with clause name in message */
                    if (local > 1000.0) {
                        /* This should trigger pretty-printing of clauses */
                        #pragma omp error message("Task reduction with 'for' clause would be useful here")
                    }
                }
            }
        }
    }
    
    *result = task_sum;
}

/* Complex nested function with macro expansion for clause printing */
#define CREATE_PARALLEL_REGION(clause_name) \
    _Pragma("omp parallel " #clause_name " if(1)") \
    { \
        int tid = omp_get_thread_num(); \
        printf("Thread %d in " #clause_name " region\n", tid); \
    }

void __attribute__((optimize("O0")))
test_macro_expansion() {
    /* These macro expansions create complex patterns for the pretty-printer */
    CREATE_PARALLEL_REGION(for)
    CREATE_PARALLEL_REGION(sections)
    
    /* Direct _Pragma usage with taskgroup */
    _Pragma("omp taskgroup task_reduction(+:x)")
    {
        double x = 0.0;
        _Pragma("omp task in_reduction(+:x)")
        {
            x += 1.0;
        }
    }
}

/* Function with switch statement containing OpenMP directives */
void __attribute__((optimize("O0")))
openmp_in_switch(int mode, double *data, int n, double *output) {
    switch (mode) {
        case 1: {
            /* Nested parallel for inside switch case */
            #pragma omp parallel for schedule(dynamic) \
                if(n > 500) num_threads(2)
            for (int i = 0; i < n; i++) {
                data[i] = data[i] * 2.0;
            }
            *output = data[0];
            break;
        }
        case 2: {
            /* Parallel sections inside switch case */
            #pragma omp parallel sections
            {
                #pragma omp section
                { *output = 0.0; }
                #pragma omp section
                { *output = 1.0; }
            }
            break;
        }
        default: {
            /* Taskgroup inside default case */
            #pragma omp taskgroup
            {
                #pragma omp task
                { *output = -1.0; }
            }
            break;
        }
    }
}

int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double results[5] = {0.0};
    double task_result = 0.0;
    double switch_output = 0.0;
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some values */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (double)i / ARRAY_SIZE;
    }
    
    /* Test all targeted OpenMP clause patterns */
    process_with_openmp(data, ARRAY_SIZE, results);
    
    taskgroup_example(data, ARRAY_SIZE, &task_result);
    
    test_macro_expansion();
    
    /* Test with different switch cases */
    for (int mode = 1; mode <= 3; mode++) {
        openmp_in_switch(mode, data, ARRAY_SIZE, &switch_output);
        results[3] += switch_output;
    }
    
    /* Additional complex directive combining multiple clauses */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: data[0:ARRAY_SIZE]) if(ARRAY_SIZE > 1000) \
        num_teams(2) thread_limit(64)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = exp(-data[i] * data[i]);
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) \
        schedule(guided) if(omp_get_max_threads() > 1)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += data[i];
    }
    
    checksum += results[0] + results[1] + results[2] + task_result;
    
    printf("Final checksum: %f\n", checksum);
    printf("Results: %f %f %f %f\n", results[0], results[1], results[2], results[3]);
    
    /* Force error diagnostic with clause names */
    #pragma omp error severity(warning) message("Testing pretty-print of clauses: for, parallel, sections, taskgroup")
    
    free(data);
    return 0;
}
