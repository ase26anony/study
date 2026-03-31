/* test_openmp_clauses.c
 * Designed to generate OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
 * OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP nodes in the AST
 * for coverage of tree-pretty-print.cc lines 1434-1445
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1000
#define M 500

/* File-scope variables for testing data environment */
static int global_array[N];
static int global_sum = 0;
static int global_task_counter = 0;

/* Function to test target parallel for with for clause */
static void test_target_parallel_for(void)
{
    int i;
    int local_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    /* This should generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for \
        map(tofrom: local_sum) map(to: global_array[0:N]) \
        reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    /* Also test combined parallel for */
    #pragma omp target parallel for \
        map(tofrom: local_sum) map(to: global_array[0:N]) \
        reduction(+:local_sum) private(i)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i] * 2;
    }
    
    global_sum += local_sum;
}

/* Function to test target parallel with parallel clause */
static void test_target_parallel(void)
{
    int i;
    int local_result = 0;
    int private_var = 0;
    
    /* This should generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel \
        map(tofrom: local_result) \
        private(private_var) shared(global_array)
    {
        private_var = omp_get_thread_num();
        
        #pragma omp atomic
        local_result += private_var;
    }
    
    /* Nested parallel region */
    #pragma omp target teams parallel \
        map(tofrom: local_result) num_teams(2) num_threads(4)
    {
        #pragma omp atomic
        local_result += 1;
    }
    
    global_sum += local_result;
}

/* Function to test target sections with sections clause */
static void test_target_sections(void)
{
    int section_a = 0, section_b = 0, section_c = 0;
    
    /* This should generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams sections \
        map(tofrom: section_a, section_b, section_c) \
        num_teams(2)
    {
        #pragma omp section
        {
            for (int i = 0; i < M; i++) {
                section_a += i % 10;
            }
        }
        
        #pragma omp section
        {
            for (int j = 0; j < M; j++) {
                section_b += j % 20;
            }
        }
        
        #pragma omp section
        {
            for (int k = 0; k < M; k++) {
                section_c += k % 30;
            }
        }
    }
    
    /* Sections inside target teams */
    #pragma omp target teams
    {
        #pragma omp sections
        {
            #pragma omp section
            { section_a *= 2; }
            
            #pragma omp section
            { section_b *= 3; }
        }
    }
    
    global_sum += section_a + section_b + section_c;
}

/* Function to test taskgroup clause */
static void test_taskgroup(void)
{
    int task_results[4] = {0};
    
    /* This should generate OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel map(tofrom: task_results[0:4])
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                for (int t = 0; t < 4; t++) {
                    #pragma omp task firstprivate(t) shared(task_results)
                    {
                        int tid = omp_get_thread_num();
                        task_results[t] = tid * 100 + t;
                        
                        #pragma omp atomic
                        global_task_counter++;
                    }
                }
            }
        }
    }
    
    /* Another taskgroup example */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { global_sum += 1; }
                
                #pragma omp task
                { global_sum += 2; }
            }
        }
    }
    
    for (int i = 0; i < 4; i++) {
        global_sum += task_results[i];
    }
}

/* Function combining multiple clauses */
static void test_combined_clauses(void)
{
    int combined_sum = 0;
    int i;
    
    /* Combined parallel for with multiple clauses */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: combined_sum) map(to: global_array[0:N]) \
        reduction(+:combined_sum) private(i) \
        collapse(1) schedule(static, 64)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i] * 3;
    }
    
    /* Nested: sections inside parallel region */
    #pragma omp target parallel map(tofrom: combined_sum)
    {
        #pragma omp sections
        {
            #pragma omp section
            { combined_sum += 100; }
            
            #pragma omp section
            { combined_sum += 200; }
        }
        
        #pragma omp taskgroup
        {
            #pragma omp task
            { combined_sum += 300; }
        }
    }
    
    global_sum += combined_sum;
}

int main(void)
{
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize global array */
    for (int i = 0; i < N; i++) {
        global_array[i] = (i * 3) % 97;
    }
    
    /* Test each clause type in separate functions */
    test_target_parallel_for();      /* Generates FOR and PARALLEL clauses */
    test_target_parallel();          /* Generates PARALLEL clause */
    test_target_sections();          /* Generates SECTIONS clause */
    test_taskgroup();                /* Generates TASKGROUP clause */
    test_combined_clauses();         /* Generates multiple clauses */
    
    /* Additional direct tests in main */
    int main_sum = 0;
    
    /* Direct target parallel for */
    #pragma omp target parallel for \
        map(tofrom: main_sum) map(to: global_array[0:N]) \
        reduction(+:main_sum)
    for (int i = 0; i < N; i += 2) {
        main_sum += global_array[i];
    }
    
    /* Direct target sections */
    int sec_a = 0, sec_b = 0;
    #pragma omp target sections map(tofrom: sec_a, sec_b)
    {
        #pragma omp section
        { sec_a = 1; }
        
        #pragma omp section
        { sec_b = 2; }
    }
    
    /* Direct taskgroup in target region */
    #pragma omp target parallel map(tofrom: main_sum)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task
                { main_sum += sec_a + sec_b; }
            }
        }
    }
    
    global_sum += main_sum;
    
    printf("Final global_sum = %d\n", global_sum);
    printf("Task counter = %d\n", global_task_counter);
    printf("Test completed.\n");
    
    return 0;
}
