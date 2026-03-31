#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int arr[4] = {0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum) reduction(*:product) \
                         reduction(-:diff) reduction(+:arr[:4])
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sum += i;
            product *= 1.0f + i * 0.001f;
            diff -= 0.5;
            arr[i % 4] += i;
        }
        
        /* Nested reduction in task */
        #pragma omp single
        {
            #pragma omp task reduction(+:sum)
            {
                for (int i = 0; i < 100; i++) {
                    sum += i % 10;
                }
            }
        }
    }
    
    results[0] = sum;
    results[1] = (int)product;
    results[2] = (int)diff;
    results[3] = arr[0] + arr[1] + arr[2] + arr[3];
}

/* Function 2: Scan operations with inscan reductions */
void test_scans(int n, int *output) {
    int sum = 0;
    int prefix_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < n; i++) {
        sum += i + 1;
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum = sum;
        output[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    int scan_array[100];
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
        #pragma omp scan exclusive(scan_array[i])
        scan_array[i] = sum;
    }
}

/* Function 3: Conditional temporaries with volatile conditions */
void test_conditionals(volatile int cond1, volatile int cond2, int *result) {
    int local_sum = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond1 > 0) reduction(+:local_sum)
    {
        #pragma omp for if(cond2 < 100)
        for (int i = 0; i < 1000; i++) {
            local_sum += i;
        }
        
        /* Nested conditional */
        #pragma omp sections if(cond1 + cond2 > 50)
        {
            #pragma omp section
            {
                #pragma omp parallel if(cond1 % 2 == 0) num_threads(2)
                {
                    local_sum += 1;
                }
            }
        }
    }
    
    *result = local_sum;
}

/* Function 4: Enter data with to mapper */
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
    #pragma omp exit data map(release: dynamic_array)
    
    free(dynamic_array);
}

/* Function 5: Combined constructs with multiple temporaries */
void test_combined(int n, volatile int cond, int *results) {
    int red_temp = 0;
    float scan_temp = 0.0f;
    
    /* Combined parallel for with reduction and if clause */
    #pragma omp parallel for simd reduction(+:red_temp) \
                if(cond > 0) reduction(inscan, +:scan_temp)
    for (int i = 0; i < n; i++) {
        red_temp += i;
        scan_temp += i * 0.5f;
        #pragma omp scan inclusive(scan_temp)
        results[i] = red_temp + (int)scan_temp;
    }
    
    /* Taskloop with reduction */
    #pragma omp taskloop reduction(*:red_temp)
    for (int i = 1; i <= 10; i++) {
        red_temp *= i;
    }
}
