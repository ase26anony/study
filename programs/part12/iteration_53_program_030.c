#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for clause */
static void test_target_parallel_for(void) {
    int local_sum = 0;
    int i;
    
    /* This will generate OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target parallel clause */
static void test_target_parallel(void) {
    int local_data[N];
    int i;
    
    /* Initialize array */
    #pragma omp parallel for private(i)
    for (i = 0; i < N; i++) {
        local_data[i] = i % 100;
    }
    
    int parallel_sum = 0;
    
    /* This will generate OMP_CLAUSE_PARALLEL */
    #pragma omp target parallel map(tofrom: parallel_sum) \
                               private(i) reduction(+:parallel_sum)
    {
        #pragma omp for
        for (i = 0; i < N; i++) {
            parallel_sum += local_data[i];
        }
    }
    
    #pragma omp atomic
    global_sum += parallel_sum;
}

/* Function to test target sections clause */
static void test_target_sections(void) {
    int section_results[3] = {0, 0, 0};
    
    /* This will generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: section_results)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                for (int i = 0; i < N/3; i++) {
                    section_results[0] += global_array[i];
                }
            }
            
            #pragma omp section
            {
                for (int i = N/3; i < 2*N/3; i++) {
                    section_results[1] += global_array[i];
                }
            }
            
            #pragma omp section
            {
                for (int i = 2*N/3; i < N; i++) {
                    section_results[2] += global_array[i];
                }
            }
        }
    }
    
    for (int i = 0; i < 3; i++) {
        #pragma omp atomic
        global_sum += section_results[i];
    }
}

/* Function to test taskgroup clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    
    /* This will generate OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                #pragma omp task in_reduction(+:task_sum)
                {
                    for (int i = 0; i < N/2; i++) {
                        #pragma omp atomic
                        task_sum += global_array[i];
                    }
                }
                
                #pragma omp task in_reduction(+:task_sum)
                {
                    for (int i = N/2; i < N; i++) {
                        #pragma omp atomic
                        task_sum += global_array[i];
                    }
                }
            }
        }
    }
    
    #pragma omp atomic
    global_sum += task_sum;
}

/* Function combining multiple clauses */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    
    /* Combined parallel and for clauses */
    #pragma omp target parallel for map(tofrom: combined_sum) \
                                   private(int i) reduction(+:combined_sum)
    for (int i = 0; i < N; i++) {
        combined_sum += global_array[i] * 2;
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

int main(void) {
    /* Initialize global array */
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        global_array[i] = (i + 1) % 50;
    }
    
    printf("Starting OpenMP tests...\n");
    
    /* Test each clause separately */
    test_target_parallel_for();      /* Generates OMP_CLAUSE_FOR */
    test_target_parallel();          /* Generates OMP_CLAUSE_PARALLEL */
    test_target_sections();          /* Generates OMP_CLAUSE_SECTIONS */
    test_taskgroup();                /* Generates OMP_CLAUSE_TASKGROUP */
    test_combined_clauses();         /* Generates combined clauses */
    
    printf("Final global sum: %d\n", global_sum);
    printf("Expected sum: %d\n", N * 49 / 2 * 4);  /* Rough estimate */
    
    return 0;
}
