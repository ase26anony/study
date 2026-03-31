#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern volatile int g_volatile_cond;

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *results) {
    int sum = 0;
    float product = 1.0f;
    double diff = 100.0;
    int array_sum[4] = {0, 0, 0, 0};
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum) reduction(*:product) reduction(-:diff)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            sum += i;
            product *= (i % 10 + 1) * 0.1f;
            diff -= 0.5;
        }
        
        /* Nested reduction in task */
        #pragma omp task reduction(+:array_sum[:4])
        {
            for (int i = 0; i < 4; i++) {
                array_sum[i] += omp_get_thread_num() + i;
            }
        }
        #pragma omp taskwait
    }
    
    /* Combined construct with reduction */
    #pragma omp parallel for simd reduction(+:sum) simdlen(4)
    for (int i = 0; i < n; i++) {
        sum += i * 2;
    }
    
    /* Taskloop reduction */
    #pragma omp taskloop reduction(*:product) nogroup
    for (int i = 1; i <= n; i++) {
        product *= (i % 5 + 1) * 0.01f;
    }
    
    results[0] = sum;
    results[1] = (int)(product * 1000);
    results[2] = (int)diff;
    results[3] = array_sum[0] + array_sum[1] + array_sum[2] + array_sum[3];
}

/* Function 2: Scan operations with inscan reduction */
void test_scans(int n, int *scan_results) {
    int prefix_sum = 0;
    int scan_array[100];
    
    for (int i = 0; i < 100; i++) {
        scan_array[i] = i % 10;
    }
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:prefix_sum) simdlen(8)
    for (int i = 0; i < n && i < 100; i++) {
        prefix_sum += scan_array[i];
        #pragma omp scan exclusive(prefix_sum)
        scan_array[i] = prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n && i < 100; i++) {
        int val = scan_array[i] * 2;
        #pragma omp scan exclusive(prefix_sum)
        prefix_sum += val;
        scan_array[i] = prefix_sum;
    }
    
    scan_results[0] = prefix_sum;
    scan_results[1] = scan_array[0] + scan_array[n-1];
}

/* Function 3: Conditional temporaries with volatile conditions */
void test_conditionals(int n, volatile int cond_var, int *cond_results) {
    int count = 0;
    int total = 0;
    
    /* Parallel region with non-constant if clause */
    #pragma omp parallel if(cond_var > 0) reduction(+:count)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            count++;
        }
        
        /* Nested if with volatile condition */
        #pragma omp sections if(cond_var < 100)
        {
            #pragma omp section
            {
                #pragma omp atomic
                total += 1;
            }
            #pragma omp section
            {
                #pragma omp atomic
                total += 2;
            }
        }
    }
    
    /* Another conditional with function argument */
    #pragma omp parallel if(n > 50) reduction(+:total)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            total += i % 3;
        }
    }
    
    cond_results[0] = count;
    cond_results[1] = total;
}

/* Function 4: Enter data with to mapper */
void test_enter_data(int n, float *data) {
    float *local_data = (float*)malloc(n * sizeof(float));
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        local_data[i] = i * 1.5f;
    }
    
    /* Enter data with to clause */
    #pragma omp enter data map(to: local_data[0:n])
    
    /* Use the data in parallel region */
    #pragma omp target enter data map(to: local_data[0:n])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        local_data[i] *= 2.0f;
    }
    
    #pragma omp target exit data map(from: local_data[0:n])
    
    /* Another enter data with structured block */
    #pragma omp enter data map(to: data[0:n/2])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n/2; i++) {
        data[i] += local_data[i];
    }
    
    #pragma omp exit data map(release: data[0:n/2])
    
    free(local_data);
}
