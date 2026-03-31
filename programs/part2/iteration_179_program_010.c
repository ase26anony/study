/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Initialize target data */
#pragma omp declare target enter(target_data)

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int[100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = omp_orig)

/* Main function with various OpenMP constructs */
int main(int argc, char *argv[]) {
    int i, j;
    volatile int iterations = 1000; /* volatile to prevent optimization */
    
    /* Set iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Initialize arrays */
    int data[1000];
    int scan_data[1000];
    int array_reduction[100] = {0};
    volatile int sink = 0; /* volatile sink to prevent elimination */
    
    for (i = 0; i < 1000; i++) {
        data[i] = i % 100;
        scan_data[i] = 1;
    }
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    int sum1 = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        if(argc > 2) /* Non-trivial condition for _condtemp_ */
    for (i = 0; i < iterations; i++) {
        sum1 += data[i];
        sum2 += data[i] * 2;
        if (data[i] > max_val) max_val = data[i];
        if (data[i] < min_val) min_val = data[i];
    }
    
    sink = sum1 + sum2 + max_val + min_val;
    
    /* 2. Array reduction - may generate additional temporaries */
    #pragma omp parallel for reduction(+:array_reduction[:100])
    for (i = 0; i < iterations; i++) {
        int idx = i % 100;
        array_reduction[idx] += data[i];
    }
    
    /* 3. Scan directive - should generate _scantemp_ */
    int scan_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(dynamic) /* Dynamic schedule adds complexity */
    for (i = 0; i < iterations; i++) {
        scan_sum += scan_data[i];
        #pragma omp scan inclusive(scan_sum)
        data[i] = scan_sum; /* Use scan result */
    }
    
    sink += scan_sum;
    
    /* 4. Nested parallelism with custom reduction */
    int custom_array[100] = {0};
    #pragma omp parallel sections reduction(vec_add : custom_array)
    {
        #pragma omp section
        {
            #pragma omp parallel for
            for (i = 0; i < iterations/2; i++) {
                int idx = i % 100;
                #pragma omp atomic
                custom_array[idx] += data[i];
            }
        }
        
        #pragma omp section
        {
            #pragma omp parallel for
            for (i = iterations/2; i < iterations; i++) {
                int idx = i % 100;
                #pragma omp atomic
                custom_array[idx] += data[i];
            }
        }
    }
    
    /* 5. Final clause with volatile condition - may generate _condtemp_ */
    volatile int final_flag = 1;
    #pragma omp parallel for final(final_flag > 0)
    for (i = 0; i < iterations; i++) {
        data[i] *= 2;
    }
    
    /* 6. Nowait clause with reduction - creates scheduling complexity */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum1)
        for (i = 0; i < iterations/2; i++) {
            sum1 += i;
        }
        
        #pragma omp for nowait reduction(+:sum2)
        for (i = iterations/2; i < iterations; i++) {
            sum2 += i;
        }
        
        #pragma omp barrier
    }
    
    /* Use target data that was entered */
    #pragma omp target teams distribute parallel for map(tofrom:target_data)
    for (i = 0; i < 1000; i++) {
        target_data[i] = data[i] % 256;
    }
    
    /* Print results to ensure side effects */
    printf("Results: sum1=%d, sum2=%d, max=%d, min=%d, scan_sum=%d\n",
           sum1, sum2, max_val, min_val, scan_sum);
    printf("Array reduction[0]=%d, custom_array[0]=%d\n", 
           array_reduction[0], custom_array[0]);
    printf("Target data[0]=%d\n", target_data[0]);
    
    return 0;
}
