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
#pragma omp declare reduction(myadd: int: omp_out += omp_in) \
    initializer(omp_priv = 0)

int main(int argc, char **argv) {
    int i;
    volatile int iterations = 1000; /* volatile to prevent optimization */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Initialize arrays */
    int data[1000];
    int scan_data[1000];
    for (i = 0; i < 1000; i++) {
        data[i] = i % 100;
        scan_data[i] = 1;
    }
    
    /* Enter data to target for OMP_CLAUSE_ENTER */
    #pragma omp target enter data map(to:target_data[0:1000])
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    int sum1 = 0, sum2 = 0, sum3 = 0;
    volatile int outer_iter = iterations; /* volatile for condition */
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2, sum3) \
        if(outer_iter > 500) /* May generate _condtemp_ */ \
        map(to:data[0:1000])
    for (i = 0; i < 1000; i++) {
        sum1 += data[i];
        sum2 += data[i] * 2;
        sum3 += data[i] / 2;
    }
    
    /* Use results to prevent elimination */
    printf("Reduction sums: %d, %d, %d\n", sum1, sum2, sum3);
    
    /* 2. Array reduction - may generate complex reduction temporaries */
    int array_sum[10] = {0};
    
    #pragma omp parallel for reduction(+:array_sum[:10]) \
        if(argc > 2) /* Another condition for _condtemp_ */
    for (i = 0; i < 1000; i++) {
        array_sum[i % 10] += data[i];
    }
    
    /* 3. Scan directive - should generate _scantemp_ */
    int scan_sum = 0;
    int scan_result[1000];
    
    #pragma omp parallel for simd \
        reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < 1000; i++) {
        /* Exclusive scan */
        scan_result[i] = scan_sum;
        #pragma omp scan exclusive(scan_sum)
        scan_sum += scan_data[i];
    }
    
    printf("Scan sum final: %d\n", scan_sum);
    
    /* 4. Nested reductions with custom reduction */
    int max_val = -1000000;
    int min_val = 1000000;
    int custom_sum = 0;
    
    #pragma omp parallel sections reduction(max:max_val) \
        reduction(min:min_val) reduction(myadd:custom_sum)
    {
        #pragma omp section
        {
            for (i = 0; i < 500; i++) {
                if (data[i] > max_val) max_val = data[i];
                custom_sum += data[i];
            }
        }
        
        #pragma omp section
        {
            for (i = 500; i < 1000; i++) {
                if (data[i] < min_val) min_val = data[i];
                custom_sum += data[i] * 2;
            }
        }
    }
    
    printf("Max: %d, Min: %d, Custom sum: %d\n", max_val, min_val, custom_sum);
    
    /* 5. Nowait clause with uneven work distribution */
    int partial_sums[4] = {0};
    
    #pragma omp parallel num_threads(4)
    {
        int tid = omp_get_thread_num();
        int start = tid * 250;
        int end = start + 250 + (tid % 2) * 10; /* Uneven work */
        
        for (i = start; i < end && i < 1000; i++) {
            partial_sums[tid] += data[i];
        }
        
        #pragma omp barrier
        
        #pragma omp single nowait
        {
            int total = 0;
            for (i = 0; i < 4; i++) {
                total += partial_sums[i];
            }
            printf("Partial sums total: %d\n", total);
        }
    }
    
    /* 6. Final clause with complex condition */
    int final_result = 0;
    
    #pragma omp parallel for reduction(+:final_result) \
        final(outer_iter < 100) /* May generate condition temporaries */
    for (i = 0; i < 1000; i++) {
        final_result += data[i] % 10;
    }
    
    printf("Final result: %d\n", final_result);
    
    /* Exit target data */
    #pragma omp target exit data map(from:target_data[0:1000])
    
    return 0;
}
