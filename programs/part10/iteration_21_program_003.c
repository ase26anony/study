/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
                        OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and ensure each construct is processed separately */
__attribute__((noinline, cold))
void test_for_clause(volatile int n, int *result) {
    /* OMP_CLAUSE_FOR: Use in combined construct with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: result[0:n]) if(n > 0) \
        num_teams(2) thread_limit(64) \
        reduction(+:result[0:n])
    for (int i = 0; i < n; i++) {
        /* Use non-trivial computation to prevent optimization */
        result[i] += (int)(sin(i * 0.1) * 100.0);
    }
}

__attribute__((noinline, cold))
void test_parallel_clause(volatile int n, int *data) {
    /* OMP_CLAUSE_PARALLEL: Explicit 'parallel' clause in target construct */
    #pragma omp target parallel map(tofrom: data[0:n]) \
        if(parallel: n > 10) num_threads(4) \
        default(shared) private(n)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic generation */
        #pragma omp critical
        data[tid % n] += tid * 2;
    }
}

__attribute__((noinline, cold))
void test_sections_clause(volatile int n, int *arr) {
    /* OMP_CLAUSE_SECTIONS: Combined construct with 'sections' clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: arr[0:n]) num_teams(2) \
        collapse(1) ordered(1)
    for (int i = 0; i < 1; i++) {  /* Dummy loop for distribute */
        #pragma omp section
        {
            arr[0] = (int)(cos(n * 0.5) * 50.0);
        }
        #pragma omp section
        {
            arr[1] = (int)(sin(n * 0.3) * 50.0);
        }
    }
}

__attribute__((noinline, cold))
void test_taskgroup_clause(volatile int n, int *sum) {
    /* OMP_CLAUSE_TASKGROUP: Taskloop with explicit taskgroup clause */
    #pragma omp taskloop taskgroup \
        grainsize(4) nogroup \
        if(taskloop: n > 5) \
        reduction(+:sum[0])
    for (int i = 0; i < n; i++) {
        /* Volatile access to prevent optimization */
        volatile int temp = i * i;
        sum[0] += temp % 100;
        
        /* Nested task with potential dependency issue for diagnostics */
        #pragma omp task untied mergeable
        {
            volatile int inner = temp / 2;
            (void)inner;
        }
    }
    
    /* Additional taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            sum[0] += 1;
        }
    }
}

/* Function with incorrect nesting to trigger diagnostic */
__attribute__((noinline, cold))
void trigger_diagnostic(volatile int n) {
    int x = 0;
    /* Incorrect: taskgroup inside parallel without taskwait - may warn */
    #pragma omp parallel shared(x)
    {
        #pragma omp taskgroup  /* This should appear in diagnostics */
        {
            #pragma omp task
            {
                x = n + 1;
            }
        }
        /* Missing taskwait here */
    }
    printf("Diagnostic test: %d\n", x);
}

int main(void) {
    volatile int N = 64;  /* Prevent constant propagation */
    int *data = (int*)calloc(N, sizeof(int));
    int *results = (int*)calloc(4, sizeof(int));
    
    if (!data || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Testing OpenMP clause coverage...\n");
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause(N, data);
    test_parallel_clause(N, data);
    test_sections_clause(N, data);
    test_taskgroup_clause(N, results);
    
    /* Trigger diagnostic path */
    trigger_diagnostic(N);
    
    /* Compute and print result to ensure execution */
    int total = 0;
    for (int i = 0; i < N; i++) {
        total += data[i];
    }
    for (int i = 0; i < 4; i++) {
        total += results[i];
    }
    
    printf("Result: %d\n", total);
    
    free(data);
    free(results);
    
    return 0;
}
