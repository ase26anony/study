/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global data for declare target enter clause */
int global_data[1000];
#pragma omp declare target enter(global_data)

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd: int: omp_out += omp_in) \
    initializer(omp_priv = 0)

int main(int argc, char *argv[]) {
    /* Use argc to prevent constant folding */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n <= 0) n = 1000;
    
    volatile int sink = 0;  /* Prevent dead code elimination */
    
    /* Initialize arrays */
    for (int i = 0; i < 1000; i++) {
        global_data[i] = i % 100;
    }
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    int sum1 = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        if(argc > 2)  /* Conditional clause - may generate _condtemp_ */
    for (int i = 0; i < n; i++) {
        int idx = i % 1000;
        sum1 += global_data[idx];
        sum2 += global_data[idx] * 2;
        if (global_data[idx] > max_val) max_val = global_data[idx];
        if (global_data[idx] < min_val) min_val = global_data[idx];
    }
    
    sink = sum1 + sum2 + max_val + min_val;
    printf("Reduction results: sum1=%d, sum2=%d, max=%d, min=%d\n", 
           sum1, sum2, max_val, min_val);
    
    /* 2. Scan directive - should generate _scantemp_ */
    int scan_sum = 0;
    int scan_array[100];
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(dynamic)  /* Dynamic schedule adds complexity */
    for (int i = 0; i < 100; i++) {
        scan_array[i] = i;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += scan_array[i];
        scan_array[i] = scan_sum;
    }
    
    sink += scan_sum;
    printf("Scan result: scan_sum=%d\n", scan_sum);
    
    /* 3. Nested parallelism with custom reduction */
    int custom_sum = 0;
    
    #pragma omp parallel sections reduction(myadd:custom_sum)
    {
        #pragma omp section
        {
            #pragma omp parallel for reduction(myadd:custom_sum) \
                if(n > 500)  /* Another conditional */
            for (int i = 0; i < n/2; i++) {
                custom_sum += i % 50;
            }
        }
        
        #pragma omp section
        {
            #pragma omp parallel for reduction(myadd:custom_sum)
            for (int i = n/2; i < n; i++) {
                custom_sum += (i % 50) * 2;
            }
        }
    }
    
    sink += custom_sum;
    printf("Custom reduction result: custom_sum=%d\n", custom_sum);
    
    /* 4. Array reduction (OpenMP 5.1) - complex case */
    int arr_sum[10] = {0};
    
    #pragma omp parallel for reduction(+:arr_sum[:10]) \
        if(scan_sum > 1000)  /* Conditional based on runtime value */
    for (int i = 0; i < n; i++) {
        int idx = i % 10;
        arr_sum[idx] += global_data[i % 1000];
    }
    
    int total_arr_sum = 0;
    for (int i = 0; i < 10; i++) {
        total_arr_sum += arr_sum[i];
    }
    sink += total_arr_sum;
    printf("Array reduction total: %d\n", total_arr_sum);
    
    /* 5. Task reduction with dependency */
    int task_sum = 0;
    
    #pragma omp parallel
    #pragma omp single
    {
        for (int i = 0; i < 10; i++) {
            #pragma omp task reduction(+:task_sum) \
                depend(inout: task_sum)  /* Dependency adds complexity */
            {
                task_sum += i * 100;
            }
        }
        #pragma omp taskwait
    }
    
    sink += task_sum;
    printf("Task reduction result: task_sum=%d\n", task_sum);
    
    /* Final output to ensure all computations are used */
    printf("Final sink value: %d\n", sink);
    
    return 0;
}
