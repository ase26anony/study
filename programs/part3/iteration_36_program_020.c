#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define CHUNK_SIZE 128

/* Test function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int i, j;
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    int product = 1;
    int array_sum[5] = {0, 0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum_int, sum_float, sum_double) \
                         reduction(*:product) \
                         reduction(+:array_sum[:5])
    {
        #pragma omp for nowait
        for (i = 0; i < n; i++) {
            sum_int += i;
            sum_float += (float)i * 0.5f;
            sum_double += (double)i * 0.25;
            product *= (i % 10) + 1;  /* Avoid overflow */
            for (j = 0; j < 5; j++) {
                array_sum[j] += i + j;
            }
        }
        
        /* Nested reduction in task */
        #pragma omp task reduction(+:sum_int)
        {
            for (i = 0; i < 10; i++) {
                sum_int += i * 2;
            }
        }
    }
    
    results[0] = sum_int;
    results[1] = (int)sum_float;
    results[2] = (int)sum_double;
    results[3] = product;
    results[4] = array_sum[0];
}

/* Test function 2: Scan operations with inscan reductions */
void test_scans(int n, int *input, int *output_prefix, int *output_exclusive) {
    int i;
    int prefix_sum = 0;
    int exclusive_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:prefix_sum)
    for (i = 0; i < n; i++) {
        prefix_sum += input[i];
        #pragma omp scan inclusive(prefix_sum)
        output_prefix[i] = prefix_sum;
    }
    
    /* For loop with explicit scan directive */
    #pragma omp parallel for reduction(inscan, +:exclusive_sum)
    for (i = 0; i < n; i++) {
        exclusive_sum += input[i];
        #pragma omp scan exclusive(exclusive_sum)
        output_exclusive[i] = exclusive_sum - input[i];
    }
}

/* Test function 3: Conditional temporaries with volatile conditions */
void test_conditionals(int n, volatile int cond1, volatile int cond2, int *result) {
    int i;
    int local_sum = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond1 > 0) reduction(+:local_sum)
    {
        #pragma omp for if(cond2 < n)
        for (i = 0; i < n; i++) {
            local_sum += i;
        }
        
        /* Nested conditional */
        #pragma omp sections if(cond1 + cond2 > 10)
        {
            #pragma omp section
            {
                local_sum += 100;
            }
            #pragma omp section
            {
                local_sum += 200;
            }
        }
    }
    
    *result = local_sum;
}

/* Test function 4: Enter data with 'to' mapper */
void test_enter_data(int n, float *data) {
    float *buffer = (float *)malloc(n * sizeof(float));
    
    /* Initialize buffer on host */
    for (int i = 0; i < n; i++) {
        buffer[i] = (float)i * 1.5f;
    }
    
    /* Use enter data with to clause */
    #pragma omp enter data map(to: buffer[0:n])
    
    /* Use the data in target region */
    #pragma omp target map(from: data[0:n]) map(tofrom: buffer[0:n])
    {
        for (int i = 0; i < n; i++) {
            data[i] = buffer[i] * 2.0f;
        }
    }
    
    /* Exit data */
    #pragma omp exit data map(release: buffer[0:n])
    
    free(buffer);
}
