#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Test 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int arr[10] = {0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum) reduction(*:product) reduction(-:diff)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            product *= 1.001f;
            diff -= 0.01;
        }
        
        /* Nested reduction in task */
        #pragma omp single
        {
            #pragma omp task reduction(+:arr[0:5])
            for (int i = 0; i < 5; i++) {
                arr[i] += omp_get_thread_num();
            }
        }
    }
    
    /* Combined construct reduction */
    #pragma omp parallel for simd reduction(+:sum) reduction(*:product)
    for (int i = 0; i < n; i++) {
        sum += i % 10;
        product *= 1.0001f;
    }
    
    /* Taskloop reduction */
    #pragma omp taskloop reduction(*:product)
    for (int i = 1; i <= n; i++) {
        product *= 1.0f + (i * 0.0001f);
    }
    
    results[0] = sum;
    results[1] = (int)product;
    results[2] = (int)diff;
    results[3] = arr[0];
}

/* Test 2: Scan operations */
void test_scans(int n, int *output) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i;
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum = sum;
        output[i] = prefix_sum;
    }
    
    /* For loop with scan directive */
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        int val = i * 2;
        sum += val;
        #pragma omp scan exclusive(prefix_sum)
        {
            prefix_sum = sum;
            output[n + i] = prefix_sum;
        }
    }
}

/* Test 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int *result) {
    int local_sum = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond1 > 0) reduction(+:local_sum)
    {
        #pragma omp for if(cond2 != 0)
        for (int i = 0; i < 100; i++) {
            local_sum += i;
        }
        
        /* Nested conditional */
        #pragma omp sections if(cond1 + cond2 > 5)
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
    
    /* Parallel with complex conditional expression */
    #pragma omp parallel if(cond1 && cond2) if(cond1 || cond2)
    {
        #pragma omp atomic
        local_sum += 1;
    }
    
    *result = local_sum;
}

/* Test 4: Enter data with 'to' mapper */
void test_enter_data(int n) {
    int *dynamic_array = (int*)malloc(n * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        dynamic_array[i] = i;
    }
    
    /* Use enter data with to clause */
    #pragma omp enter data map(to: dynamic_array[0:n])
    
    /* Use the data in parallel region */
    #pragma omp target enter data map(to: dynamic_array[0:n])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        dynamic_array[i] *= 2;
    }
    
    #pragma omp target exit data map(from: dynamic_array[0:n])
    
    /* Another enter data variant */
    #pragma omp enter data map(to: dynamic_array) if(n > 100)
    
    #pragma omp exit data map(release: dynamic_array)
    
    free(dynamic_array);
}
