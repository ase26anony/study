#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Test function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float prod = 1.0f;
    double diff = 100.0;
    int arr[3] = {0, 0, 0};
    
    /* Combined construct with multiple reductions */
    #pragma omp parallel for simd reduction(+:sum) reduction(*:prod) reduction(-:diff)
    for (int i = 0; i < n; i++) {
        sum += i;
        prod *= 1.0f + i * 0.001f;
        diff -= 0.5;
    }
    
    /* Nested reduction in taskloop */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:arr[:3])
            for (int i = 0; i < n; i++) {
                arr[i % 3] += i;
            }
        }
    }
    
    results[0] = sum;
    results[1] = (int)prod;
    results[2] = (int)diff;
    results[3] = arr[0] + arr[1] + arr[2];
}

/* Test function 2: Scan operations */
void test_scans(int n, int *results) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = sum;
    }
    
    /* For loop with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
        #pragma omp scan inclusive(sum)
    }
    
    results[0] = sum;
    results[1] = prefix_sum;
}

/* Test function 3: Conditional temporaries */
void test_conditionals(int n, int *results, volatile int cond) {
    int sum1 = 0, sum2 = 0;
    
    /* Parallel with non-constant if clause */
    #pragma omp parallel if(cond > 0) reduction(+:sum1)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum1 += i;
        }
    }
    
    /* Another conditional with function argument */
    #pragma omp parallel if(n > 1000) reduction(+:sum2)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum2 += i * 2;
        }
    }
    
    results[0] = sum1;
    results[1] = sum2;
}

/* Test function 4: Enter data with to mapper */
void test_enter_data(int n) {
    int *array = (int *)malloc(n * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Enter data with to clause */
    #pragma omp enter data map(to: array[0:n])
    
    /* Use the data in parallel region */
    #pragma omp target data map(from: array[0:n])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Exit data */
    #pragma omp exit data map(from: array[0:n])
    
    free(array);
}
