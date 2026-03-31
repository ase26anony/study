/* test_omp_clauses.c - Program to exercise OpenMP clause pretty-printing */
#include <stdio.h>
#include <omp.h>

volatile int global_counter = 0;

/* Test OMP_CLAUSE_FOR */
void test_parallel_for() {
    int i;
    int sum = 0;
    volatile int local_sum = 0;
    
    #pragma omp parallel for schedule(static) private(i) shared(sum) firstprivate(local_sum)
    for (i = 0; i < 100; i++) {
        sum += i;
        local_sum = i;
    }
    
    printf("test_parallel_for completed: sum = %d\n", sum);
    #pragma omp atomic
    global_counter++;
}

/* Test OMP_CLAUSE_PARALLEL */
void test_parallel() {
    int thread_id;
    volatile int local_var = 42;
    
    #pragma omp parallel private(thread_id) shared(global_counter) firstprivate(local_var)
    {
        thread_id = omp_get_thread_num();
        #pragma omp atomic
        global_counter++;
        printf("Thread %d in parallel region (local_var = %d)\n", thread_id, local_var);
    }
    
    printf("test_parallel completed\n");
}

/* Test OMP_CLAUSE_SECTIONS */
void test_sections() {
    int section1_result = 0;
    int section2_result = 0;
    volatile int control = 1;
    
    #pragma omp parallel sections private(control) shared(section1_result, section2_result)
    {
        #pragma omp section
        {
            control = 10;
            for (int i = 0; i < 50; i++) {
                section1_result += i;
            }
            printf("Section 1 completed: %d\n", section1_result);
        }
        
        #pragma omp section
        {
            control = 20;
            for (int i = 50; i < 100; i++) {
                section2_result += i;
            }
            printf("Section 2 completed: %d\n", section2_result);
        }
    }
    
    printf("test_sections completed: total = %d\n", section1_result + section2_result);
    #pragma omp atomic
    global_counter++;
}

/* Test OMP_CLAUSE_TASKGROUP */
void test_taskgroup() {
    int task_sum = 0;
    volatile int sync_var = 0;
    
    #pragma omp parallel master shared(task_sum, sync_var)
    {
        #pragma omp taskgroup
        {
            #pragma omp task shared(task_sum) firstprivate(sync_var)
            {
                sync_var = 1;
                #pragma omp atomic
                task_sum += 100;
            }
            
            #pragma omp task shared(task_sum) firstprivate(sync_var)
            {
                sync_var = 2;
                #pragma omp atomic
                task_sum += 200;
            }
            
            #pragma omp task shared(task_sum) firstprivate(sync_var)
            {
                sync_var = 3;
                #pragma omp atomic
                task_sum += 300;
            }
        }
        
        printf("Taskgroup completed: sum = %d\n", task_sum);
    }
    
    printf("test_taskgroup completed\n");
    #pragma omp atomic
    global_counter++;
}

/* Combined construct with nested clauses */
void test_combined() {
    int i, j;
    volatile int temp = 0;
    
    /* Combined parallel for with multiple clauses */
    #pragma omp parallel for collapse(2) private(i, j) shared(global_counter) firstprivate(temp) schedule(dynamic)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            temp = i * j;
            if (temp > 20) {
                #pragma omp atomic
                global_counter++;
            }
        }
    }
    
    printf("test_combined completed\n");
}

int main() {
    printf("Starting OpenMP clause coverage test...\n");
    
    test_parallel_for();
    test_parallel();
    test_sections();
    test_taskgroup();
    test_combined();
    
    printf("\nAll tests completed. Global counter = %d\n", global_counter);
    printf("Expected clauses exercised:\n");
    printf("  - OMP_CLAUSE_FOR (via parallel for, combined parallel for)\n");
    printf("  - OMP_CLAUSE_PARALLEL (via parallel region)\n");
    printf("  - OMP_CLAUSE_SECTIONS (via parallel sections)\n");
    printf("  - OMP_CLAUSE_TASKGROUP (via taskgroup)\n");
    
    return 0;
}
