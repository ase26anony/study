/* test_omp_clauses.c
 * 
 * This test program is designed to trigger coverage of specific OpenMP clause
 * printing code in GCC's tree pretty-printer (tree-pretty-print.cc).
 * The uncovered lines handle the OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP cases.
 *
 * Compile with: gcc -O1 -fopenmp -fdump-tree-original -fdump-tree-gimple -c test_omp_clauses.c
 * Additional flags for more coverage: -fdump-tree-omplower -fdump-tree-all -Wopenmp-parsing
 */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 100
static int array[N];
static volatile int vol_bound = N; /* volatile to prevent optimization */

/* Each test function is marked noinline and cold to ensure separate
 * processing in the compiler and prevent dead code elimination.
 */

__attribute__((noinline, cold))
void test_for_clause(void)
{
    /* Use 'for' clause in a combined construct.
     * The 'for' clause appears explicitly in the internal representation
     * of this combined construct.
     */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: array[0:N]) if(0)
    for (int i = 0; i < vol_bound; ++i) {
        /* Use sin to prevent elimination */
        array[i] += (int)(sin(i * 0.1) * 10.0);
    }
}

__attribute__((noinline, cold))
void test_parallel_clause(void)
{
    /* Use 'parallel' clause in a target construct.
     * This creates OMP_CLAUSE_PARALLEL in the internal tree.
     */
    int x = 0;
    #pragma omp target parallel map(tofrom: x) if(0)
    {
        /* Potential data race to trigger diagnostic */
        x += omp_get_thread_num();
    }
    array[0] += x;
}

__attribute__((noinline, cold))
void test_sections_clause(void)
{
    /* Use 'sections' clause in a combined construct.
     * Note: The 'sections' clause is part of the combined construct
     * and will appear in the internal representation.
     */
    int a = 0, b = 0;
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b) if(0)
    {
        #pragma omp section
        {
            a = 1;
            /* Use volatile to prevent dead code elimination */
            if (vol_bound > 0) a += array[0];
        }
        #pragma omp section
        {
            b = 2;
            if (vol_bound > 0) b += array[1];
        }
    }
    array[2] = a + b;
}

__attribute__((noinline, cold))
void test_taskgroup_clause(void)
{
    /* Use 'taskgroup' clause in a taskloop construct.
     * This creates OMP_CLAUSE_TASKGROUP in the internal tree.
     */
    int sum = 0;
    #pragma omp taskloop taskgroup
    for (int i = 0; i < vol_bound; ++i) {
        /* Use rand() to prevent optimization */
        sum += rand() % 10;
    }
    #pragma omp taskwait
    array[3] = sum;
}

/* Additional test with nested taskgroup to increase chance of diagnostic */
__attribute__((noinline, cold))
void test_taskgroup_nested(void)
{
    int x = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(x)
            {
                /* Potential race condition - may trigger warning */
                x = 1;
            }
        }
    }
    array[4] = x;
}

int main(void)
{
    /* Initialize array */
    for (int i = 0; i < N; ++i) {
        array[i] = i;
    }
    
    /* Call all test functions to ensure all constructs are processed */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_taskgroup_nested();
    
    /* Compute a simple checksum to ensure execution */
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        sum += array[i];
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
