/* test_omp_clauses.c
 * 
 * This test program is designed to trigger the uncovered lines in
 * tree-pretty-print.cc (lines 1434-1445) by using OpenMP constructs
 * with the specific clause types: for, parallel, sections, and taskgroup.
 * The coverage is achieved through compiler tree dumps (-fdump-tree-*)
 * or diagnostic messages during compilation.
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Use volatile variables to prevent optimization removal */
volatile int vol_bound = 100;
volatile int vol_idx = 0;

/* External function call to prevent dead code elimination */
extern double sin(double);

/* Each test function is marked noinline and cold to ensure
 * individual processing and prevent inlining */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    double arr[100];
    
    /* OMP_CLAUSE_FOR: for clause in combined construct */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr[0:100]) if(vol_bound > 50)
    for (i = 0; i < vol_bound; i++) {
        arr[i] = sin(i * 0.1);
    }
    
    /* Use result to prevent elimination */
    vol_idx = (int)arr[vol_bound-1];
}

__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int x = 0;
    
    /* OMP_CLAUSE_PARALLEL: parallel clause in target construct */
    #pragma omp target parallel if(vol_bound > 0) \
                map(tofrom: x) reduction(+:x)
    {
        x += omp_get_thread_num() + 1;
    }
    
    vol_idx += x;
}

__attribute__((noinline, cold))
void test_sections_clause(void) {
    int a = 0, b = 0;
    
    /* OMP_CLAUSE_SECTIONS: sections clause in combined construct */
    #pragma omp target teams distribute parallel for sections \
                map(tofrom: a, b) num_teams(2)
    {
        #pragma omp section
        {
            a = vol_bound * 2;
        }
        #pragma omp section
        {
            b = vol_bound / 2;
        }
    }
    
    vol_idx += a + b;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int sum = 0;
    int i;
    
    /* OMP_CLAUSE_TASKGROUP: taskgroup clause in taskloop construct */
    #pragma omp taskloop taskgroup reduction(+:sum) \
                grainsize(10) num_tasks(5)
    for (i = 0; i < vol_bound; i++) {
        sum += i;
    }
    
    /* Additional taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            sum += vol_idx;
        }
    }
    
    vol_idx = sum % 100;
}

int main(void) {
    int result = 0;
    
    /* Initialize with non-zero value */
    vol_bound = 100;
    vol_idx = 1;
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    result += vol_idx;
    
    test_parallel_clause();
    result += vol_idx;
    
    test_sections_clause();
    result += vol_idx;
    
    test_taskgroup_clause();
    result += vol_idx;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    
    return 0;
}
