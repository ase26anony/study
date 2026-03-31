/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -o test_omp test_omp_clauses.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global data for declare target enter clause */
#define N 1000
int data_array[N];
volatile int sink; /* Prevent optimization */

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [N] : \
    for (int i = 0; i < N; i++) out[i] += in[i]) \
    initializer(omp_priv = {0})

/* Declare target with enter clause - triggers OMP_CLAUSE_ENTER */
#pragma omp declare target enter(data_array)

/* Function with volatile parameter to prevent constant folding */
int compute_threshold(volatile int x) {
    return x * 2 + 1;
}

int main(int argc, char *argv[]) {
    int i;
    int sum = 0, sum1 = 0, sum2 = 0;
    int scan_sum = 0;
    int max_val = -1000000;
    int min_val = 1000000;
    int array_sum[N] = {0};
    
    /* Runtime-dependent iteration count */
    int iterations = (argc > 1) ? atoi(argv[1]) : 500;
    if (iterations < 100) iterations = 100;
    if (iterations > N) iterations = N;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data_array[i] = i % 100;
    }
    
    /* Complex condition with function call - may generate _condtemp_ */
    volatile int threshold = 250;
    int cond_val = compute_threshold(threshold);
    
    printf("Starting OpenMP tests with %d iterations\n", iterations);
    
    /* Test 1: target teams with reduction and if clause - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) \
        reduction(max:max_val) \
        if(target: argc > 1) \
        map(tofrom: sum, max_val) \
        map(to: data_array[0:iterations])
    for (i = 0; i < iterations; i++) {
        sum += data_array[i];
        if (data_array[i] > max_val) {
            max_val = data_array[i];
        }
    }
    
    sink = sum; /* Use result to prevent elimination */
    printf("Target reduction sum: %d, max: %d\n", sum, max_val);
    
    /* Test 2: Multiple reductions in single construct - may generate multiple _reductemp_ */
    #pragma omp parallel for reduction(+:sum1, sum2) \
        reduction(min:min_val) \
        if(parallel: cond_val > 300) /* Non-trivial condition */
    for (i = 0; i < iterations; i++) {
        sum1 += data_array[i];
        sum2 += data_array[N - i - 1];
        if (data_array[i] < min_val) {
            min_val = data_array[i];
        }
    }
    
    sink = sum1 + sum2;
    printf("Double reduction sum1: %d, sum2: %d, min: %d\n", sum1, sum2, min_val);
    
    /* Test 3: SIMD with scan directive - generates _scantemp_ */
    scan_sum = 0;
    #pragma omp simd reduction(inscan, +:scan_sum) \
        simdlen(8)
    for (i = 0; i < iterations; i++) {
        #pragma omp scan inclusive(scan_sum)
        scan_sum += data_array[i];
    }
    
    sink = scan_sum;
    printf("Scan reduction result: %d\n", scan_sum);
    
    /* Test 4: Array reduction with custom reducer - complex case */
    #pragma omp parallel for reduction(vec_add: array_sum) \
        schedule(dynamic, 16) /* Uneven work distribution */
    for (i = 0; i < iterations; i++) {
        for (int j = 0; j < N; j++) {
            array_sum[j] += data_array[i] * (j % 10);
        }
    }
    
    /* Test 5: Nested parallelism with nowait - creates complex scheduling */
    #pragma omp parallel num_threads(4)
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < iterations/2; i++) {
            sum += data_array[i] * 2;
        }
        
        #pragma omp for reduction(*:sum1)
        for (i = iterations/2; i < iterations; i++) {
            sum1 *= (data_array[i] + 1);
        }
    }
    
    /* Test 6: Task reduction - another path for temporary generation */
    int task_sum = 0;
    #pragma omp parallel
    #pragma omp single
    {
        for (i = 0; i < 10; i++) {
            #pragma omp task reduction(+:task_sum) \
                if(argc > 2) /* Conditional task creation */
            {
                task_sum += data_array[i * iterations/10];
            }
        }
        #pragma omp taskwait
    }
    
    sink = task_sum;
    printf("Task reduction sum: %d\n", task_sum);
    
    /* Final output to ensure all computations are used */
    printf("Final results - Sum: %d, Sum1: %d, Scan: %d, Task: %d\n", 
           sum, sum1, scan_sum, task_sum);
    
    return 0;
}
