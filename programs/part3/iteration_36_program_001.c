#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern volatile int g_volatile_cond;

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *arr, float *farr, int *results) {
    int sum = 0;
    float fsum = 0.0f;
    int prod = 1;
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum, fsum) reduction(*:prod) if(g_volatile_cond > 0)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += arr[i];
            fsum += farr[i];
        }
        
        #pragma omp single
        {
            prod = 1;
            for (int i = 1; i <= n/10; i++) {
                prod *= i;
            }
        }
    }
    
    /* Nested reduction with SIMD */
    #pragma omp parallel for simd reduction(+:sum) collapse(2)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            sum += arr[i*2 + j];
        }
    }
    
    /* Taskloop reduction */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskloop reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += arr[i] % 10;
        }
    }
    
    results[0] = sum;
    results[1] = (int)fsum;
    results[2] = prod;
}

/* Function 2: Scan operations */
void test_scans(int n, int *arr, int *scan_results) {
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        prefix_sum += arr[i];
        #pragma omp scan inclusive(prefix_sum)
        scan_results[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        prefix_sum += arr[i] * 2;
        #pragma omp scan inclusive(prefix_sum)
        scan_results[i] += prefix_sum;
    }
}

/* Function 3: Conditional temporaries */
void test_conditionals(int n, int *arr, volatile int cond_var, int *result) {
    int local_sum = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond_var > 0) if(cond_var < n) reduction(+:local_sum)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
        }
    }
    
    /* Combined construct with if clause */
    #pragma omp parallel for if(cond_var != 0) schedule(dynamic)
    for (int i = 0; i < n; i++) {
        #pragma omp atomic
        result[i] += local_sum;
    }
}

/* Function 4: Enter data with to mapper */
void test_enter_data(int n, float *data) {
    /* Create data mapping with 'to' clause */
    #pragma omp enter data map(to: data[0:n])
    
    /* Use the data in parallel region */
    #pragma omp target enter data map(to: data[0:n/2])
    
    /* Nested data regions */
    #pragma omp target data map(to: data[n/2:n/2])
    {
        #pragma omp target teams distribute parallel for
        for (int i = n/2; i < n; i++) {
            data[i] = data[i] * 2.0f;
        }
    }
    
    #pragma omp exit data map(from: data[0:n])
}
