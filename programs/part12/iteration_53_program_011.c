#include <stdio.h>
#include <stdlib.h>

#define N 1000
static int global_array[N];
static int global_sum = 0;

/* Function to test target parallel for clause */
static void test_target_parallel_for(void) {
    int i;
    int local_sum = 0;
    
    /* This should generate OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp target teams distribute parallel for \
                map(tofrom: local_sum) private(i) reduction(+:local_sum)
    for (i = 0; i < N; i++) {
        local_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += local_sum;
}

/* Function to test target sections clause */
static void test_target_sections(void) {
    int section1_sum = 0, section2_sum = 0;
    int i;
    
    /* This should generate OMP_CLAUSE_SECTIONS */
    #pragma omp target teams map(tofrom: section1_sum, section2_sum)
    #pragma omp sections private(i)
    {
        #pragma omp section
        for (i = 0; i < N/2; i++) {
            section1_sum += global_array[i];
        }
        
        #pragma omp section
        for (i = N/2; i < N; i++) {
            section2_sum += global_array[i];
        }
    }
    
    #pragma omp atomic
    global_sum += section1_sum + section2_sum;
}

/* Function to test taskgroup clause */
static void test_taskgroup(void) {
    int task_sum = 0;
    int i;
    
    /* This should generate OMP_CLAUSE_TASKGROUP */
    #pragma omp target parallel map(tofrom: task_sum) private(i)
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task_sum = 0;
                    for (i = 0; i < N; i += 4) {
                        local_task_sum += global_array[i];
                    }
                    #pragma omp atomic
                    task_sum += local_task_sum;
                }
                
                #pragma omp task private(i) shared(task_sum)
                {
                    int local_task_sum = 0;
                    for (i = 1; i < N; i += 4) {
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

/* Function with combined parallel and for clauses */
static void test_combined_clauses(void) {
    int combined_sum = 0;
    int i;
    
    /* Combined parallel and for clauses in one pragma */
    #pragma omp target parallel for \
                map(tofrom: combined_sum) private(i) reduction(+:combined_sum)
    for (i = 0; i < N; i++) {
        combined_sum += global_array[i];
    }
    
    #pragma omp atomic
    global_sum += combined_sum;
}

/* Main function with additional OpenMP constructs */
int main(void) {
    int i;
    int main_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < N; i++) {
        global_array[i] = i % 100;
    }
    
    printf("Testing OpenMP clauses for pretty-printer coverage...\n");
    
    /* Test 1: Target parallel for */
    printf("1. Testing target parallel for...\n");
    test_target_parallel_for();
    
    /* Test 2: Target sections */
    printf("2. Testing target sections...\n");
    test_target_sections();
    
    /* Test 3: Taskgroup */
    printf("3. Testing taskgroup...\n");
    test_taskgroup();
    
    /* Test 4: Combined clauses */
    printf("4. Testing combined clauses...\n");
    test_combined_clauses();
    
    /* Additional test in main: nested sections */
    printf("5. Testing nested sections in main...\n");
    #pragma omp target teams map(tofrom: main_sum)
    {
        #pragma omp sections private(i)
        {
            #pragma omp section
            {
                int sec_sum = 0;
                for (i = 0; i < N/3; i++) {
                    sec_sum += global_array[i];
                }
                #pragma omp atomic
                main_sum += sec_sum;
            }
            
            #pragma omp section
            {
                int sec_sum = 0;
                for (i = N/3; i < 2*N/3; i++) {
                    sec_sum += global_array[i];
                }
                #pragma omp atomic
                main_sum += sec_sum;
            }
        }
    }
    
    global_sum += main_sum;
    
    /* Expected sum calculation */
    int expected_sum = 0;
    for (i = 0; i < N; i++) {
        expected_sum += global_array[i];
    }
    expected_sum = expected_sum * 4 + main_sum; /* 4 tests + main sections */
    
    printf("Final global_sum = %d\n", global_sum);
    printf("Expected sum = %d\n", expected_sum);
    
    if (global_sum == expected_sum) {
        printf("SUCCESS: All OpenMP constructs executed correctly.\n");
    } else {
        printf("WARNING: Sum mismatch - check OpenMP implementation.\n");
    }
    
    return 0;
}
