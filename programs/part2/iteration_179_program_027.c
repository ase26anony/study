/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test test_omp_internal_clauses.c */

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
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (iterations < 100) iterations = 1000;
    
    int data[1000];
    int sum = 0, max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int reductemp_trigger = 0;
    
    /* Initialize arrays with non-trivial pattern */
    for (int i = 0; i < 1000; i++) {
        data[i] = (i * 3) % 97;
        target_data[i] = (i * 7) % 113;
    }
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) reduction(min:min_val) \
        if(argc > 2)  /* For _condtemp_ */
    for (int i = 0; i < iterations % 1000; i++) {
        sum += data[i];
        if (data[i] > max_val) max_val = data[i];
        if (data[i] < min_val) min_val = data[i];
    }
    
    /* Force side effect to prevent optimization */
    volatile int sink = sum + max_val + min_val;
    
    /* 2. Array reduction - more likely to generate temporaries */
    int arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (int i = 0; i < iterations % 500; i++) {
        arr_sum[i % 10] += data[i];
    }
    
    /* 3. Scan directive - for _scantemp_ */
    int partial_sums[100] = {0};
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (int i = 0; i < 100; i++) {
        int val = data[i] + i;
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        partial_sums[i] = scan_sum;
    }
    
    /* 4. Nested parallelism with custom reduction */
    int custom_sum = 0;
    #pragma omp parallel sections reduction(myadd:custom_sum)
    {
        #pragma omp section
        {
            #pragma omp parallel for reduction(myadd:custom_sum)
            for (int i = 0; i < 200; i++) {
                custom_sum += data[i];
            }
        }
        #pragma omp section
        {
            #pragma omp parallel for reduction(myadd:custom_sum)
            for (int i = 200; i < 400; i++) {
                custom_sum += target_data[i % 1000];
            }
        }
    }
    
    /* 5. Nowait with uneven work distribution */
    int nowait_sum1 = 0, nowait_sum2 = 0;
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:nowait_sum1)
        for (int i = 0; i < 300; i++) {
            nowait_sum1 += data[i];
        }
        
        #pragma omp for nowait reduction(+:nowait_sum2)
        for (int i = 300; i < 600; i++) {
            nowait_sum2 += data[i] * 2;
        }
        
        #pragma omp barrier
    }
    
    /* 6. Conditional compilation with volatile */
    volatile int cond = (argc > 3);
    int cond_sum = 0;
    #pragma omp parallel for reduction(+:cond_sum) if(cond)
    for (int i = 0; i < 100; i++) {
        cond_sum += data[i];
    }
    
    /* 7. SIMD with multiple reductions */
    double dsum = 0.0, dmax = -1e30;
    #pragma omp simd reduction(+:dsum) reduction(max:dmax)
    for (int i = 0; i < 100; i++) {
        double val = data[i] * 0.5;
        dsum += val;
        if (val > dmax) dmax = val;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: sum=%d, max=%d, min=%d\n", sum, max_val, min_val);
    printf("Scan sum=%d, custom=%d\n", scan_sum, custom_sum);
    printf("Nowait sums: %d, %d\n", nowait_sum1, nowait_sum2);
    printf("Conditional sum=%d\n", cond_sum);
    printf("Double sum=%.2f, max=%.2f\n", dsum, dmax);
    
    /* Array reduction result */
    int total_arr_sum = 0;
    for (int i = 0; i < 10; i++) total_arr_sum += arr_sum[i];
    printf("Array reduction sum=%d\n", total_arr_sum);
    
    /* Partial scan results */
    printf("Partial sums[0]=%d, [99]=%d\n", partial_sums[0], partial_sums[99]);
    
    return 0;
}
