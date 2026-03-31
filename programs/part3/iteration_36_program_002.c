#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Test function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    int arr[4] = {0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum, fsum, dsum) reduction(+:arr[:4])
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            fsum += i * 0.5f;
            dsum += i * 0.25;
            arr[i % 4] += i;
        }
        
        /* Nested reduction in task */
        #pragma omp single
        {
            #pragma omp task reduction(*:sum)
            {
                sum = (sum > 0) ? sum : 1;
                for (int j = 0; j < 10; j++) {
                    sum *= 2;
                }
            }
        }
    }
    
    /* Combined construct with reduction */
    #pragma omp parallel for simd reduction(+:sum) collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
    }
    
    results[0] = sum;
    results[1] = (int)fsum;
    results[2] = (int)dsum;
    results[3] = arr[0] + arr[1] + arr[2] + arr[3];
}

/* Test function 2: Scan operations */
void test_scans(int n, int *scan_results) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        int val = i * 2;
        sum += val;
        #pragma omp scan inclusive(sum)
    }
    
    scan_results[0] = sum;
    scan_results[1] = prefix_sum;
}

/* Test function 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int *cond_results) {
    int count = 0;
    
    /* Parallel with non-constant if clause */
    #pragma omp parallel if(cond1 > 0) reduction(+:count)
    {
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            count += i;
        }
        
        /* Nested parallel with another condition */
        #pragma omp parallel if(cond2 < 100) shared(count)
        {
            #pragma omp atomic
            count++;
        }
    }
    
    /* Taskloop with condition */
    #pragma omp taskloop if(cond1 != cond2) reduction(+:count)
    for (int i = 0; i < 50; i++) {
        count += i * 2;
    }
    
    cond_results[0] = count;
}

/* Test function 4: Enter data with to mapper */
void test_enter_data(int n, float *data, int *enter_result) {
    float *local_data = (float*)malloc(n * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        local_data[i] = i * 1.5f;
    }
    
    /* Enter data with to clause */
    #pragma omp enter data map(to: local_data[0:n])
    
    /* Use the data in parallel region */
    float sum = 0.0f;
    #pragma omp target data map(tofrom: sum) map(to: local_data[0:n])
    #pragma omp target teams distribute parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += local_data[i];
    }
    
    /* Exit data */
    #pragma omp exit data map(from: local_data[0:n])
    
    *enter_result = (int)sum;
    free(local_data);
}
