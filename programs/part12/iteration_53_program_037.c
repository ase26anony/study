/* test_openmp_clauses.c
 * Generates OpenMP constructs to trigger OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP pretty-printing logic.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for \
        reduction(+:local_sum) map(tofrom:local_sum) \
        private(i) shared(global_array)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel clause */
static void test_target_parallel(void)
{
    int i;
    int local_sum = 0;
    
    /* This generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel private(i) reduction(+:local_sum) \
        map(tofrom:local_sum) shared(global_array)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i] * 2;
        }
        
        /* Nested taskgroup inside target parallel region */
        /* This generates OMP_CLAUSE_TASKGROUP */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                int temp = local_sum;
                temp += 1;  /* Simple task work */
                #pragma omp atomic
                local_sum += temp;
            }
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target sections clause */
static void test_target_sections(void)
{
    int section1_sum = 0, section2_sum = 0;
    
    /* This generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom:section1_sum, section2_sum)
    {
        #pragma omp sections private(global_array)
        {
            #pragma omp section
            {
                for (int i = 0; i < N/2; i++) {
                    section1_sum += global_array[i];
                }
            }
            
            #pragma omp section
            {
                for (int i = N/2; i < N; i++) {
                    section2_sum += global_array[i];
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section1_sum + section2_sum;
}

/* Function combining multiple clauses */
static void test_combined_clauses(void)
{
    int i;
    int combined_sum = 0;
    
    /* Combined parallel and for clauses in single pragma */
    /* Generates both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp target parallel for private(i) \
        reduction(+:combined_sum) map(tofrom:combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    /* Taskgroup with nested tasks */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup  /* Generates OMP_CLAUSE_TASKGROUP */
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    combined_sum += 1;
                }
                
                #pragma omp task
                {
                    #pragma omp atomic
                    combined_sum += 2;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

int main(void)
{
    /* Initialize array with predictable values */
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clauses for pretty-printer coverage...\n");
    
    /* Test each clause in different contexts */
    test_target_parallel_for();      /* for clause */
    test_target_parallel();          /* parallel and taskgroup clauses */
    test_target_sections();          /* sections clause */
    test_combined_clauses();         /* combined clauses */
    
    /* Additional direct usage in main for extra coverage */
    int main_local = 0;
    
    /* Target parallel for in main */
    #pragma omp target parallel for reduction(+:main_local) \
        private(global_array)
    for (int i = 0; i < N; i++) {
        main_local += global_array[i];
    }
    
    /* Target sections in main */
    int sec1 = 0, sec2 = 0;
    #pragma omp target sections map(tofrom:sec1, sec2)
    {
        #pragma omp section
        {
            for (int i = 0; i < N/2; i++) sec1 += global_array[i];
        }
        #pragma omp section
        {
            for (int i = N/2; i < N; i++) sec2 += global_array[i];
        }
    }
    
    global_sum += main_local + sec1 + sec2;
    
    printf("Final sum: %d\n", global_sum);
    printf("Expected sum: %d\n", (N/2)*(99) * 8 + 3);  /* Formula for verification */
    
    return 0;
}
