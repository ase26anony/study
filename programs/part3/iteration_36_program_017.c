#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Test 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    int arr[4] = {0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum, fsum, dsum) \
                         reduction(+:arr[:4])
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            fsum += (float)i * 0.5f;
            dsum += (double)i * 0.25;
            arr[i % 4] += i;
        }
        
        /* Nested reduction in tasks */
        #pragma omp taskloop reduction(*:sum)
        for (int i = 1; i < 10; i++) {
            sum *= (i % 3) + 1;
        }
    }
    
    results[0] = sum;
    results[1] = (int)fsum;
    results[2] = (int)dsum;
    results[3] = arr[0] + arr[1] + arr[2] + arr[3];
}

/* Test 2: Scan operations with inscan reductions */
void test_scans(int n, int *results) {
    int sum = 0;
    int scan_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan exclusive(scan_sum)
        scan_sum = sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
        #pragma omp scan inclusive(sum)
    }
    
    results[0] = sum;
    results[1] = scan_sum;
}

/* Test 3: Conditional temporaries with volatile conditions */
void test_conditionals(volatile int cond1, volatile int cond2, int n, int *result) {
    int sum = 0;
    
    /* Parallel with non-constant if clause */
    #pragma omp parallel if(cond1 > 0) reduction(+:sum)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
        }
        
        /* Nested parallel with another condition */
        #pragma omp parallel if(cond2 < 100) num_threads(2)
        {
            #pragma omp for
            for (int i = 0; i < 10; i++) {
                sum += i * 2;
            }
        }
    }
    
    *result = sum;
}

/* Test 4: Enter data with to mapper */
void test_enter_data(int n) {
    int *array = (int *)malloc(n * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        array[i] = i;
    }
    
    /* Use enter data with to clause */
    #pragma omp enter data map(to: array[0:n])
    
    /* Process data in target region */
    #pragma omp target map(from: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            array[i] *= 2;
        }
    }
    
    #pragma omp exit data map(from: array[0:n])
    
    /* Verify and cleanup */
    int check = 0;
    for (int i = 0; i < n; i++) {
        check += array[i];
    }
    
    free(array);
    
    /* Store result in global to prevent optimization */
    extern int enter_data_result;
    enter_data_result = check;
}

/* Combined test with nested constructs */
void test_combined(volatile int cond, int n, int *results) {
    int total = 0;
    int scan_val = 0;
    
    /* Complex construct mixing reduction, scan, and condition */
    #pragma omp parallel if(cond > 50) reduction(+:total)
    {
        #pragma omp for simd reduction(inscan, +:total)
        for (int i = 0; i < n; i++) {
            total += i;
            #pragma omp scan exclusive(scan_val)
            scan_val = total;
        }
        
        #pragma omp taskloop reduction(*:total)
        for (int i = 1; i < n/2; i++) {
            total *= (i % 5) + 1;
        }
    }
    
    results[0] = total;
    results[1] = scan_val;
}
