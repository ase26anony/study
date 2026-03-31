#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int* results) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    int arr[10] = {0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum, fsum, dsum) reduction(+:arr[:10])
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            fsum += i * 0.5f;
            dsum += i * 0.25;
            arr[i % 10] += i;
        }
        
        /* Nested reduction in task */
        #pragma omp single
        {
            #pragma omp task reduction(*:sum)
            {
                sum = (sum > 0) ? sum : 1;
                for (int j = 0; j < 10; j++) {
                    sum *= (arr[j] + 1);
                }
            }
        }
    }
    
    results[0] = sum;
    results[1] = (int)fsum;
    results[2] = (int)dsum;
}

/* Function 2: Scan operations */
void test_scans(int n, int* scan_results) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = sum;
        scan_results[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        int val = i * 2;
        sum += val;
        #pragma omp scan inclusive(sum)
        scan_results[i + n] = sum;
    }
}

/* Function 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int n, int* results) {
    int sum = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond1 > 0) reduction(+:sum)
    {
        #pragma omp for if(cond2 < n)
        for (int i = 0; i < n; i++) {
            sum += i;
        }
        
        /* Task with conditional */
        #pragma omp single
        {
            #pragma omp task if(cond1 && cond2)
            {
                sum *= 2;
            }
        }
    }
    
    results[0] = sum;
    
    /* Teams construct with condition */
    #pragma omp target teams if(cond1 != 0) map(tofrom:sum)
    {
        #pragma omp distribute parallel for reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += 1;
        }
    }
    
    results[1] = sum;
}

/* Function 4: Enter data with 'to' mapper */
void test_enter_data(int n, int* data) {
    /* Allocate and initialize data */
    int* array = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Use enter data with 'to' mapper */
    #pragma omp enter data map(to: array[0:n])
    
    /* Process data on target */
    #pragma omp target map(alloc: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    /* Copy back and free */
    #pragma omp exit data map(from: array[0:n])
    
    for (int i = 0; i < n; i++) {
        data[i] = array[i];
    }
    
    free(array);
}

/* Function 5: Combined complex patterns */
void test_combined(volatile int cond, int n, int* results) {
    int total = 0;
    int scan_array[100];
    
    /* Combined parallel for simd with reduction */
    #pragma omp parallel for simd reduction(+:total) if(cond > 5)
    for (int i = 0; i < n && i < 100; i++) {
        total += i * 3;
        scan_array[i] = 0;
    }
    
    /* Taskloop with reduction */
    #pragma omp taskloop reduction(*:total)
    for (int i = 1; i < n && i < 50; i++) {
        total *= (i + 1);
    }
    
    /* SIMD with inscan */
    int scan_sum = 0;
    #pragma omp simd reduction(inscan, +:scan_sum)
    for (int i = 0; i < n && i < 100; i++) {
        scan_sum += i;
        #pragma omp scan exclusive(scan_array[i])
        scan_array[i] = scan_sum;
    }
    
    results[0] = total;
    results[1] = scan_sum;
    results[2] = scan_array[n/2];
}
