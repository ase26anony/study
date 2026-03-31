/* test_omp_internal_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target enter(data_array)
static int data_array[1000];

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd: int: omp_out += omp_in) \
    initializer(omp_priv = 0)

/* Function to create non-trivial conditions */
int check_threshold(int val) {
    volatile int limit = 500; /* volatile to prevent optimization */
    return val > limit;
}

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    
    /* Use argc to prevent compile-time optimization */
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 100;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data_array[i] = i % 100;
    }
    
    /* 1. Target region with reduction and if clause - may generate _reductemp_ and _condtemp_ */
    int target_sum = 0;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: target_sum) reduction(+:target_sum) \
        if(target: argc > 2) /* Non-trivial condition */
    for (i = 0; i < N; i++) {
        target_sum += data_array[i];
    }
    
    /* Volatile sink to prevent elimination */
    volatile int sink1 = target_sum;
    
    /* 2. Parallel region with multiple reductions - may generate _reductemp_ */
    #pragma omp parallel for reduction(+:sum) reduction(max:max_val) reduction(min:min_val) \
        if(parallel: check_threshold(N)) /* Function call in condition */
    for (i = 0; i < N; i++) {
        sum += data_array[i];
        if (data_array[i] > max_val) max_val = data_array[i];
        if (data_array[i] < min_val) min_val = data_array[i];
    }
    
    /* 3. SIMD region with inscan reduction - explicitly generates _scantemp_ */
    int partial_sums[100];
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(static, 10) /* Uneven scheduling */
    for (i = 0; i < N; i++) {
        int val = data_array[i];
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        partial_sums[i % 100] = scan_sum;
    }
    
    /* 4. Nested parallelism with custom reduction */
    int custom_sum = 0;
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for reduction(myadd:custom_sum) nowait
        for (i = 0; i < N; i++) {
            custom_sum += data_array[i] * 2;
        }
    }
    
    /* 5. Array reduction (OpenMP 5.1) - may generate complex temporaries */
    int arr_reduce[10] = {0};
    #pragma omp parallel for reduction(+:arr_reduce[:10])
    for (i = 0; i < N; i++) {
        arr_reduce[i % 10] += data_array[i];
    }
    
    /* 6. Taskloop with reduction and if clause */
    int task_sum = 0;
    #pragma omp taskloop reduction(+:task_sum) if(argc > 1) \
        grainsize(10) num_tasks(20)
    for (i = 0; i < N; i++) {
        task_sum += data_array[i] / 2;
    }
    
    /* Print results to ensure side effects */
    printf("Results:\n");
    printf("  Target sum: %d\n", target_sum);
    printf("  Sum: %d, Max: %d, Min: %d\n", sum, max_val, min_val);
    printf("  Scan sum: %d\n", scan_sum);
    printf("  Custom sum: %d\n", custom_sum);
    printf("  Task sum: %d\n", task_sum);
    printf("  Array reduce[0]: %d\n", arr_reduce[0]);
    
    /* Use volatile to force memory operations */
    volatile int final_check = target_sum + sum + scan_sum + custom_sum + task_sum;
    
    return 0;
}
