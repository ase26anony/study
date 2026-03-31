#include <stdio.h>
#include <stdlib.h>

/* Complex reduction patterns */
void test_reduction_temporaries(int n, int* results) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    int arr[3] = {0, 0, 0};
    
    /* Multiple reduction variables of different types */
    #pragma omp parallel reduction(+:sum, fsum, dsum) reduction(+:arr[:3])
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            fsum += i * 0.5f;
            dsum += i * 0.25;
            arr[i % 3] += i;
        }
        
        /* Nested reduction with taskloop */
        #pragma omp taskloop reduction(*:sum) if(0)
        for (int i = 1; i < 10; i++) {
            sum *= i;
        }
    }
    
    results[0] = sum;
    results[1] = (int)fsum;
    results[2] = (int)dsum;
    results[3] = arr[0] + arr[1] + arr[2];
}

/* Scan temporaries with inscan clause */
void test_scan_temporaries(int n, int* results) {
    int sum = 0;
    int scan_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan inclusive(scan_sum)
        scan_sum = sum;
    }
    
    /* Combined parallel for with scan */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
        #pragma omp scan exclusive(scan_sum)
        scan_sum = sum;
    }
    
    results[0] = sum;
    results[1] = scan_sum;
}

/* Conditional temporaries with volatile conditions */
void test_conditional_temporaries(int n, volatile int cond, int* results) {
    int sum1 = 0, sum2 = 0;
    
    /* Parallel if with volatile condition */
    #pragma omp parallel if(cond > 0) reduction(+:sum1)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum1 += i;
        }
    }
    
    /* Nested if clauses with function arguments */
    #pragma omp parallel if(n > 100) if(cond != 0) reduction(+:sum2)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum2 += i * 2;
        }
    }
    
    results[0] = sum1;
    results[1] = sum2;
}

/* Enter data with 'to' mapper */
void test_enter_data(int n) {
    int* array = (int*)malloc(n * sizeof(int));
    
    if (array) {
        /* OMP_CLAUSE_ENTER with 'to' modifier */
        #pragma omp enter data map(to: array[0:n])
        
        /* Use the array */
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] = i * i;
        }
        
        #pragma omp exit data map(from: array[0:n])
        
        free(array);
    }
}
