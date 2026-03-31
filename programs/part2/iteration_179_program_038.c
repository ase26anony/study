/* test_omp_clauses.c */
/* Compile with: gcc -O3 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test_omp_clauses.c -o test_omp_clauses */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target enter(data_array)
static int data_array[1000];

/* Custom reduction for complex cases */
#pragma omp declare reduction(vec_add : int [100] : \
    for (int i = 0; i < 100; i++) \
        omp_out[i] += omp_in[i]) \
    initializer(omp_priv = {0})

/* Function to create non-trivial conditions */
int check_threshold(volatile int *val) {
    return (*val > 100);
}

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    volatile int threshold = 50; /* volatile to prevent optimization */
    int scan_sum = 0;
    int array_sum[100] = {0};
    
    /* Initialize data */
    for (i = 0; i < 1000; i++) {
        data_array[i] = i % 100;
    }
    
    /* Runtime-dependent iteration count */
    int n_iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n_iterations <= 0) n_iterations = 1000;
    
    /* 1. Target region with reduction and if clause - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) reduction(+:sum) \
        if(target: argc > 1) /* Non-trivial condition */
    for (i = 0; i < n_iterations; i++) {
        sum += data_array[i % 1000];
    }
    
    /* Use result to prevent elimination */
    volatile int sink = sum;
    
    /* 2. Parallel for with multiple reductions - may generate multiple _reductemp_ */
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        if(parallel: check_threshold(&threshold)) /* Function call in condition */
    for (i = 0; i < n_iterations; i++) {
        int val = data_array[i % 1000];
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    /* 3. SIMD with inscan reduction - should generate _scantemp_ */
    int partial_sums[10] = {0};
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        num_threads(4)
    for (i = 0; i < n_iterations; i++) {
        int val = data_array[i % 1000];
        
        #pragma omp scan inclusive(scan_sum)
        scan_sum += val;
        
        /* Store partial results */
        if (i % 100 == 0) {
            partial_sums[i / 100 % 10] = scan_sum;
        }
    }
    
    /* 4. Array reduction with custom reducer - complex case */
    int local_arr[100] = {0};
    #pragma omp parallel for reduction(vec_add: array_sum) \
        private(local_arr) schedule(dynamic, 10)
    for (i = 0; i < n_iterations; i++) {
        local_arr[i % 100] = data_array[i % 1000];
        for (int j = 0; j < 100; j++) {
            array_sum[j] += local_arr[j];
        }
    }
    
    /* 5. Nested parallelism with nowait - creates complex scheduling */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < n_iterations/2; i++) {
            sum += i;
        }
        
        #pragma omp for reduction(*:scan_sum) if(parallel: threshold > 25)
        for (i = n_iterations/2; i < n_iterations; i++) {
            scan_sum *= (data_array[i % 1000] + 1);
        }
    }
    
    /* 6. Task with final clause - may generate condition temporaries */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task final(i > 5) /* Conditional final */
                {
                    data_array[i] *= 2;
                }
            }
        }
    }
    
    /* Print results to ensure side effects */
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d\n", 
           sum, max_val, min_val, scan_sum);
    printf("Array sum[0]=%d, partial_sums[5]=%d\n", array_sum[0], partial_sums[5]);
    
    return 0;
}
