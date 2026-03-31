/* test_omp_clauses.c - Coverage for OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, 
                         OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 100
volatile int trigger = 100;  /* Prevent optimization */

/* Function to test OMP_CLAUSE_FOR */
__attribute__((noinline, cold))
void test_for_clause(void) {
    int i;
    float a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }
    
    /* Use target teams distribute parallel for simd with explicit 'for' clause */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        num_teams(2) thread_limit(64)
    for (i = 0; i < trigger && i < N; i++) {  /* volatile bound */
        c[i] = a[i] + b[i] + sinf((float)i);  /* Prevent dead code elimination */
    }
    
    /* Use result to prevent optimization */
    volatile float sum = 0.0f;
    for (i = 0; i < N; i++) {
        sum += c[i];
    }
}

/* Function to test OMP_CLAUSE_PARALLEL */
__attribute__((noinline, cold))
void test_parallel_clause(void) {
    int x = 0;
    
    /* Use target with parallel clause */
    #pragma omp target parallel map(tofrom: x) \
        if(trigger > 50) num_threads(4)
    {
        int tid = omp_get_thread_num();
        #pragma omp atomic
        x += tid + 1;
        
        /* Add some computation to prevent optimization */
        volatile float temp = sinf((float)tid);
        (void)temp;
    }
    
    /* Use result */
    volatile int result = x;
    (void)result;
}

/* Function to test OMP_CLAUSE_SECTIONS */
__attribute__((noinline, cold))
void test_sections_clause(void) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Use target teams with sections clause */
    #pragma omp target teams distribute parallel for sections \
        map(tofrom: sum1, sum2, sum3) num_teams(2)
    {
        #pragma omp section
        {
            for (int i = 0; i < trigger && i < 10; i++) {
                sum1 += i * 2;
            }
        }
        
        #pragma omp section
        {
            for (int j = 0; j < trigger && j < 10; j++) {
                sum2 += j * 3;
            }
        }
        
        #pragma omp section
        {
            for (int k = 0; k < trigger && k < 10; k++) {
                sum3 += k * 4;
            }
        }
    }
    
    /* Use results */
    volatile int total = sum1 + sum2 + sum3;
    (void)total;
}

/* Function to test OMP_CLAUSE_TASKGROUP */
__attribute__((noinline, cold))
void test_taskgroup_clause(void) {
    int counter = 0;
    
    /* Create a taskgroup */
    #pragma omp taskgroup task_reduction(+:counter)
    {
        /* Spawn some tasks */
        #pragma omp task
        {
            for (int i = 0; i < trigger && i < 5; i++) {
                #pragma omp atomic
                counter += i;
            }
        }
        
        #pragma omp task
        {
            for (int j = 0; j < trigger && j < 5; j++) {
                #pragma omp atomic
                counter += j * 2;
            }
        }
        
        /* Wait for tasks to complete */
        #pragma omp taskwait
    }
    
    /* Also test taskloop with taskgroup clause */
    int arr[20];
    #pragma omp taskloop taskgroup grainsize(4) num_tasks(5)
    for (int i = 0; i < trigger && i < 20; i++) {
        arr[i] = i * i;
    }
    
    /* Use results */
    volatile int check = counter + arr[0];
    (void)check;
}

/* Function with potential data race to trigger diagnostic */
__attribute__((noinline, cold))
void trigger_diagnostic(void) {
    int shared_var = 0;
    
    /* This might trigger a warning about data race */
    #pragma omp taskgroup
    {
        #pragma omp task shared(shared_var)
        {
            shared_var = 1;  /* Potential race condition */
        }
        
        #pragma omp task shared(shared_var)
        {
            shared_var = 2;  /* Another potential race */
        }
    }
    
    volatile int v = shared_var;
    (void)v;
}

int main(void) {
    /* Initialize random seed for variability */
    srand(42);
    
    printf("Testing OpenMP clause coverage...\n");
    
    /* Call all test functions */
    test_for_clause();
    test_parallel_clause();
    test_sections_clause();
    test_taskgroup_clause();
    
    /* Also trigger diagnostic path */
    trigger_diagnostic();
    
    /* Compute a final result to ensure execution */
    int final_result = 0;
    #pragma omp parallel for reduction(+:final_result)
    for (int i = 0; i < trigger && i < 50; i++) {
        final_result += (int)(sinf((float)i) * 100.0f);
    }
    
    printf("Final result: %d\n", final_result);
    return 0;
}
