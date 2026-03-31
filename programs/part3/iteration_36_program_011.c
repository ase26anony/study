#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern volatile int g_volatile_cond;

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int array_sum[5] = {0};
    
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
            #pragma omp task reduction(+:sum)
            {
                for (int i = 0; i < 100; i++) {
                    sum += i % 5;
                }
            }
        }
    }
    
    /* Reduction on array elements */
    #pragma omp parallel for reduction(+:array_sum[:5])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 5; j++) {
            array_sum[j] += (i + j) % 10;
        }
    }
    
    /* Taskloop reduction */
    #pragma omp taskloop reduction(*:product)
    for (int i = 1; i <= n; i++) {
        product *= 1.0f + (i % 7) * 0.005f;
    }
    
    results[0] = sum;
    results[1] = (int)product;
    results[2] = (int)diff;
    results[3] = array_sum[0];
}

/* Function 2: Scan operations */
void test_scans(int n, int *output) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum;
        output[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        int val = (i * 3) % 17;
        sum += val;
        
        #pragma omp scan exclusive(prefix_sum)
        {
            output[i] = prefix_sum;
            prefix_sum = sum;
        }
    }
}

/* Function 3: Conditional clauses with volatile */
void test_conditionals(int n, int *data) {
    volatile int local_volatile = g_volatile_cond;
    
    /* Parallel with if clause using volatile */
    #pragma omp parallel if(local_volatile > 0) num_threads(4)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
    }
    
    /* Combined parallel for with if */
    #pragma omp parallel for if(local_volatile < 100) schedule(dynamic)
    for (int i = 0; i < n; i++) {
        data[i] += i;
    }
    
    /* Nested conditionals */
    #pragma omp parallel if(local_volatile != 0)
    {
        #pragma omp for if(local_volatile % 2 == 0)
        for (int i = 0; i < n; i++) {
            data[i] = data[i] % 256;
        }
    }
}

/* Function 4: Enter data with to mapper */
void test_enter_data(int n, float *array) {
    /* Enter data with to clause */
    #pragma omp enter data map(to: array[0:n])
    
    /* Use the data in parallel region */
    #pragma omp target enter data map(to: array[0:n])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        array[i] = array[i] * 2.0f + (float)i;
    }
    
    #pragma omp target exit data map(from: array[0:n])
    
    /* Another enter data with structured block */
    #pragma omp enter data map(to: array[n/2:n/2])
    
    #pragma omp target teams distribute parallel for
    for (int i = n/2; i < n; i++) {
        array[i] = array[i] / 2.0f;
    }
    
    #pragma omp target exit data map(from: array[n/2:n/2])
    #pragma omp exit data map(release: array[0:n])
}

/* Function 5: Mixed complex patterns */
void test_mixed_patterns(int n, int *results) {
    volatile int cond = g_volatile_cond;
    int scan_temp = 0;
    
    /* Combined construct with reduction and if */
    #pragma omp parallel for simd reduction(+:results[0]) if(cond > 50) schedule(static)
    for (int i = 0; i < n; i++) {
        results[0] += i * i;
    }
    
    /* Reduction with scan in nested loop */
    #pragma omp parallel reduction(+:results[1])
    {
        #pragma omp for reduction(inscan, +:scan_temp)
        for (int i = 0; i < n; i++) {
            scan_temp += i % 7;
            #pragma omp scan exclusive(results[2])
            results[2] = scan_temp;
        }
        results[1] = scan_temp;
    }
}
