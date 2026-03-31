/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int global_array[100];
#pragma omp end declare target

/* Initialize global array on target device */
#pragma omp declare target enter(global_array)

/* Custom reduction for complex cases */
#pragma omp declare reduction(complex_add : int : omp_out = omp_out + omp_in * 2) \
    initializer(omp_priv = 0)

int main(int argc, char **argv) {
    int i;
    volatile int iterations = 1000; /* volatile to prevent optimization */
    
    /* Initialize global array on host */
    for (i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    /* Update target copy */
    #pragma omp target update to(global_array)
    
    /* Test 1: Complex reduction with multiple variables - may generate _reductemp_ */
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int max_val = -1000000, min_val = 1000000;
    
    /* Combined construct with reduction and if clause - may generate both _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2, sum3) \
        reduction(max:max_val) reduction(min:min_val) \
        if(argc > 1) /* Non-trivial condition */
    for (i = 0; i < iterations; i++) {
        int val = i * (argc > 1 ? 2 : 1);
        sum1 += val;
        sum2 += val * 2;
        sum3 += val / 2;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    /* Force side effects */
    printf("Reductions: sum1=%d, sum2=%d, sum3=%d, max=%d, min=%d\n", 
           sum1, sum2, sum3, max_val, min_val);
    
    /* Test 2: Array reduction - may generate additional _reductemp_ */
    int arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (i = 0; i < iterations; i++) {
        arr_sum[i % 10] += i;
    }
    
    /* Test 3: Scan directive - should generate _scantemp_ */
    int scan_sum = 0;
    int scan_results[100];
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10) /* Uneven scheduling */
    for (i = 0; i < 100; i++) {
        /* Exclusive scan */
        #pragma omp scan exclusive(scan_sum)
        scan_results[i] = scan_sum;
        scan_sum += i + 1;
    }
    
    printf("Scan results[0]=%d, [99]=%d, final sum=%d\n", 
           scan_results[0], scan_results[99], scan_sum);
    
    /* Test 4: Nested parallelism with custom reduction */
    int custom_sum = 0;
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for reduction(complex_add:custom_sum) nowait
        for (i = 0; i < iterations; i++) {
            custom_sum += i;
        }
        
        /* Additional work in same parallel region */
        #pragma omp single
        {
            printf("Thread %d in nested region\n", omp_get_thread_num());
        }
    }
    
    printf("Custom reduction sum: %d\n", custom_sum);
    
    /* Test 5: Target region with mapped arrays */
    int host_arr[500];
    int device_sum = 0;
    
    for (i = 0; i < 500; i++) {
        host_arr[i] = i * 3;
    }
    
    #pragma omp target map(to:host_arr) map(tofrom:device_sum) \
        if(iterations > 500) /* Another condition */
    {
        #pragma omp teams distribute parallel for reduction(+:device_sum)
        for (i = 0; i < 500; i++) {
            device_sum += host_arr[i];
        }
    }
    
    printf("Device sum: %d\n", device_sum);
    
    /* Test 6: Conditional compilation with volatile */
    volatile int cond = argc;
    int volatile_sum = 0;
    
    #pragma omp parallel for reduction(+:volatile_sum) \
        if(cond) /* Volatile condition */
    for (i = 0; i < 200; i++) {
        volatile_sum += i * cond;
    }
    
    printf("Volatile sum: %d\n", volatile_sum);
    
    /* Use results to prevent optimization */
    int total = sum1 + sum2 + sum3 + max_val + min_val + arr_sum[0] + 
                scan_sum + custom_sum + device_sum + volatile_sum;
    
    return total == 0 ? 0 : 1;
}
