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
    #pragma omp parallel reduction(+:sum, fsum, dsum) \
                         reduction(+:arr[:10])
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
                sum = sum * 2;
            }
        }
    }
    
    results[0] = sum;
    results[1] = (int)fsum;
    results[2] = (int)dsum;
    results[3] = arr[0];
}

/* Function 2: Scan operations */
void test_scans(int n, int* results) {
    int sum = 0;
    int scan_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan exclusive(scan_sum)
        scan_sum += i;
        sum += i * 2;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(sum)
        sum += i;
    }
    
    results[0] = sum;
    results[1] = scan_sum;
}

/* Function 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int n, int* results) {
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
            for (int i = 0; i < n; i++) {
                sum += i * 2;
            }
        }
    }
    
    /* Taskloop with condition */
    #pragma omp taskloop if(cond1 != cond2) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += i * 3;
    }
    
    results[0] = sum;
}

/* Function 4: Enter data with to mapper */
void test_enter_data(int n, int* results) {
    int* data = (int*)malloc(n * sizeof(int));
    int* buffer = (int*)malloc(n * sizeof(int));
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        data[i] = i;
        buffer[i] = 0;
    }
    
    /* Enter data with to mapper - should trigger OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: data[0:n])
    
    /* Also test with array section */
    #pragma omp enter data map(to: buffer[0:n/2])
    
    /* Use the data in parallel region */
    #pragma omp target enter data map(to: data[0:n])
    
    int sum = 0;
    #pragma omp target teams distribute parallel for reduction(+:sum) \
                     map(tofrom: data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
        sum += data[i];
    }
    
    #pragma omp exit data map(from: data[0:n])
    
    results[0] = sum;
    
    free(data);
    free(buffer);
}
