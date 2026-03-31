/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
                         OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, cold))

/* Global volatile to prevent dead code elimination */
volatile int g_volatile_bound = 100;
volatile int g_volatile_result = 0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE void test_for_clause(void) {
    int i;
    double arr[100];
    
    /* Use target teams distribute parallel for simd with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr[0:100]) if(target: g_volatile_bound > 50)
    for (i = 0; i < g_volatile_bound; i++) {
        /* Use math function to prevent optimization */
        arr[i] = sin(i * 0.01) + cos(i * 0.005);
    }
    
    /* Use result to prevent elimination */
    g_volatile_result += (int)arr[g_volatile_bound % 100];
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE void test_parallel_clause(void) {
    int local_sum = 0;
    
    /* Use target with parallel clause */
    #pragma omp target parallel reduction(+:local_sum) \
                map(tofrom: local_sum) device(0) if(parallel: g_volatile_bound > 0)
    {
        int tid = omp_get_thread_num();
        /* Volatile access to prevent optimization */
        local_sum += tid + (int)(g_volatile_bound * 0.1);
    }
    
    g_volatile_result += local_sum;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE void test_sections_clause(void) {
    int section_results[3] = {0, 0, 0};
    
    /* Use target teams with distribute parallel for sections */
    #pragma omp target teams distribute parallel for sections \
                num_teams(2) thread_limit(4) map(tofrom: section_results[0:3])
    {
        #pragma omp section
        {
            section_results[0] = 1 * (int)g_volatile_bound;
        }
        #pragma omp section
        {
            section_results[1] = 2 * (int)g_volatile_bound;
        }
        #pragma omp section
        {
            section_results[2] = 3 * (int)g_volatile_bound;
        }
    }
    
    g_volatile_result += section_results[0] + section_results[1] + section_results[2];
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE void test_taskgroup_clause(void) {
    int task_sum = 0;
    
    /* Create a taskgroup with taskloop */
    #pragma omp taskgroup task_reduction(+:task_sum)
    {
        #pragma omp taskloop grainsize(10) nogroup
        for (int i = 0; i < g_volatile_bound; i++) {
            /* Potential data race to trigger diagnostic */
            #pragma omp atomic
            task_sum += i * (int)(sin(i) * 100);
        }
    }
    
    /* Also test standalone taskgroup clause */
    #pragma omp taskgroup
    {
        #pragma omp task shared(task_sum)
        {
            #pragma omp atomic
            task_sum += (int)g_volatile_bound;
        }
    }
    
    g_volatile_result += task_sum;
}

/* Function 5: Combined test with error to trigger diagnostic */
NOINLINE void test_combined_with_warning(void) {
    int shared_var = 0;
    
    /* This may trigger a warning about shared variable in taskgroup */
    #pragma omp taskgroup
    {
        #pragma omp task shared(shared_var)
        {
            /* Potential race condition */
            shared_var++;
        }
        
        #pragma omp task shared(shared_var)
        {
            shared_var *= 2;
        }
    }
    
    g_volatile_result += shared_var;
}

int main(void) {
    /* Initialize with non-zero value */
    g_volatile_bound = 100 + (int)(rand() % 50);
    
    printf("Testing OpenMP clause coverage...\n");
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_combined_with_warning();
    
    /* Print result to ensure execution */
    printf("Final result: %d\n", g_volatile_result);
    
    return 0;
}
