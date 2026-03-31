/* test-openmp-clauses.c
 * 
 * This program is designed to generate OpenMP AST nodes for the clauses:
 *   OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 * which are printed by the pretty-printer in tree-pretty-print.cc (lines 1434-1445).
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test-openmp-clauses.c
 * Additional dump flags: -fdump-tree-all, -fdump-tree-gimple
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test 'for' and 'parallel' clauses in combination */
static void test_for_and_parallel(void) {
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL nodes */
    #pragma omp target teams distribute parallel for \
                reduction(+:local_sum) map(tofrom:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Also test combined parallel for */
    #pragma omp target parallel for reduction(+:local_sum) \
                private(i) map(tofrom:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test 'sections' clause */
static void test_sections(void) {
    int section_a = 0, section_b = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS node */
    #pragma omp target teams sections reduction(+:section_a, section_b) \
                map(tofrom:section_a, section_b)
    {
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) {
                section_a += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = N/2; i < N; i++) {
                section_b += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section_a + section_b;
}

/* Function to test 'taskgroup' clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Outer parallel region */
    #pragma omp target parallel map(tofrom:task_sum)
    {
        /* This generates OMP_CLAUSE_TASKGROUP node */
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum)
            {
                int local = 0;
                for (int i = 0; i < N; i++) {
                    local += global_array[i] % 10;
                }
                #pragma omp atomic
                task_sum += local;
            }
            
            #pragma omp task shared(task_sum)
            {
                int local = 0;
                for (int i = 0; i < N; i++) {
                    local += global_array[i] % 5;
                }
                #pragma omp atomic
                task_sum += local;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Main function with mixed constructs */
int main(void) {
    int i;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = (i + 1) % 100;
    }
    
    /* Test 1: for and parallel clauses */
    test_for_and_parallel();
    
    /* Test 2: sections clause */
    test_sections();
    
    /* Test 3: taskgroup clause */
    test_taskgroup();
    
    /* Additional nested test in main */
    int extra_sum = 0;
    
    /* Combined parallel for with sections */
    #pragma omp target parallel map(tofrom:extra_sum)
    {
        #pragma omp for reduction(+:extra_sum) private(i)
        for (i = 0; i < N; i++) {
            extra_sum += global_array[i] * 3;
        }
        
        #pragma omp sections reduction(+:extra_sum)
        {
            #pragma omp section
            { extra_sum += 1; }
            
            #pragma omp section
            { extra_sum += 2; }
        }
        
        #pragma omp taskgroup
        {
            #pragma omp task shared(extra_sum)
            { extra_sum += 10; }
        }
    }
    
    global_sum += extra_sum;
    
    printf("Final sum: %d\n", global_sum);
    
    /* Verification */
    int expected = 0;
    for (i = 0; i < N; i++) {
        expected += global_array[i] * 6;  /* from test_for_and_parallel */
        expected += global_array[i];      /* from test_sections */
        expected += (global_array[i] % 10) + (global_array[i] % 5); /* from test_taskgroup */
        expected += global_array[i] * 3;  /* from extra_sum in main */
    }
    expected += 13;  /* from extra_sum constants (1+2+10) */
    
    if (global_sum == expected) {
        printf("Verification passed!\n");
        return 0;
    } else {
        printf("Verification failed! Expected: %d, Got: %d\n", expected, global_sum);
        return 1;
    }
}
