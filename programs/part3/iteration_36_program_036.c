#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *arr, float *farr, volatile int cond) {
    int sum_int = 0;
    float sum_float = 0.0f;
    long long prod = 1;
    
    /* Multiple reduction variables with different types */
    #pragma omp parallel for simd reduction(+:sum_int, sum_float) \
                             reduction(*:prod) if(cond > 0)
    for (int i = 0; i < n; i++) {
        sum_int += arr[i];
        sum_float += farr[i];
        if (arr[i] != 0)
            prod *= (arr[i] % 10) + 1;
    }
    
    /* Nested reduction in taskloop */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:sum_int)
            for (int i = 0; i < n; i++) {
                sum_int += arr[i] * 2;
            }
        }
    }
    
    printf("Reductions: sum_int=%d, sum_float=%.2f, prod=%lld\n", 
           sum_int, sum_float, prod);
}

/* Function 2: Scan operations */
void test_scans(int n, int *arr) {
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        prefix_sum += arr[i];
        #pragma omp scan exclusive(prefix_sum)
        arr[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        arr[i] += prefix_sum;
    }
    
    printf("Scan completed, last prefix=%d\n", prefix_sum);
}

/* Function 3: Conditional temporaries */
void test_conditionals(int n, int *arr, volatile int v1, volatile int v2) {
    int result = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel for if(v1 > 0) reduction(+:result)
    for (int i = 0; i < n; i++) {
        result += arr[i];
    }
    
    /* Combined construct with if clause */
    #pragma omp target teams distribute parallel for \
            if(v2 < 100) reduction(+:result)
    for (int i = 0; i < n; i++) {
        result -= arr[i];
    }
    
    printf("Conditional result: %d\n", result);
}

/* Function 4: Enter data with 'to' mapper */
void test_enter_data(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    /* Trigger OMP_CLAUSE_ENTER with 'to' modifier */
    #pragma omp enter data map(to: data[0:n])
    
    /* Use the data in target region */
    #pragma omp target map(from: data[0:n])
    {
        for (int i = 0; i < n; i++) {
            data[i] += 1;
        }
    }
    
    #pragma omp exit data map(from: data[0:n])
    
    int check = 0;
    for (int i = 0; i < n; i++) {
        check += data[i];
    }
    printf("Enter data check sum: %d\n", check);
    
    free(data);
}
