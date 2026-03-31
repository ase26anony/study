/* test_omp_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(complex_reduce : double : \
    omp_out = omp_out * 0.9 + omp_in * 0.1) \
    initializer(omp_priv = 0.0)

/* Function to create runtime-dependent conditions */
int runtime_condition(int argc) {
    volatile int cond = argc; /* volatile to prevent optimization */
    return cond > 2;
}

int main(int argc, char **argv) {
    int i, N = 1000;
    volatile int sink; /* volatile sink to prevent dead code elimination */
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        target_data[i] = i;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* 1. Complex reduction with multiple variables and if clause 
       (may trigger _reductemp_ and _condtemp_) */
    double sum1 = 0.0, sum2 = 0.0;
    double max_val = -1e9, min_val = 1e9;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        if(argc > 1) /* runtime condition */
    for (i = 0; i < N; i++) {
        sum1 += target_data[i];
        sum2 += target_data[i] * 0.5;
        if (target_data[i] > max_val) max_val = target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    sink = (int)sum1; /* Use results */
    
    /* 2. Array reduction (OpenMP 5.1) - may create additional temporaries */
    double arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (i = 0; i < N; i++) {
        arr_sum[i % 10] += target_data[i];
    }
    
    /* 3. SIMD with inscan reduction (triggers _scantemp_) */
    double scan_sum = 0.0;
    double scan_results[N];
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < N; i++) {
        scan_sum += target_data[i];
        #pragma omp scan inclusive(scan_sum)
        scan_results[i] = scan_sum;
    }
    
    sink = (int)scan_results[N-1];
    
    /* 4. Nested parallelism with custom reduction and volatile condition */
    volatile int vol_cond = argc;
    double custom_red = 0.0;
    
    #pragma omp parallel sections reduction(complex_reduce:custom_red) \
        if(runtime_condition(argc))
    {
        #pragma omp section
        {
            for (i = 0; i < N/2; i++) {
                custom_red += target_data[i];
            }
        }
        #pragma omp section
        {
            for (i = N/2; i < N; i++) {
                custom_red += target_data[i] * 2.0;
            }
        }
    }
    
    /* 5. Task with final clause (may create condition temporaries) */
    int task_result = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task final(argc > 3) /* runtime final condition */
        {
            for (i = 0; i < N; i++) {
                #pragma omp atomic
                task_result += target_data[i];
            }
        }
    }
    
    /* 6. Target region with enter/exit data */
    int device_sum = 0;
    #pragma omp target teams distribute parallel for reduction(+:device_sum) \
        map(tofrom: device_sum)
    for (i = 0; i < N; i++) {
        device_sum += target_data[i];
    }
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    /* Print results to ensure side effects */
    printf("Results: sum1=%.2f, sum2=%.2f, max=%.2f, min=%.2f\n", 
           sum1, sum2, max_val, min_val);
    printf("Scan sum=%.2f, Custom reduction=%.2f\n", scan_sum, custom_red);
    printf("Device sum=%d, Task result=%d\n", device_sum, task_result);
    
    return 0;
}
