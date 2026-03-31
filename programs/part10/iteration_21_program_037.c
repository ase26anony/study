/* test_omp_clauses.c */
#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, cold))

/* Global volatile to prevent dead code elimination */
volatile int g_volatile = 100;
volatile int g_result = 0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE void test_for_clause() {
    int i;
    double arr[100];
    
    /* Use target teams distribute parallel for simd with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:100]) if(g_volatile > 50)
    for (i = 0; i < 100; i++) {
        arr[i] = sin(i * 0.1) * cos(i * 0.05);
    }
    
    /* Use result to prevent optimization */
    g_result += (int)arr[g_volatile % 100];
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE void test_parallel_clause() {
    int x = 0;
    
    /* Use target parallel with explicit 'parallel' clause */
    #pragma omp target parallel map(tofrom: x) if(g_volatile > 0) \
        num_threads(2)
    {
        x = omp_get_thread_num() + 1;
    }
    
    g_result += x;
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE void test_sections_clause() {
    int a = 0, b = 0;
    
    /* Use target teams with sections clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b) num_teams(2)
    {
        #pragma omp section
        {
            a = rand() % 100;
        }
        #pragma omp section
        {
            b = rand() % 100;
        }
    }
    
    g_result += a + b;
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE void test_taskgroup_clause() {
    int sum = 0;
    
    /* Use taskloop with taskgroup clause - this should generate OMP_CLAUSE_TASKGROUP */
    #pragma omp taskloop taskgroup num_tasks(10)
    for (int i = 0; i < 100; i++) {
        #pragma omp atomic
        sum += i;
    }
    
    /* Also test standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            sum += g_volatile;
        }
    }
    
    g_result += sum;
}

/* Additional test with combined construct to ensure all clauses are processed */
NOINLINE void test_combined() {
    int arr[50];
    
    /* Complex construct that may trigger multiple clause processings */
    #pragma omp target teams distribute parallel for simd \
        if(g_volatile > 25) num_teams(2) thread_limit(4) \
        map(tofrom: arr[0:50])
    for (int i = 0; i < 50; i++) {
        arr[i] = i * i;
    }
    
    /* Force potential diagnostic: data race scenario */
    int shared_var = 0;
    #pragma omp parallel shared(shared_var)
    {
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            /* Potential race condition to trigger warning */
            shared_var += i;  /* This may trigger -Wopenmp-parsing warnings */
        }
    }
    
    g_result += arr[g_volatile % 50] + shared_var;
}

int main() {
    /* Initialize random seed */
    srand(42);
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_combined();
    
    /* Print result to ensure execution */
    printf("Result: %d\n", g_result);
    
    return 0;
}
