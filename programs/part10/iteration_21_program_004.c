/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
                         OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, cold))
#define VOLATILE volatile

/* Global array to prevent dead code elimination */
static VOLATILE int global_array[1024] = {0};

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE void test_for_clause(void) {
    int i;
    VOLATILE int n = 100;
    
    /* Use for clause in combined construct - will generate OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: global_array[0:n]) \
        num_teams(2) thread_limit(64)
    for (i = 0; i < n; i++) {
        /* Use math function to prevent optimization */
        global_array[i] += (int)sin(i * 0.1) * 10;
    }
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE void test_parallel_clause(void) {
    VOLATILE int x = 0;
    
    /* Use parallel clause with target - will generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: x) \
        if(target: 1) num_threads(4)
    {
        /* Introduce potential data race for diagnostic generation */
        #pragma omp atomic
        x += omp_get_thread_num() + 1;
        
        /* Use external function call */
        (void)rand();
    }
    
    /* Use result to prevent dead code elimination */
    global_array[0] = x;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE void test_sections_clause(void) {
    VOLATILE int a = 0, b = 0, c = 0;
    
    /* Use sections clause in combined construct - will generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b, c) \
        num_teams(2)
    {
        #pragma omp section
        {
            a = 1;
            /* Use math function */
            (void)cos(0.5);
        }
        #pragma omp section
        {
            b = 2;
        }
        #pragma omp section
        {
            c = 3;
        }
    }
    
    /* Use results */
    global_array[1] = a + b + c;
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE void test_taskgroup_clause(void) {
    VOLATILE int sum = 0;
    int i;
    
    /* Use taskgroup clause with taskloop - will generate OMP_CLAUSE_TASKGROUP */
    #pragma omp taskgroup task_reduction(+:sum)
    {
        #pragma omp taskloop grainsize(10) \
            in_reduction(+:sum) nogroup
        for (i = 0; i < 100; i++) {
            /* Introduce potential issue for diagnostics */
            VOLATILE int* ptr = &global_array[i % 1024];
            sum += *ptr + i;
        }
    }
    
    /* Alternative: Direct taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            global_array[10] = 999;
        }
        #pragma omp task
        {
            global_array[11] = 888;
        }
    }
    
    global_array[2] = sum;
}

/* Additional function with error to trigger diagnostics */
NOINLINE void trigger_diagnostic(void) {
    VOLATILE int problematic = 0;
    
    /* This may trigger OpenMP parsing warnings */
    #pragma omp target parallel map(tofrom: problematic)
    {
        /* Potential data race - may generate diagnostic */
        problematic += omp_get_thread_num();
        
        /* No flush - might trigger warning in some configurations */
        #pragma omp task
        {
            problematic = 1;
        }
    }
}

int main(void) {
    VOLATILE int seed = 42;
    
    /* Initialize random seed */
    srand(seed);
    
    printf("Testing OpenMP clause coverage...\n");
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    trigger_diagnostic();
    
    /* Compute and print result to ensure execution */
    int total = 0;
    for (int i = 0; i < 1024; i++) {
        total += global_array[i];
    }
    
    printf("Result: %d\n", total);
    printf("OpenMP max threads: %d\n", omp_get_max_threads());
    
    return 0;
}
