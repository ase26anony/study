/* test_omp_clause_coverage.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -c test_omp_clause_coverage.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target enter(data_array)
static int data_array[1000];

/* Custom reduction for complex cases */
#pragma omp declare reduction(my_add : int : omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create non-trivial condition */
int check_threshold(int argc) {
    volatile int threshold = 500; /* volatile to prevent optimization */
    return argc > threshold;
}

int main(int argc, char *argv[]) {
    int i;
    int sum = 0, sum1 = 0, sum2 = 0;
    int max_val = -1000000, min_val = 1000000;
    int scan_sum = 0;
    int array_sum = 0;
    
    /* Runtime-dependent iteration count */
    int N = 1000;
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 100;
    
    /* Initialize data */
    for (i = 0; i < 1000; i++) {
        data_array[i] = i % 100;
    }
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) \
        reduction(min:min_val) \
        if(argc > 1)  /* Non-trivial condition for _condtemp_ */
    for (i = 0; i < N; i++) {
        sum1 += data_array[i % 1000];
        sum2 += data_array[(i + 1) % 1000];
        if (data_array[i % 1000] > max_val) max_val = data_array[i % 1000];
        if (data_array[i % 1000] < min_val) min_val = data_array[i % 1000];
    }
    
    /* Volatile sink to prevent optimization */
    volatile int sink1 = sum1 + sum2 + max_val + min_val;
    
    /* 2. Scan directive - should generate _scantemp_ */
    int partial_sums[1000];
    #pragma omp parallel for simd \
        reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < N; i++) {
        int val = data_array[i % 1000];
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] = scan_sum;
        scan_sum += val;
    }
    
    /* 3. Array reduction (OpenMP 5.1) - may generate additional temporaries */
    int arr[100] = {0};
    #pragma omp parallel for reduction(+:arr[:50])
    for (i = 0; i < N; i++) {
        arr[i % 50] += data_array[i % 1000];
    }
    
    /* 4. Custom reduction with volatile condition */
    volatile int volatile_flag = argc;
    #pragma omp parallel for reduction(my_add:array_sum) \
        if(check_threshold(argc))  /* Function call in condition */
    for (i = 0; i < N; i++) {
        array_sum += data_array[i % 1000];
    }
    
    /* 5. Nested parallelism with reduction */
    #pragma omp parallel
    {
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < N; i++) {
            sum += data_array[i % 1000];
        }
        
        #pragma omp for reduction(max:max_val)
        for (i = 0; i < N/2; i++) {
            if (data_array[i % 1000] > max_val) 
                max_val = data_array[i % 1000];
        }
    }
    
    /* 6. Target region with enter clause usage */
    #pragma omp target map(tofrom: sum)
    {
        #pragma omp teams distribute parallel for reduction(+:sum)
        for (i = 0; i < 100; i++) {
            sum += data_array[i];
        }
    }
    
    /* Print results to ensure side effects */
    printf("Results: sum=%d, sum1=%d, sum2=%d, max=%d, min=%d, scan_sum=%d, array_sum=%d\n",
           sum, sum1, sum2, max_val, min_val, scan_sum, array_sum);
    
    return 0;
}
