#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int arr[4] = {0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum) reduction(*:product) reduction(-:diff)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            product *= 1.0f + (i % 10) * 0.01f;
            diff -= 0.5;
        }
        
        /* Nested reduction in task */
        #pragma omp single
        {
            #pragma omp task reduction(+:arr[:4])
            {
                for (int i = 0; i < 4; i++) {
                    arr[i] += omp_get_thread_num() + i;
                }
            }
            #pragma omp taskwait
        }
    }
    
    /* Combined construct with reduction */
    #pragma omp parallel for simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    results[0] = sum;
    results[1] = (int)(product * 1000);
    results[2] = (int)diff;
    results[3] = arr[0] + arr[1] + arr[2] + arr[3];
}

/* Function 2: Scan operations */
void test_scans(int n, int *output) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum;
        output[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += output[i];
        #pragma omp scan exclusive(prefix_sum)
        {
            prefix_sum = sum;
            output[i] = prefix_sum - output[i];
        }
    }
}

/* Function 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int n, int *result) {
    int sum = 0;
    
    /* Parallel with non-constant if clause */
    #pragma omp parallel if(cond1 > 0) reduction(+:sum)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
        }
        
        /* Nested conditional */
        #pragma omp sections if(cond2 < 10)
        {
            #pragma omp section
            {
                sum += 100;
            }
            #pragma omp section
            {
                sum += 200;
            }
        }
    }
    
    /* Taskloop with conditional */
    #pragma omp parallel if(cond1 || cond2)
    {
        #pragma omp single
        {
            #pragma omp taskloop if(cond1 && cond2) reduction(+:sum)
            for (int i = 0; i < n; i++) {
                sum += i % 5;
            }
        }
    }
    
    *result = sum;
}

/* Function 4: Enter data with to mapper */
void test_enter_data(int n) {
    int *array1 = (int *)malloc(n * sizeof(int));
    int *array2 = (int *)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        array1[i] = i;
        array2[i] = n - i;
    }
    
    /* Enter data with to mapper - should trigger OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: array1[0:n], array2[0:n])
    
    /* Use the data in parallel region */
    #pragma omp target enter data map(to: array1[0:n])
    
    /* Exit data */
    #pragma omp exit data map(from: array1[0:n])
    #pragma omp exit data map(release: array2[0:n])
    
    free(array1);
    free(array2);
}
