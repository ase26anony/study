#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Test 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int arr[3] = {0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum) reduction(*:product) reduction(-:diff)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            sum += i;
            product *= 1.0f + (i % 10) * 0.01f;
            diff -= 0.5;
        }
        
        /* Nested reduction in task */
        #pragma omp task reduction(+:arr[:3])
        {
            for (int i = 0; i < 3; i++) {
                arr[i] += omp_get_thread_num() + i;
            }
        }
        #pragma omp taskwait
    }
    
    /* Combined construct with reduction */
    #pragma omp parallel for simd reduction(+:sum) reduction(*:product)
    for (int i = 0; i < n; i++) {
        sum += i % 100;
        product *= 1.0f + (i % 5) * 0.02f;
    }
    
    results[0] = sum;
    results[1] = (int)product;
    results[2] = (int)diff;
    results[3] = arr[0] + arr[1] + arr[2];
}

/* Test 2: Scan operations */
void test_scans(int n, int *scan_results) {
    int prefix_sum = 0;
    float prefix_prod = 1.0f;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:prefix_sum) reduction(inscan, *:prefix_prod)
    for (int i = 1; i <= n; i++) {
        prefix_sum += i;
        #pragma omp scan exclusive(prefix_sum)
        prefix_prod *= 1.0f + i * 0.01f;
        #pragma omp scan exclusive(prefix_prod)
        scan_results[i-1] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        int val = (i + 1) * 2;
        prefix_sum += val;
        #pragma omp scan exclusive(prefix_sum)
        scan_results[i] += prefix_sum;
    }
}

/* Test 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int *cond_result) {
    int count = 0;
    
    /* Parallel with non-constant if clause */
    #pragma omp parallel if(cond1 > 0) reduction(+:count)
    {
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            count += i;
        }
    }
    
    /* Nested parallel with conditional */
    #pragma omp parallel if(cond1 || cond2)
    {
        #pragma omp for
        for (int i = 0; i < 50; i++) {
            #pragma omp atomic
            cond_result[0] += i;
        }
        
        /* Task with conditional */
        #pragma omp task if(cond1 && cond2)
        {
            #pragma omp atomic
            cond_result[1] += 1;
        }
        #pragma omp taskwait
    }
    
    cond_result[2] = count;
}

/* Test 4: Enter data with to mapper */
void test_enter_data(int n, float *data) {
    /* Create data to map */
    float *array = (float*)malloc(n * sizeof(float));
    for (int i = 0; i < n; i++) {
        array[i] = i * 1.5f;
    }
    
    /* Trigger OMP_CLAUSE_ENTER with 'to' modifier */
    #pragma omp enter data map(to: array[0:n])
    
    /* Use the data in target region */
    #pragma omp target map(from: data[0:n]) map(to: array[0:n])
    {
        #pragma omp parallel for
        for (int i = 0; i < n; i++) {
            data[i] = array[i] * 2.0f;
        }
    }
    
    #pragma omp exit data map(release: array[0:n])
    free(array);
}
