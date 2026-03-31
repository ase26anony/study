/* test_openmp_clauses.c
 * Generates OpenMP constructs with target clauses to trigger
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, 
 * and OMP_CLAUSE_TASKGROUP pretty-printing logic.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test parallel and for clauses */
static void test_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* Combined parallel and for clause in target region */
    #pragma omp target parallel for reduction(+:local_sum) private(i) map(tofrom:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Separate parallel clause in target region */
    #pragma omp target parallel private(i) shared(global_array) reduction(+:local_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            local_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test sections clause */
static void test_sections(void) {
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* Sections clause inside target teams construct */
    #pragma omp target teams distribute parallel for private(i) reduction(+:section1_sum)
    for (i = 0; i < N/2; i++) {
        section1_sum += global_array[i];
    }
    
    /* Direct sections clause in target region */
    #pragma omp target sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/4; i++) {
                section2_sum += global_array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = N/4; i < N/2; i++) {
                section2_sum += global_array[i];
            }
        }
    }
    
    #pragma omp atomic
    global_sum += section1_sum + section2_sum;
}

/* Function to test taskgroup clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* Taskgroup inside target parallel region */
    #pragma omp target parallel private(i) shared(global_array, task_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task private(i) shared(global_array, task_sum)
                {
                    int local_task_sum = 0;
                    for (i = 0; i < N/2; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
                
                #pragma omp task private(i) shared(global_array, task_sum)
                {
                    int local_task_sum = 0;
                    for (i = N/2; i < N; i++) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function with nested clauses */
static void test_nested_clauses(void) {
    int nested_sum = 0;
    int i, j;
    
    /* Nested: parallel with for inside target region */
    #pragma omp target parallel private(i, j) shared(global_array) reduction(+:nested_sum)
    {
        #pragma omp for collapse(2)
        for (i = 0; i < 10; i++) {
            for (j = 0; j < N/10; j++) {
                nested_sum += global_array[i * (N/10) + j];
            }
        }
    }
    
    /* Target teams distribute parallel for - generates for clause */
    #pragma omp target teams distribute parallel for simd private(i) reduction(+:nested_sum)
    for (i = 0; i < N; i++) {
        nested_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += nested_sum;
}

int main(void) {
    int i;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clauses for pretty-printer coverage...\n");
    
    /* Test all clause types */
    test_parallel_for();      /* Tests parallel and for clauses */
    test_sections();          /* Tests sections clause */
    test_taskgroup();         /* Tests taskgroup clause */
    test_nested_clauses();    /* Tests nested combinations */
    
    /* Additional direct tests in main */
    int main_sum = 0;
    
    /* Direct target parallel for */
    #pragma omp target parallel for private(i) reduction(+:main_sum) map(tofrom:main_sum)
    for (i = 0; i < N; i++) {
        main_sum += global_array[i];
    }
    
    /* Direct target sections */
    #pragma omp target sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                main_sum += global_array[i];
            }
        }
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                main_sum += global_array[i];
            }
        }
    }
    
    global_sum += main_sum;
    
    printf("Final sum: %d\n", global_sum);
    printf("Expected sum: %d\n", (N * 99) / 2);  /* Sum of 0..99 repeated 10 times */
    
    return 0;
}
