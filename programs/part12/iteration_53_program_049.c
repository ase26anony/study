/* test_openmp_clauses.c
 * Generates OpenMP constructs with specific clauses to trigger
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, and
 * OMP_CLAUSE_TASKGROUP pretty-printing logic.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-original test_openmp_clauses.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function 1: Tests target teams distribute parallel for (OMP_CLAUSE_FOR) */
static void test_for_clause(void) {
    int local_sum = 0;
    
    /* Initialize array */
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* Target with teams distribute parallel for - generates OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_sum) map(to: global_array[0:N]) \
        reduction(+:local_sum) private(i)
    for (int i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
    
    printf("For clause test sum: %d\n", local_sum);
}

/* Function 2: Tests target parallel (OMP_CLAUSE_PARALLEL) */
static void test_parallel_clause(void) {
    int parallel_sum = 0;
    
    /* Target parallel - generates OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: parallel_sum) \
        map(to: global_array[0:N]) reduction(+:parallel_sum) \
        num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        int chunk = N / omp_get_num_threads();
        int start = thread_id * chunk;
        int end = (thread_id == omp_get_num_threads() - 1) ? N : start + chunk;
        
        for (int i = start; i < end; i++) {
            parallel_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += parallel_sum;
    
    printf("Parallel clause test sum: %d\n", parallel_sum);
}

/* Function 3: Tests target sections (OMP_CLAUSE_SECTIONS) */
static void test_sections_clause(void) {
    int section_sum[3] = {0, 0, 0};
    
    /* Target teams with sections - generates OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: section_sum[0:3]) \
        map(to: global_array[0:N]) num_teams(2)
    {
        #pragma omp sections reduction(+:section_sum)
        {
            #pragma omp section
            {
                for (int i = 0; i < N/3; i++) {
                    section_sum[0] += global_array[i];
                }
            }
            
            #pragma omp section
            {
                for (int i = N/3; i < 2*N/3; i++) {
                    section_sum[1] += global_array[i];
                }
            }
            
            #pragma omp section
            {
                for (int i = 2*N/3; i < N; i++) {
                    section_sum[2] += global_array[i];
                }
            }
        }
    }
    
    int total = section_sum[0] + section_sum[1] + section_sum[2];
    #pragma omp atomic
    global_sum += total;
    
    printf("Sections clause test sum: %d\n", total);
}

/* Function 4: Tests taskgroup (OMP_CLAUSE_TASKGROUP) */
static void test_taskgroup_clause(void) {
    int task_sum = 0;
    
    /* Target parallel region with taskgroup - generates OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel map(tofrom: task_sum) \
        map(to: global_array[0:N]) num_threads(2)
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                #pragma omp task in_reduction(+:task_sum)
                {
                    int local = 0;
                    for (int i = 0; i < N/2; i++) {
                        local += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local;
                }
                
                #pragma omp task in_reduction(+:task_sum)
                {
                    int local = 0;
                    for (int i = N/2; i < N; i++) {
                        local += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
    
    printf("Taskgroup clause test sum: %d\n", task_sum);
}

/* Function 5: Tests combined parallel for clause */
static void test_combined_parallel_for(void) {
    int combined_sum = 0;
    
    /* Combined parallel for - generates both OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp target parallel for map(tofrom: combined_sum) \
        map(to: global_array[0:N]) reduction(+:combined_sum) \
        private(i) schedule(static, 100)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
    
    printf("Combined parallel+for test sum: %d\n", combined_sum);
}

/* Function 6: Tests nested sections inside teams */
static void test_nested_sections(void) {
    int nested_sum = 0;
    
    /* Target teams with nested sections */
    #pragma omp target teams map(tofrom: nested_sum) \
        map(to: global_array[0:N]) num_teams(3)
    {
        int team_id = omp_get_team_num();
        
        #pragma omp sections reduction(+:nested_sum) private(i)
        {
            #pragma omp section
            {
                int start = team_id * (N/3);
                int end = (team_id == 2) ? N : start + (N/3);
                for (int i = start; i < end; i++) {
                    nested_sum += global_array[i];
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
    
    printf("Nested sections test sum: %d\n", nested_sum);
}

int main(void) {
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Initialize global array */
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        global_array[i] = (i * 3) % 97;
    }
    
    /* Test all clause types */
    test_for_clause();           /* OMP_CLAUSE_FOR */
    test_parallel_clause();      /* OMP_CLAUSE_PARALLEL */
    test_sections_clause();      /* OMP_CLAUSE_SECTIONS */
    test_taskgroup_clause();     /* OMP_CLAUSE_TASKGROUP */
    test_combined_parallel_for(); /* Both PARALLEL and FOR */
    test_nested_sections();      /* Nested SECTIONS */
    
    printf("\nFinal global sum: %d\n", global_sum);
    printf("Expected sum: %d\n", 6 * (N/2 * 98));  /* Rough estimate */
    
    return 0;
}
