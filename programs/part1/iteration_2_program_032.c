/* Test program to trigger uncovered pretty-printing of OpenMP clauses */
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

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
        
        #pragma omp scan exclusive(exclusive_sum)
        exclusive_sum += arr[i] * 0.5;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block[:1]) \
        map(to: block->values[:N]) \
        map(to: block->indices[:N])
    
    #pragma omp target enter data map(to: block[:1]) \
        map(to: block->values) \
        map(to: block->indices)
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 1.5;
        block->indices[i] = i;
    }
    
    #pragma omp target exit data map(from: block->values[:N]) \
        map(from: block->indices[:N])
}

/* Function 3: Complex nested loops with reduction, lastprivate, linear */
void test_complex_reduction(double *a, double *b, int n, int m) {
    double total = 0.0;
    int last_i = 0, last_j = 0;
    
    /* This complex construct may generate _LOOPTEMP_ and _REDUCTEMP_ clauses */
    #pragma omp parallel for collapse(2) reduction(+:total) \
        lastprivate(last_i, last_j) linear(j:1) private(a)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            a[i * m + j] = i * j * 0.1;
            total += a[i * m + j];
            last_i = i;
            last_j = j;
        }
    }
    
    /* Another complex reduction with if clause */
    #pragma omp parallel for reduction(+:total) if(n > 500) \
        lastprivate(last_i)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            b[i * m + j] = a[i * m + j] * 2.0;
            total += b[i * m + j];
        }
        last_i = i;
    }
}

/* Function 4: SIMD with conditionals (may generate _CONDTEMP_) */
void test_simd_with_conditionals(double *arr, int n) {
    #pragma omp simd reduction(+:arr[:n]) simdlen(8) \
        aligned(arr:64)
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            arr[i] = arr[i] * 2.0;
        } else {
            arr[i] = arr[i] / 2.0;
        }
    }
    
    /* SIMD with lastprivate and linear - may generate internal temps */
    int last_val = 0;
    #pragma omp simd linear(i:1) lastprivate(last_val)
    for (int i = 0; i < n; i++) {
        arr[i] += i;
        last_val = i;
    }
}

/* Function 5: Target region with complex mapping */
void test_target_region(struct DataBlock *block, double *output) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[:N]) \
        map(to: block->indices[:N]) \
        map(from: output[:N]) \
        reduction(+:block->result)
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * block->indices[i];
        output[i] = block->values[i];
        block->result += output[i];
    }
}

/* Function 6: Combined parallel for with scan (may generate _SCANTEMP_) */
void test_combined_scan(double *arr, int n) {
    double scan_var = 0.0;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_var)
    for (int i = 0; i < n; i++) {
        #pragma omp scan exclusive(scan_var)
        double temp = arr[i] + scan_var;
        scan_var += arr[i];
        arr[i] = temp;
    }
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    double *array3 = (double*)malloc(N * M * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    
    struct DataBlock block;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i * 0.01;
        output[i] = 0.0;
        block.values[i] = i * 0.5;
        block.indices[i] = i;
    }
    
    for (int i = 0; i < N * M; i++) {
        array2[i] = i * 0.001;
        array3[i] = 0.0;
    }
    
    block.result = 0.0;
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(&block);
    test_complex_reduction(array2, array3, N, M);
    test_simd_with_conditionals(array1, N);
    test_target_region(&block, output);
    test_combined_scan(array1, N);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + output[i];
    }
    
    printf("Final result: %f\n", final_sum);
    printf("Block result: %f\n", block.result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(output);
    
    return 0;
}
