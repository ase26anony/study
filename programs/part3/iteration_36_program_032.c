#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Test function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double prod_double = 1.0;
    int array_sum[5] = {0, 0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum_int, sum_float) reduction(*:prod_double)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            sum_int += i;
            sum_float += i * 0.5f;
            if (i > 0) prod_double *= 1.0001;
        }
        
        /* Nested reduction in task */
        #pragma omp task reduction(+:array_sum[:5])
        {
            for (int i = 0; i < 5; i++) {
                array_sum[i] += omp_get_thread_num() + i;
            }
        }
        #pragma omp taskwait
    }
    
    /* Combined construct with reduction */
    #pragma omp parallel for simd reduction(+:sum_int) collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum_int += i * j;
        }
    }
    
    results[0] = sum_int;
    results[1] = (int)sum_float;
    results[2] = (int)prod_double;
    results[3] = array_sum[0];
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
        #pragma omp scan exclusive(prefix_sum)
        {
            prefix_sum = sum;
            scan_results[i] = prefix_sum;
        }
    }
    
    scan_results[n] = sum;
}

/* Test function 3: Conditional temporaries */
void test_conditionals(volatile int cond1, volatile int cond2, int *result) {
    int local_sum = 0;
    
    /* Parallel with volatile condition - forces condtemp */
    #pragma omp parallel if(cond1 > 0) reduction(+:local_sum)
    {
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            local_sum += i;
        }
        
        /* Nested conditional */
        #pragma omp sections if(cond2 < 10)
        {
            #pragma omp section
            {
                local_sum += 1000;
            }
            #pragma omp section
            {
                local_sum += 2000;
            }
        }
    }
    
    *result = local_sum;
}

/* Test function 4: Enter data with 'to' mapper */
void test_enter_data(int N) {
    int *array = (int*)malloc(N * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < N; i++) {
        array[i] = i;
    }
    
    /* This should trigger OMP_CLAUSE_ENTER with to modifier */
    #pragma omp enter data map(to: array[0:N])
    
    /* Use the array in parallel region */
    #pragma omp target enter data map(to: array[0:N])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        array[i] *= 2;
    }
    
    #pragma omp target exit data map(from: array[0:N])
    #pragma omp exit data map(release: array)
    
    free(array);
}
