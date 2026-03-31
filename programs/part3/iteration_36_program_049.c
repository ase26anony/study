#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern void test_reductions(int n, int *results);
extern void test_scans(int n, float *results);
extern void test_conditionals(volatile int cond1, volatile int cond2);
extern void test_enter_data(int n, double *array);

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    long long product = 1;
    float float_sum = 0.0f;
    int array_sum[4] = {0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum, float_sum) reduction(*:product)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            sum += i;
            product *= (i % 10) + 1;  /* Avoid overflow */
            float_sum += i * 0.5f;
        }
        
        /* Nested reduction in task */
        #pragma omp task reduction(+:array_sum[:4])
        {
            for (int i = 0; i < n; i++) {
                array_sum[i % 4] += i;
            }
        }
        #pragma omp taskwait
    }
    
    /* Combined construct with reduction */
    #pragma omp parallel for simd reduction(+:sum) reduction(+:float_sum)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
        float_sum += i * 0.25f;
    }
    
    /* Taskloop reduction */
    #pragma omp taskloop reduction(*:product) num_tasks(4)
    for (int i = 1; i <= n; i++) {
        product *= (i % 5) + 1;
    }
    
    results[0] = sum;
    results[1] = (int)product;
    results[2] = (int)float_sum;
    results[3] = array_sum[0] + array_sum[1] + array_sum[2] + array_sum[3];
}

/* Function 2: Scan operations */
void test_scans(int n, float *results) {
    float prefix_sum = 0.0f;
    float inscan_sum = 0.0f;
    float array[100];
    
    for (int i = 0; i < 100; i++) {
        array[i] = (float)i;
    }
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:inscan_sum)
    for (int i = 0; i < n && i < 100; i++) {
        inscan_sum += array[i];
        #pragma omp scan exclusive(inscan_sum)
        array[i] = inscan_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n && i < 100; i++) {
        prefix_sum += array[i];
        #pragma omp scan inclusive(prefix_sum)
        results[i] = prefix_sum;
    }
    
    /* Another scan pattern */
    float scan_array[50];
    #pragma omp parallel for simd reduction(inscan, +:inscan_sum)
    for (int i = 0; i < 50; i++) {
        scan_array[i] = (float)i * 2.0f;
        #pragma omp scan exclusive(inscan_sum)
        inscan_sum += scan_array[i];
    }
}

/* Function 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2) {
    int sum = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond1 > 0) reduction(+:sum)
    {
        #pragma omp for if(cond2 != 0)
        for (int i = 0; i < 100; i++) {
            sum += i;
        }
    }
    
    /* Nested conditionals */
    #pragma omp parallel if(cond1 + cond2 > 5) num_threads(4)
    {
        #pragma omp for if(cond1 % 2 == 0)
        for (int i = 0; i < 50; i++) {
            #pragma omp atomic
            sum += i;
        }
    }
    
    /* Conditional in task */
    #pragma omp task if(cond1 > cond2)
    {
        for (int i = 0; i < 10; i++) {
            sum += i;
        }
    }
    #pragma omp taskwait
}

/* Function 4: Enter data with to clause */
void test_enter_data(int n, double *array) {
    /* Multiple enter data directives with to mapper */
    #pragma omp enter data map(to: array[0:n])
    
    /* Combined with other clauses */
    #pragma omp target enter data map(to: array[0:n/2]) if(n > 100)
    
    /* Multiple arrays */
    double secondary[50];
    #pragma omp enter data map(to: secondary[0:50])
    
    /* Use the data */
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n && i < 1000; i++) {
        array[i] = array[i] * 2.0;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: array[0:n])
    #pragma omp exit data map(release: secondary[0:50])
}
