#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern volatile int g_volatile_cond;

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int* results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int arr_sum[4] = {0, 0, 0, 0};
    
    /* Combined construct with multiple reductions */
    #pragma omp parallel for simd reduction(+:sum) reduction(*:product) \
            reduction(-:diff) reduction(+:arr_sum[:4]) num_threads(4)
    for (int i = 0; i < n; i++) {
        sum += i;
        product *= 1.001f;
        diff -= 0.01;
        arr_sum[i % 4] += i * 2;
    }
    
    /* Nested reduction in taskloop */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskloop reduction(+:sum) nogroup
        for (int i = 0; i < n/2; i++) {
            sum += i * 3;
        }
    }
    
    results[0] = sum;
    results[1] = (int)product;
    results[2] = (int)diff;
    results[3] = arr_sum[0] + arr_sum[1] + arr_sum[2] + arr_sum[3];
}

/* Function 2: Scan operations with inscan reductions */
void test_scans(int n, int* results) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum) simdlen(4)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum;
    }
    
    /* Parallel for with explicit scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        int val = i * 2;
        sum += val;
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = sum - val;
    }
    
    results[0] = sum;
    results[1] = prefix_sum;
}

/* Function 3: Conditional temporaries with volatile conditions */
void test_conditionals(int n, int* results) {
    volatile int v1 = g_volatile_cond;
    volatile int v2 = n > 100 ? 1 : 0;
    int sum = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(v1 > 0) reduction(+:sum) if(v2) num_threads(2)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
        }
    }
    
    /* Nested conditional */
    #pragma omp parallel if(n > 50)
    {
        #pragma omp for if(v1 != 0)
        for (int i = 0; i < n; i++) {
            sum += i * 2;
        }
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
    
    /* Enter data with to clause */
    #pragma omp enter data map(to: data[0:n]) map(to: buffer[:n])
    
    /* Use the data in parallel region */
    #pragma omp target enter data map(to: data[0:n])
    
    #pragma omp target teams distribute parallel for map(tofrom: buffer[:n])
    for (int i = 0; i < n; i++) {
        buffer[i] = data[i] * 2;
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += buffer[i];
    }
    
    #pragma omp exit data map(release: data[0:n], buffer[:n])
    
    free(data);
    free(buffer);
    
    results[0] = sum;
}
