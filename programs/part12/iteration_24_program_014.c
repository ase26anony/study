/* test_omp_clauses.c
 * This program exercises specific OpenMP clauses to trigger
 * the pretty-printer logic for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL,
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP.
 * Compile with: gcc -O1 -fopenmp -fdump-tree-all test_omp_clauses.c -o test_omp_clauses
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR clause */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Direct usage of 'for' clause in combined construct */
    #pragma omp parallel for private(i) shared(sum) schedule(static) num_threads(2)
    for (i = 0; i < 100; i++) {
        sum += i;
        local_volatile = i;  /* Prevent optimization */
    }
    
    /* Additional 'for' clause in worksharing context */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            #pragma omp atomic
            global_counter++;
        }
    }
    
    printf("test_parallel_for: sum = %d, global_counter = %d\n", sum, global_counter);
}

/* Test OMP_CLAUSE_PARALLEL clause */
void test_parallel(void) {
    int thread_id;
    volatile int barrier_var = 0;
    
    /* Standalone parallel region with data-sharing clauses */
    #pragma omp parallel private(thread_id) shared(barrier_var) default(none)
    {
        thread_id = omp_get_thread_num();
        
        #pragma omp critical
        {
            printf("Thread %d in parallel region\n", thread_id);
        }
        
        #pragma omp barrier
        barrier_var = thread_id;
        
        #pragma omp atomic
        global_counter += thread_id;
    }
    
    /* Nested parallel region */
    #pragma omp parallel if(global_counter > 0)
    {
        #pragma omp single
        {
            printf("Nested parallel region active\n");
        }
    }
    
    printf("test_parallel completed\n");
}

/* Test OMP_CLAUSE_SECTIONS clause */
void test_sections(void) {
    int section_a = 0, section_b = 0, section_c = 0;
    volatile int dummy = 0;
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(dummy) shared(section_a, section_b, section_c) \
            num_threads(3)
    {
        #pragma omp section
        {
            for (int i = 0; i < 1000; i++) {
                section_a += i;
                dummy = i;  /* Prevent optimization */
            }
            printf("Section A completed\n");
        }
        
        #pragma omp section
        {
            for (int j = 0; j < 500; j++) {
                section_b += j * 2;
                dummy = j;
            }
            printf("Section B completed\n");
        }
        
        #pragma omp section
        {
            for (int k = 0; k < 300; k++) {
                section_c += k * 3;
                dummy = k;
            }
            printf("Section C completed\n");
        }
    }
    
    /* Additional sections construct inside parallel region */
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { global_counter += 1; }
            
            #pragma omp section
            { global_counter += 2; }
        }
    }
    
    printf("test_sections: a=%d, b=%d, c=%d\n", section_a, section_b, section_c);
}

/* Test OMP_CLAUSE_TASKGROUP clause */
void test_taskgroup(void) {
    int task_sum = 0;
    volatile int sync_var = 0;
    
    /* Taskgroup inside parallel master region */
    #pragma omp parallel master
    {
        #pragma omp taskgroup
        {
            for (int i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(task_sum)
                {
                    int local = i * i;
                    #pragma omp atomic
                    task_sum += local;
                    sync_var = local;  /* Prevent optimization */
                }
            }
        }
        
        /* Ensure all tasks complete */
        #pragma omp taskwait
    }
    
    /* Taskgroup inside parallel single region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 100;
                }
                
                #pragma omp task
                {
                    #pragma omp atomic
                    global_counter += 200;
                }
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d\n", task_sum);
}

/* Combined test with nested constructs */
void test_combined(void) {
    /* Nested constructs to increase tree traversal */
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < 20; i++) {
            #pragma omp taskgroup
            {
                #pragma omp task
                {
                    volatile int x = i;
                    #pragma omp atomic
                    global_counter += x;
                }
            }
        }
        
        #pragma omp sections
        {
            #pragma omp section
            {
                #pragma omp task
                { global_counter += 1000; }
            }
        }
    }
    
    printf("test_combined completed\n");
}

int main(void) {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Reset counter */
    global_counter = 0;
    
    /* Execute all test functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    /* Final verification */
    printf("\nAll tests completed successfully!\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    if (global_counter > 0) {
        printf("SUCCESS: OpenMP constructs were executed.\n");
        return 0;
    } else {
        printf("ERROR: No OpenMP work performed.\n");
        return 1;
    }
}
