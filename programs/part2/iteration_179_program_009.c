/* test_omp_clauses.c - Trigger internal OpenMP temporary clause generation */

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

/* Function to create non-trivial condition */
int check_threshold(volatile int val, int limit) {
    return val > limit;
}

int main(int argc, char **argv) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    volatile int cond_temp = 0;  /* volatile to prevent optimization */
    int scan_sum = 0;
    int array_sum[100] = {0};
    
    /* Runtime-dependent iteration count */
    int N = 1000;
    if (argc > 1) N = atoi(argv[1]);
    if (N < 100) N = 100;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        data_array[i] = i % 100;
    }
    
    /* 1. Complex target region with reduction and condition - may generate _reductemp_ and _condtemp_ */
    /* OMP_CLAUSE__REDUCTEMP_ and OMP_CLAUSE__CONDTEMP_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 2) map(tofrom: sum, max_val) map(to: data_array[0:N])
    for (i = 0; i < N; i++) {
        sum += data_array[i];
        if (data_array[i] > max_val) max_val = data_array[i];
    }
    
    printf("Target region: sum = %d, max = %d\n", sum, max_val);
    
    /* 2. Parallel region with multiple reductions and complex condition */
    /* OMP_CLAUSE__REDUCTEMP_ */
    #pragma omp parallel for reduction(+:sum) reduction(min:min_val) \
        if(parallel: check_threshold(N, 500)) \
        schedule(dynamic, 16) nowait
    for (i = 0; i < N; i++) {
        sum += i;
        if (i < min_val) min_val = i;
    }
    
    printf("Parallel region: sum = %d, min = %d\n", sum, min_val);
    
    /* 3. SIMD region with inscan reduction - generates OMP_CLAUSE__SCANTEMP_ */
    /* OMP_CLAUSE__SCANTEMP_ */
    #pragma omp simd reduction(inscan, +:scan_sum) simdlen(8)
    for (i = 0; i < N; i++) {
        #pragma omp scan inclusive(scan_sum)
        scan_sum += data_array[i];
    }
    
    printf("Scan sum = %d\n", scan_sum);
    
    /* 4. Array reduction with custom reduction - may generate additional temporaries */
    /* OMP_CLAUSE__REDUCTEMP_ */
    #pragma omp parallel for reduction(vec_add: array_sum)
    for (i = 0; i < N; i++) {
        int idx = i % 100;
        array_sum[idx] += data_array[i];
    }
    
    /* 5. Nested parallelism with condition */
    omp_set_nested(1);
    #pragma omp parallel if(argc > 1) num_threads(2)
    {
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < N/2; i++) {
            sum += i;
        }
        
        #pragma omp single
        {
            #pragma omp task if(argc > 3)
            {
                volatile int task_sum = 0;
                #pragma omp simd reduction(+:task_sum)
                for (i = 0; i < 100; i++) {
                    task_sum += array_sum[i];
                }
                printf("Task sum = %d\n", task_sum);
            }
        }
    }
    
    /* 6. Workshare with reduction */
    #pragma omp parallel
    {
        #pragma omp sections reduction(+:sum)
        {
            #pragma omp section
            {
                for (i = 0; i < N/4; i++) sum += 1;
            }
            #pragma omp section
            {
                for (i = 0; i < N/4; i++) sum += 2;
            }
        }
    }
    
    printf("Final sum = %d\n", sum);
    
    return 0;
}
