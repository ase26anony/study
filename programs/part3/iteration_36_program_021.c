#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int* results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int array_sum[3] = {0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum) reduction(*:product) reduction(-:diff)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            product *= 1.0f + (i % 10) * 0.01f;
            diff -= 0.1;
        }
        
        /* Nested reduction in task */
        #pragma omp single
        {
            #pragma omp task reduction(+:array_sum[:3])
            {
                for (int i = 0; i < n; i++) {
                    array_sum[i % 3] += i * 2;
                }
            }
        }
    }
    
    /* Combined construct with reduction */
    #pragma omp parallel for simd reduction(+:sum) reduction(*:product)
    for (int i = 0; i < n; i++) {
        sum += i * 3;
        product *= 1.0f - (i % 5) * 0.005f;
    }
    
    results[0] = sum;
    results[1] = (int)product;
    results[2] = (int)diff;
    results[3] = array_sum[0] + array_sum[1] + array_sum[2];
}

/* Function 2: Scan operations */
void test_scans(int n, int* scan_results) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum;
        scan_results[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        int val = i * 2;
        sum += val;
        #pragma omp scan exclusive(prefix_sum)
        {
            scan_results[i] = prefix_sum;
            prefix_sum = sum;
        }
    }
}

/* Function 3: Conditional temporaries */
void test_conditionals(int n, volatile int cond1, volatile int cond2, int* cond_results) {
    int sum1 = 0, sum2 = 0;
    
    /* Parallel with volatile condition - forces condtemp */
    #pragma omp parallel if(cond1 > 0) reduction(+:sum1)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum1 += i;
        }
    }
    
    /* Nested conditionals */
    #pragma omp parallel if(cond1 || cond2) reduction(+:sum2)
    {
        #pragma omp for if(cond1 && cond2)
        for (int i = 0; i < n; i++) {
            sum2 += i * 2;
        }
    }
    
    cond_results[0] = sum1;
    cond_results[1] = sum2;
}

/* Function 4: Enter data with 'to' mapper */
void test_enter_data(int n, float* data) {
    /* Allocate some data */
    float* local_data = (float*)malloc(n * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        local_data[i] = i * 1.5f;
    }
    
    /* Use enter data with 'to' mapper - should trigger OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: local_data[0:n])
    
    /* Process data in parallel */
    #pragma omp target enter data map(to: local_data[0:n])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        local_data[i] *= 2.0f;
    }
    
    #pragma omp target exit data map(from: local_data[0:n])
    
    /* Copy back results */
    for (int i = 0; i < n; i++) {
        data[i] = local_data[i];
    }
    
    free(local_data);
}
