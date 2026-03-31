/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
                         OMP_CLAUSE_SECTIONS, and OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, cold))
#define VOLATILE volatile

/* Global array to prevent optimization */
VOLATILE int global_array[100] = {0};
VOLATILE int global_counter = 0;

/* Function 1: Test OMP_CLAUSE_FOR */
NOINLINE void test_for_clause(void) {
    VOLATILE int i;
    VOLATILE int n = 50;
    VOLATILE double result = 0.0;
    
    /* Use target teams distribute parallel for simd with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: result) num_teams(2) thread_limit(64)
    for (i = 0; i < n; i++) {
        /* Use math function to prevent optimization */
        result += sin(i * 0.1) * cos(i * 0.05);
        global_array[i % 100] += 1;
    }
    
    global_counter += (int)(result * 1000);
}

/* Function 2: Test OMP_CLAUSE_PARALLEL */
NOINLINE void test_parallel_clause(void) {
    VOLATILE int x = 0;
    VOLATILE double val = 0.0;
    
    /* Use target with parallel clause */
    #pragma omp target parallel map(tofrom: x, val) \
        num_threads(4) if(global_counter > 0)
    {
        int tid = omp_get_thread_num();
        /* Introduce potential data race for diagnostic */
        #pragma omp critical
        {
            x += tid;
            val += log(fabs(sin(tid + 1)) + 1.0);
        }
        global_array[tid % 100] += 1;
    }
    
    global_counter += x + (int)(val * 100);
}

/* Function 3: Test OMP_CLAUSE_SECTIONS */
NOINLINE void test_sections_clause(void) {
    VOLATILE int a = 0, b = 0, c = 0;
    
    /* Use target teams with sections clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: a, b, c) num_teams(2)
    {
        #pragma omp section
        {
            a = 100;
            global_array[0] += a;
        }
        #pragma omp section
        {
            b = 200;
            global_array[1] += b;
        }
        #pragma omp section
        {
            c = 300;
            global_array[2] += c;
        }
    }
    
    global_counter += a + b + c;
}

/* Function 4: Test OMP_CLAUSE_TASKGROUP */
NOINLINE void test_taskgroup_clause(void) {
    VOLATILE int sum = 0;
    VOLATILE int i;
    
    /* Use taskloop with taskgroup clause */
    #pragma omp taskloop taskgroup \
        grainsize(10) num_tasks(5) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        /* Use volatile and external call to prevent optimization */
        VOLATILE int idx = i % 50;
        sum += (int)(sin(idx) * 100) + global_array[idx];
        
        /* Nested taskgroup for extra coverage */
        #pragma omp taskgroup
        {
            VOLATILE int temp = idx * 2;
            global_array[idx] += temp;
        }
    }
    
    global_counter += sum;
}

/* Function 5: Additional test with standalone taskgroup */
NOINLINE void test_standalone_taskgroup(void) {
    VOLATILE int x = 0;
    
    /* Standalone taskgroup construct */
    #pragma omp taskgroup
    {
        #pragma omp task shared(x)
        {
            #pragma omp atomic
            x += 10;
        }
        
        #pragma omp task shared(x)
        {
            #pragma omp atomic
            x += 20;
        }
        
        /* Taskwait is implicit at end of taskgroup */
    }
    
    global_counter += x;
}

/* Main function that calls all tests */
int main(void) {
    VOLATILE int seed = 42;
    VOLATILE int total = 0;
    
    /* Initialize with some non-zero values */
    srand(seed);
    for (int i = 0; i < 100; i++) {
        global_array[i] = rand() % 10;
    }
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    test_standalone_taskgroup();
    
    /* Compute final result to ensure execution */
    for (int i = 0; i < 100; i++) {
        total += global_array[i];
    }
    total += global_counter;
    
    printf("Result: %d\n", total);
    
    /* Force diagnostic potential: incorrect nesting hint */
    #pragma omp parallel
    {
        /* This might trigger warnings about implicit barrier */
        #pragma omp taskgroup
        {
            VOLATILE int y = total;
            y = y + 1;
        }
    }
    
    return 0;
}
