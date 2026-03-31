/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, PARALLEL, SECTIONS, TASKGROUP */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 100
volatile int g_volatile = 100;

/* Prevent optimization and ensure each construct is processed separately */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    float arr[SIZE];
    
    /* OMP_CLAUSE_FOR: for clause in combined construct */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:SIZE]) if(g_volatile > 50)
    for (i = 0; i < SIZE; i++) {
        arr[i] = sin(i * 0.1f) * cos(i * 0.05f);
    }
    
    /* Use result to prevent dead code elimination */
    volatile float sum = 0.0f;
    for (i = 0; i < SIZE; i++) {
        sum += arr[i];
    }
    (void)sum;
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int shared_var = 0;
    
    /* OMP_CLAUSE_PARALLEL: parallel clause in target construct */
    #pragma omp target parallel map(tofrom: shared_var) \
        if(g_volatile > 75) num_threads(2)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        shared_var += tid + 1;
        
        /* Force some computation */
        volatile double x = tid * 3.14159;
        (void)x;
    }
    
    /* Potential data race warning trigger */
    volatile int check = shared_var;
    (void)check;
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int a = 0, b = 0, c = 0;
    
    /* OMP_CLAUSE_SECTIONS: sections clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b, c) private(g_volatile)
    {
        #pragma omp section
        {
            a = omp_get_team_num() + 1;
            volatile float t = sin(a * 0.5f);
            (void)t;
        }
        #pragma omp section
        {
            b = omp_get_num_teams() * 2;
            volatile float t = cos(b * 0.3f);
            (void)t;
        }
        #pragma omp section
        {
            c = omp_get_thread_num() * 3;
            volatile float t = tan(c * 0.1f);
            (void)t;
        }
    }
    
    /* Use results */
    volatile int total = a + b + c;
    (void)total;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup clause in taskloop */
    #pragma omp taskloop taskgroup reduction(+:sum) \
        nogroup if(g_volatile < 200)
    for (int i = 0; i < 50; i++) {
        sum += i * (omp_get_thread_num() + 1);
        
        /* Force computation */
        volatile double y = log(i + 1.0);
        (void)y;
    }
    
    /* Complete taskgroup */
    #pragma omp taskwait
    
    /* Potential diagnostic: unused variable if sum not used */
    volatile int result = sum;
    (void)result;
}

/* Additional test with standalone taskgroup */
__attribute__((noinline, cold))
void test_standalone_taskgroup(void) {
    int x = 0;
    
    /* Standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task shared(x)
        {
            #pragma omp atomic
            x += 1;
        }
        
        #pragma omp task shared(x)
        {
            #pragma omp atomic
            x += 2;
        }
    }
    
    volatile int final_x = x;
    (void)final_x;
}

int main(void) {
    int total = 0;
    
    /* Initialize volatile for conditionals */
    g_volatile = 150;
    
    /* Call all test functions */
    test_for_clause();
    total += 1;
    
    test_parallel_clause();
    total += 2;
    
    test_sections_clause();
    total += 3;
    
    test_taskgroup_clause();
    total += 4;
    
    test_standalone_taskgroup();
    total += 5;
    
    /* Print result to ensure execution */
    printf("Test completed with code: %d\n", total);
    
    /* Trigger potential format string analysis */
    volatile const char *msg = "OpenMP clauses: for, parallel, sections, taskgroup";
    __builtin_printf("%s\n", msg);
    
    return total > 10 ? 0 : 1;
}
