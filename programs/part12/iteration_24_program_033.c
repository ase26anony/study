/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * pretty-printer coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;  /* Prevent optimization */

/* Test OMP_CLAUSE_FOR - Pattern A with combined directive */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Combined parallel for with schedule clause */
    #pragma omp parallel for schedule(static) private(i) shared(sum) firstprivate(local_volatile)
    for (i = 0; i < 100; i++) {
        sum += i;
        local_volatile = i;  /* Use volatile to prevent optimization */
    }
    
    printf("test_parallel_for: sum = %d\n", sum);
    #pragma omp atomic
    global_counter += sum;
}

/* Test OMP_CLAUSE_PARALLEL - Pattern B */
void test_parallel(void) {
    int local_sum = 0;
    
    /* Standalone parallel region with data-sharing clauses */
    #pragma omp parallel shared(local_sum) default(none)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        local_sum += tid;
    }
    
    printf("test_parallel: thread sum = %d\n", local_sum);
    #pragma omp atomic
    global_counter += local_sum;
}

/* Test OMP_CLAUSE_SECTIONS - Pattern C */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    
    /* Combined parallel sections */
    #pragma omp parallel sections private(section1_result, section2_result) \
        shared(global_counter)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            printf("Section 1: %d\n", section1_result);
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            printf("Section 2: %d\n", section2_result);
        }
    }
    
    #pragma omp atomic
    global_counter += (section1_result + section2_result);
}

/* Test OMP_CLAUSE_TASKGROUP - Pattern D */
void test_taskgroup(void) {
    int task_sum = 0;
    
    /* Nested: parallel region with master construct containing taskgroup */
    #pragma omp parallel
    {
        #pragma omp master
        {
            /* Taskgroup with task dependences */
            #pragma omp taskgroup
            {
                #pragma omp task shared(task_sum) 
                {
                    #pragma omp atomic
                    task_sum += 1;
                }
                
                #pragma omp task shared(task_sum)
                {
                    #pragma omp atomic
                    task_sum += 2;
                }
                
                /* Wait for all tasks in the group */
                #pragma omp taskwait
            }
        }
    }
    
    printf("test_taskgroup: task sum = %d\n", task_sum);
    #pragma omp atomic
    global_counter += task_sum;
}

/* Additional test with nested constructs for deeper tree traversal */
void test_nested_constructs(void) {
    int nested_sum = 0;
    
    /* Parallel region containing multiple constructs */
    #pragma omp parallel shared(nested_sum)
    {
        /* For construct inside parallel */
        #pragma omp for schedule(dynamic) nowait
        for (int i = 0; i < 100; i++) {
            #pragma omp atomic
            nested_sum += i;
        }
        
        /* Sections inside the same parallel region */
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp atomic
                nested_sum += 1000;
            }
            #pragma omp section
            {
                #pragma omp atomic
                nested_sum += 2000;
            }
        }
    }
    
    printf("test_nested_constructs: nested sum = %d\n", nested_sum);
    #pragma omp atomic
    global_counter += nested_sum;
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize global counter */
    global_counter = 0;
    
    /* Test each clause in separate functions */
    test_parallel_for();      /* Triggers OMP_CLAUSE_FOR */
    test_parallel();          /* Triggers OMP_CLAUSE_PARALLEL */
    test_sections();          /* Triggers OMP_CLAUSE_SECTIONS */
    test_taskgroup();         /* Triggers OMP_CLAUSE_TASKGROUP */
    test_nested_constructs(); /* Additional coverage with nesting */
    
    /* Final verification */
    printf("\nAll tests completed.\n");
    printf("Final global counter value: %d\n", global_counter);
    
    if (global_counter > 0) {
        printf("SUCCESS: All OpenMP constructs executed.\n");
        return 0;
    } else {
        printf("ERROR: No OpenMP work performed.\n");
        return 1;
    }
}
