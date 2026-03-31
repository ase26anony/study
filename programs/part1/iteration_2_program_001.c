/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double sum;
};

/* Function 1: Scan directives with inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double prefix_sum[N] = {0};
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        #pragma omp for reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
        }
        
        /* Trigger OMP_CLAUSE_INCLUSIVE */
        #pragma omp scan inclusive(local_sum)
        {
            /* Do something with scanned value */
        }
    }
    
    /* Nested scan with exclusive clause */
    #pragma omp parallel
    {
        double scan_var = 0.0;
        #pragma omp for
        for (int i = 1; i < n; i++) {
            #pragma omp scan exclusive(scan_var)
            {
                prefix_sum[i] = scan_var;
                scan_var += arr[i-1];
            }
        }
    }
}

/* Function 2: Enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block->values[0:N]) \
                           map(to: block->indices) \
                           map(alloc: block->sum)
    
    #pragma omp target enter data map(to: block->values[0:N/2]) \
                                  map(to: block->indices[0:N/2])
    
    /* Complex mapping to generate internal temporaries */
    #pragma omp target data map(tofrom: block->values) \
                            map(alloc: block->indices[0:M])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N; i++) {
            block->values[i] *= 2.0;
        }
    }
    
    #pragma omp exit data map(from: block->sum)
}

/* Function 3: Complex loops to generate _LOOPTEMP_, _REDUCTEMP_ */
void test_complex_reductions(double *a, double *b, int n) {
    double sum1 = 0.0, sum2 = 0.0;
    int last_val = 0;
    
    /* Combined clauses to provoke internal temporaries */
    #pragma omp parallel for reduction(+:sum1, sum2) \
                             lastprivate(last_val) \
                             linear(i:1) \
                             schedule(dynamic, 16)
    for (int i = 0; i < n; i++) {
        sum1 += a[i];
        sum2 += b[i];
        last_val = i;
        
        /* Conditional inside loop for _CONDTEMP_ */
        if (a[i] > 0.5) {
            #pragma omp atomic
            sum1 += 0.1;
        }
    }
    
    /* Nested parallelism with reduction */
    #pragma omp parallel if(n > 500) num_threads(4)
    {
        #pragma omp for reduction(+:sum1) nowait
        for (int i = 0; i < n/2; i++) {
            sum1 += a[i] * b[i];
        }
        
        #pragma omp for reduction(max:sum2)
        for (int i = n/2; i < n; i++) {
            if (b[i] > sum2) sum2 = b[i];
        }
    }
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ */
void test_simd_with_conditionals(double *arr, int n) {
    double threshold = 0.5;
    
    #pragma omp simd reduction(+:threshold) \
                       linear(i:1) \
                       simdlen(8)
    for (int i = 0; i < n; i++) {
        /* Complex conditional to generate _CONDTEMP_ */
        double val = arr[i];
        if (val > 0.0) {
            if (val < 1.0) {
                arr[i] = val * val;
            } else {
                arr[i] = 1.0 / val;
            }
        } else {
            arr[i] = -val;
        }
        threshold += 0.001;
    }
    
    /* SIMD with lastprivate */
    double last = 0.0;
    #pragma omp simd lastprivate(last)
    for (int i = 0; i < n; i++) {
        arr[i] += i * 0.01;
        last = arr[i];
    }
}

/* Function 5: Target regions with complex data clauses */
void test_target_regions(struct DataBlock *block) {
    #pragma omp target map(tofrom: block->values[0:N]) \
                       map(to: block->indices[0:N]) \
                       map(alloc: block->sum)
    {
        #pragma omp teams distribute parallel for \
                    reduction(+:block->sum)
        for (int i = 0; i < N; i++) {
            block->values[i] = block->indices[i] * 1.5;
            block->sum += block->values[i];
        }
    }
    
    /* Multiple map clauses with different types */
    #pragma omp target teams distribute parallel for \
                map(to: block->indices) \
                map(from: block->values[100:200]) \
                map(always, tofrom: block->sum)
    for (int i = 0; i < M; i++) {
        block->values[i + 100] = block->indices[i] * 2.0;
    }
}

/* Function 6: Scan with both inclusive and exclusive */
void test_mixed_scan(double *arr, int n) {
    double inclusive_scan = 0.0;
    double exclusive_scan = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            /* Mix of scan directives */
            if (i % 2 == 0) {
                #pragma omp scan inclusive(inclusive_scan)
                {
                    arr[i] += inclusive_scan;
                    inclusive_scan += 1.0;
                }
            } else {
                #pragma omp scan exclusive(exclusive_scan)
                {
                    arr[i] += exclusive_scan;
                    exclusive_scan += 2.0;
                }
            }
        }
    }
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock block;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
        block.values[i] = 0.0;
        block.indices[i] = i;
    }
    block.sum = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call all test functions to trigger different OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(&block);
    test_complex_reductions(array1, array2, N);
    test_simd_with_conditionals(array1, N);
    test_target_regions(&block);
    test_mixed_scan(array2, N);
    
    /* Final reduction for verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                             schedule(guided)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + block.values[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Block sum: %f\n", block.sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
