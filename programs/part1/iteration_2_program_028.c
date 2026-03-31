/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* Structure for complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void scan_test(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum) private(exclusive_prefix)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(partial_sum)
        arr[i] += partial_sum;
        partial_sum += arr[i];
        
        if (i % 2 == 0) {
            #pragma omp scan exclusive(exclusive_prefix)
            arr[i] -= exclusive_prefix;
            exclusive_prefix += 1.0;
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void enter_data_test(struct DataBlock *block) {
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    #pragma omp target enter data map(to: block->values[N/2:N/2]) \
                                  map(to: block->indices[N/2:N/2])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 0.5;
        block->indices[i] = i;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values)
}

/* Function 3: Complex nested loops with reduction, lastprivate, linear */
void complex_reduction_test(double *a, double *b, int n, int m) {
    double total = 0.0;
    int last_i = 0, last_j = 0;
    
    /* This complex construct may generate _LOOPTEMP_ and _REDUCTEMP_ clauses */
    #pragma omp parallel for collapse(2) reduction(+:total) \
                lastprivate(last_i, last_j) linear(j:1) private(i, j)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            a[i * m + j] = i * j * 0.1;
            b[i * m + j] = i + j;
            total += a[i * m + j];
            last_i = i;
            last_j = j;
        }
    }
    
    /* Additional parallel region with if clause */
    #pragma omp parallel if(n > 500) num_threads(4)
    {
        #pragma omp for reduction(+:total) nowait
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                total += b[i * m + j];
            }
        }
        
        #pragma omp single
        {
            printf("Intermediate total: %f\n", total);
        }
    }
}

/* Function 4: SIMD loop with conditionals (may generate _CONDTEMP_) */
void simd_conditional_test(double *arr, int n) {
    #pragma omp simd reduction(+:arr[:n]) linear(i:1) \
                simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            arr[i] *= 2.0;
        } else if (i % 3 == 1) {
            arr[i] /= 2.0;
        } else {
            arr[i] += 1.0;
        }
    }
    
    /* Nested SIMD with scan - may generate _SCANTEMP_ */
    double scan_var = 0.0;
    #pragma omp simd reduction(inscan,+:scan_var)
    for (int i = 0; i < n; i++) {
        arr[i] += scan_var;
        #pragma omp scan exclusive(scan_var)
        scan_var += arr[i];
    }
}

/* Function 5: Target regions with complex mappings */
void target_complex_test(struct DataBlock *block1, struct DataBlock *block2) {
    #pragma omp target map(tofrom: block1->values[0:N]) \
                       map(alloc: block2->values[0:N]) \
                       map(to: block1->indices[0:N]) \
                       depend(inout: block1) \
                       nowait
    {
        #pragma omp teams distribute parallel for simd \
                    reduction(+:block1->result)
        for (int i = 0; i < N; i++) {
            block1->values[i] = block1->indices[i] * 2.0;
            block1->result += block1->values[i];
        }
    }
    
    #pragma omp taskwait
    
    /* Use enter with to clause specifically */
    #pragma omp target enter data map(to: block2->values) \
                                  map(alloc: block2->indices)
    
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < N; i++) {
        block2->values[i] = block1->values[i] * 0.5;
    }
    
    #pragma omp target exit data map(from: block2->values) \
                                 map(release: block2->indices)
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    
    struct DataBlock block1, block2;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i * 1.0;
        array3[i] = i * 2.0;
    }
    
    for (int i = 0; i < N * M; i++) {
        array2[i] = i * 0.01;
    }
    
    printf("Starting OpenMP tests...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    scan_test(array1, N);
    
    enter_data_test(&block1);
    
    complex_reduction_test(array1, array2, N, M);
    
    simd_conditional_test(array3, N);
    
    target_complex_test(&block1, &block2);
    
    /* Final reduction across all results */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                schedule(dynamic, 16) \
                proc_bind(spread)
    for (int i = 0; i < N; i++) {
        final_result += array1[i] + array3[i];
        if (i < N/2) {
            final_result += block1.values[i];
        }
    }
    
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
