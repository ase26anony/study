#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int* results) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    long lsum = 0;
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum, fsum, dsum, lsum)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            sum += i;
            fsum += (float)i * 0.5f;
        }
        
        #pragma omp for simd reduction(+:dsum, lsum)
        for (int i = 0; i < n; i++) {
            dsum += (double)i * 0.25;
            lsum += i * 2L;
        }
    }
    
    /* Taskloop with reduction */
    int prod = 1;
    #pragma omp parallel
    #pragma omp single
    #pragma omp taskloop reduction(*:prod) grainsize(10)
    for (int i = 1; i <= 10; i++) {
        prod *= i;
    }
    
    results[0] = sum;
    results[1] = (int)fsum;
    results[2] = (int)dsum;
    results[3] = lsum;
    results[4] = prod;
}

/* Function 2: Scan operations */
void test_scans(int n, int* array, int* scan_results) {
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        array[i] = i + 1;
        #pragma omp scan exclusive(prefix_sum)
        scan_results[i] = prefix_sum;
        prefix_sum += array[i];
    }
    
    /* Another scan in parallel for */
    int scan2 = 0;
    #pragma omp parallel for reduction(inscan, +:scan2)
    for (int i = 0; i < n; i++) {
        int val = array[i] * 2;
        #pragma omp scan exclusive(scan2)
        scan_results[n + i] = scan2;
        scan2 += val;
    }
}

/* Function 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int n, int* result) {
    int sum = 0;
    
    /* Parallel with non-constant if clause */
    #pragma omp parallel if(cond1 > 0) reduction(+:sum)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
        }
    }
    
    /* Nested parallel with another condition */
    #pragma omp parallel if(cond2 != 0)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp atomic
            result[i] += sum;
        }
    }
    
    /* Combined parallel for with if */
    #pragma omp parallel for if(cond1 && cond2) reduction(+:sum)
    for (int i = 0; i < n/2; i++) {
        sum += i * 2;
    }
}

/* Function 4: Enter data with to mapper */
void test_enter_data(int n, float* data) {
    /* Initialize data on host */
    for (int i = 0; i < n; i++) {
        data[i] = (float)i;
    }
    
    /* Enter data with to mapper */
    #pragma omp enter data map(to: data[0:n])
    
    /* Use the data in target region if supported */
    #ifdef _OPENMP
    #pragma omp target if(0)  /* Disabled but keeps the enter data relevant */
    #endif
    {
        /* Empty - just for structure */
    }
    
    /* Exit data */
    #pragma omp exit data map(from: data[0:n])
}
