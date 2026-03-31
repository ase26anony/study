/* test_openmp_clauses.c
 * Designed to trigger uncovered pretty-printing logic in tree-pretty-print.cc
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple -o test test_openmp_clauses.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Test scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 1.5;
    }
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:prefix_sum)
        for (int i = 0; i < n; i++) {
            #pragma omp scan inclusive(prefix_sum)
            prefix_sum += arr[i];
            
            if (i > 0) {
                #pragma omp scan exclusive(exclusive_sum)
                exclusive_sum += arr[i-1];
            }
        }
    }
    
    printf("Scan test: prefix_sum = %f, exclusive_sum = %f\n", prefix_sum, exclusive_sum);
}

/* Function 2: Test enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Force generation of enter data with to clause */
    #pragma omp target enter data map(to: block[0:1]) \
        depend(inout: block) nowait
    
    /* Another enter data with explicit to mapper */
    #pragma omp target enter data map(to: block->values[0:N]) \
        map(to: block->indices[0:N])
    
    /* Complex enter data with conditional */
    if (block->result > 0) {
        #pragma omp target enter data map(to: block->result)
    }
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double sum = 0.0;
    double last_val = 0.0;
    
    /* Combined reduction, lastprivate, and linear clauses */
    #pragma omp parallel for reduction(+:sum) lastprivate(last_val) linear(i:1) \
        schedule(dynamic, 16) collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            a[i*M + j] = i * j * 0.5;
            b[i*M + j] = a[i*M + j] * 2.0;
            sum += a[i*M + j];
            last_val = b[i*M + j];
        }
    }
    
    /* Nested parallel regions with reduction */
    #pragma omp parallel reduction(+:sum)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            sum += i * 0.1;
        }
        
        #pragma omp for reduction(+:sum)
        for (int i = 0; i < n/2; i++) {
            sum += i * 0.2;
        }
    }
    
    printf("Temporaries test: sum = %f, last_val = %f\n", sum, last_val);
}

/* Function 4: SIMD with conditionals to generate _CONDTEMP_ */
void test_simd_conditionals(double *arr, int n) {
    #pragma omp simd reduction(+:arr[:n]) linear(i:1) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        /* Conditional inside SIMD loop may generate _CONDTEMP_ */
        if (i % 2 == 0) {
            arr[i] = arr[i] * 2.0;
        } else {
            arr[i] = arr[i] / 2.0;
        }
        
        /* Another conditional with complex expression */
        arr[i] = (i > n/2) ? arr[i] + 1.0 : arr[i] - 1.0;
    }
    
    /* SIMD with lastprivate */
    double last = 0.0;
    #pragma omp simd lastprivate(last)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * i;
        last = arr[i];
    }
    
    printf("SIMD test: last = %f\n", last);
}

/* Function 5: Target regions with complex mapping */
void test_target_regions(struct DataBlock *block, double *output) {
    /* Target with map clauses */
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[0:N]) \
        map(to: block->indices[0:N]) \
        map(from: output[0:N]) \
        reduction(+:block->result) \
        num_teams(4) thread_limit(64)
    for (int i = 0; i < N; i++) {
        block->values[i] = block->indices[i] * 2.5;
        output[i] = block->values[i] * 3.0;
        block->result += output[i];
    }
    
    /* Target data with enter/exit */
    #pragma omp target data map(to: block[0:1]) \
        map(alloc: output[0:N])
    {
        #pragma omp target map(present, tofrom: block->result)
        {
            block->result *= 0.5;
        }
    }
    
    printf("Target test: result = %f\n", block->result);
}

/* Function 6: Test scan directive with multiple clauses */
void test_scan_directive(double *arr, int n) {
    double sum = 0.0;
    double prod = 1.0;
    
    #pragma omp parallel
    {
        #pragma omp for reduction(inscan, +:sum)
        for (int i = 0; i < n; i++) {
            sum += arr[i];
            #pragma omp scan inclusive(sum)
            
            /* Exclusive scan on product */
            if (i > 0) {
                #pragma omp scan exclusive(prod)
                prod *= arr[i];
            }
        }
    }
    
    printf("Scan directive: sum = %f, prod = %f\n", sum, prod);
}

int main() {
    /* Allocate test data */
    double *array1 = (double*)malloc(N * M * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        block->indices[i] = i;
        block->values[i] = i * 0.1;
        output[i] = 0.0;
    }
    block->result = 0.0;
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(block);
    test_internal_temporaries(array1, array2, N);
    test_simd_conditionals(array1, N);
    test_target_regions(block, output);
    test_scan_directive(array1, N);
    
    /* Final reduction for verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        if(N>100)  /* Conditional to affect gimplification */
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + output[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(output);
    free(block);
    
    return 0;
}
