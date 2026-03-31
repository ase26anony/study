#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define CHUNK_SIZE 100

/* Function prototypes */
void test_reduction_temporaries(int *data, int size, volatile int cond);
void test_scan_temporaries(float *data, int size);
void test_conditional_temporaries(double *data, int size, volatile int v1, volatile int v2);
void test_enter_data(int *array, int size);

/* Complex reduction patterns across multiple variables */
void test_reduction_temporaries(int *data, int size, volatile int cond) {
    int sum = 0;
    long long product = 1;
    float float_sum = 0.0f;
    int i;
    
    /* Multiple reduction clauses with different operations */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            reduction(+:float_sum) if(cond > 0)
    for (i = 0; i < size; i++) {
        sum += data[i];
        if (data[i] != 0) product *= data[i];
        float_sum += (float)data[i] / 100.0f;
    }
    
    /* Nested reduction pattern */
    #pragma omp parallel
    {
        int local_sum = 0;
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < size/2; i++) {
            local_sum += data[i];
        }
        #pragma omp atomic
        sum += local_sum;
        
        #pragma omp for reduction(*:product)
        for (i = size/2; i < size; i++) {
            if (data[i] != 0) product *= data[i];
        }
    }
    
    /* Taskloop with reduction */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskloop reduction(+:sum) grainsize(CHUNK_SIZE)
        for (i = 0; i < size; i += 2) {
            sum += data[i];
        }
    }
    
    printf("Reduction results: sum=%d, product=%lld, float_sum=%.2f\n", 
           sum, product, float_sum);
}

/* Scan operations with inscan clause */
void test_scan_temporaries(float *data, int size) {
    float prefix_sum = 0.0f;
    float total_sum = 0.0f;
    int i;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:total_sum)
    for (i = 0; i < size; i++) {
        total_sum += data[i];
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = total_sum;
        data[i] = prefix_sum;  /* Store prefix sum */
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:total_sum)
    for (i = 0; i < size; i++) {
        total_sum += data[i] * 2.0f;
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = total_sum;
        data[i] = prefix_sum;
    }
    
    /* Combined construct with scan */
    #pragma omp parallel for simd reduction(inscan, +:total_sum) \
            simdlen(4)
    for (i = 0; i < size; i++) {
        if (i % 2 == 0) {
            total_sum += data[i];
        }
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = total_sum;
    }
    
    printf("Scan results: total_sum=%.2f, prefix_sum=%.2f\n", total_sum, prefix_sum);
}

/* Conditional temporaries with volatile conditions */
void test_conditional_temporaries(double *data, int size, volatile int v1, volatile int v2) {
    double sum = 0.0;
    int i;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel for reduction(+:sum) if(v1 > 0) if(v2 < 100)
    for (i = 0; i < size; i++) {
        sum += data[i];
    }
    
    /* Nested parallel regions with conditions */
    #pragma omp parallel if(v1 != v2)
    {
        #pragma omp for reduction(+:sum) if(v1 > 10)
        for (i = 0; i < size/2; i++) {
            sum += data[i] * 2.0;
        }
        
        #pragma omp sections if(v2 < 50)
        {
            #pragma omp section
            {
                for (i = size/2; i < 3*size/4; i++) {
                    sum += data[i];
                }
            }
            #pragma omp section
            {
                for (i = 3*size/4; i < size; i++) {
                    sum += data[i] * 0.5;
                }
            }
        }
    }
    
    /* Task with conditional */
    #pragma omp parallel if(v1 > 5)
    #pragma omp single
    {
        #pragma omp task if(v2 < 75)
        {
            for (i = 0; i < size; i += 4) {
                sum += data[i];
            }
        }
        #pragma omp taskwait
    }
    
    printf("Conditional results: sum=%.2f (v1=%d, v2=%d)\n", sum, v1, v2);
}

/* Enter data with to clause */
void test_enter_data(int *array, int size) {
    /* Multiple enter data directives with to mapper */
    #pragma omp enter data map(to: array[0:size])
    
    /* Enter data for a subsection */
    int start = size / 4;
    int length = size / 2;
    #pragma omp enter data map(to: array[start:length])
    
    /* Nested enter data in parallel region */
    #pragma omp parallel
    {
        int *local_copy;
        #pragma omp single
        {
            local_copy = (int*)malloc(size * sizeof(int));
            #pragma omp enter data map(to: local_copy[0:size])
        }
        
        #pragma omp for
        for (int i = 0; i < size; i++) {
            local_copy[i] = array[i] * 2;
        }
        
        #pragma omp single
        {
            #pragma omp exit data map(from: local_copy[0:size])
            free(local_copy);
        }
    }
    
    printf("Enter data completed for array of size %d\n", size);
}
