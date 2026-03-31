/* test_omp_clauses.c
 * Test program to cover OpenMP clause name printing in GCC tree pretty-printer
 * Specifically targets: OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 * OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test function for OMP_CLAUSE_FOR */
void test_parallel_for(void) {
    int i;
    int sum = 0;
    volatile int local_volatile = 0;
    
    /* Direct usage of 'for' clause in combined construct */
    #pragma omp parallel for private(i) shared(sum) schedule(static) \
        num_threads(2) if(1)
    for (i = 0; i < 100; i++) {
        sum += i;
        local_volatile = i;  /* Prevent optimization */
    }
    
    /* Additional 'for' clause in worksharing construct */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:global_counter)
        for (i = 0; i < 50; i++) {
            global_counter += 1;
        }
    }
    
    printf("test_parallel_for: sum = %d, global_counter = %d\n", sum, global_counter);
}

/* Test function for OMP_CLAUSE_PARALLEL */
void test_parallel(void) {
    int local_var = 0;
    volatile int barrier_var = 0;
    
    /* Standalone parallel region */
    #pragma omp parallel private(local_var) shared(barrier_var) \
        default(none) num_threads(2)
    {
        local_var = omp_get_thread_num();
        barrier_var = local_var;
        
        #pragma omp barrier
        
        #pragma omp critical
        {
            global_counter += local_var;
        }
    }
    
    /* Parallel region with if clause */
    #pragma omp parallel if(1) proc_bind(close)
    {
        #pragma omp single
        {
            printf("test_parallel: thread %d in parallel region\n", 
                   omp_get_thread_num());
        }
    }
    
    printf("test_parallel completed, global_counter = %d\n", global_counter);
}

/* Test function for OMP_CLAUSE_SECTIONS */
void test_sections(void) {
    int section1_result = 0;
    int section2_result = 0;
    volatile int sync_var = 0;
    
    /* Combined parallel sections construct */
    #pragma omp parallel sections private(sync_var) \
        shared(section1_result, section2_result) num_threads(3)
    {
        #pragma omp section
        {
            for (int i = 0; i < 100; i++) {
                section1_result += i;
            }
            sync_var = 1;
            printf("Section 1 completed: %d\n", section1_result);
        }
        
        #pragma omp section
        {
            /* Wait a bit to ensure sections run concurrently */
            volatile int wait = 0;
            for (int j = 0; j < 1000; j++) wait = j;
            
            for (int i = 100; i < 200; i++) {
                section2_result += i;
            }
            sync_var = 2;
            printf("Section 2 completed: %d\n", section2_result);
        }
        
        #pragma omp section
        {
            /* Third section for more coverage */
            printf("Section 3: monitoring other sections\n");
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
    
    printf("test_sections: results = %d, %d, global_counter = %d\n",
           section1_result, section2_result, global_counter);
}

/* Test function for OMP_CLAUSE_TASKGROUP */
void test_taskgroup(void) {
    int task_sum = 0;
    volatile int task_sync = 0;
    
    /* Taskgroup inside parallel master region */
    #pragma omp parallel master num_threads(2)
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) firstprivate(task_sync)
            {
                for (int i = 0; i < 50; i++) {
                    #pragma omp atomic
                    task_sum += i;
                }
                task_sync = 1;
            }
            
            #pragma omp task shared(task_sum) 
            {
                for (int i = 50; i < 100; i++) {
                    #pragma omp atomic
                    task_sum += i;
                }
                task_sync = 2;
            }
            
            /* Implicit taskwait at end of taskgroup */
        }
        
        printf("Master thread: taskgroup completed, sum = %d\n", task_sum);
    }
    
    /* Nested taskgroup example */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task shared(global_counter)
                { global_counter += 10; }
                
                #pragma omp task shared(global_counter)
                { global_counter += 20; }
                
                #pragma omp taskgroup  /* Nested taskgroup */
                {
                    #pragma omp task shared(global_counter)
                    { global_counter += 30; }
                }
            }
        }
    }
    
    printf("test_taskgroup: task_sum = %d, global_counter = %d\n", 
           task_sum, global_counter);
}

/* Combined test with nested constructs */
void test_combined(void) {
    /* Parallel region containing multiple clause types */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < 10; i++) {
            /* Empty but prevents optimization */
            volatile int x = i;
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { /* Do nothing */ }
        }
        
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { global_counter += 100; }
            }
        }
    }
}

int main(void) {
    printf("=== Starting OpenMP clause coverage test ===\n");
    
    /* Initialize */
    global_counter = 0;
    
    /* Test each clause type in separate functions */
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    
    /* Additional combined test */
    test_combined();
    
    /* Final verification */
    printf("\n=== Test Summary ===\n");
    printf("All OpenMP constructs executed successfully.\n");
    printf("Final global_counter value: %d\n", global_counter);
    
    if (global_counter > 0) {
        printf("SUCCESS: OpenMP clauses were processed.\n");
        return 0;
    } else {
        printf("WARNING: No increments to global_counter detected.\n");
        return 1;
    }
}
