/* test_omp_coverage.c - Program to trigger uncovered OpenMP clause pretty-printing */

#include <stdio.h>
#include <stdlib.h>

#define N 2000
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
    
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
        
        if (i > 0) {
            #pragma omp scan exclusive(exclusive_sum)
            exclusive_sum += arr[i-1];
        }
    }
    
    printf("Scan test: prefix_sum = %f, exclusive_sum = %f\n", prefix_sum, exclusive_sum);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp target enter data map(to: block->values[0:N]) \
        to(block->indices[0:N/2])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] *= 2.0;
        if (i < N/2) {
            block->indices[i] = i * 2;
        }
    }
    
    #pragma omp target exit data map(from: block->values[0:N])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double red_temp = 0.0;
    int last_val = 0;
    
    /* This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:red_temp) lastprivate(last_val) \
        linear(i:1) collapse(2) if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            red_temp += a[i] * b[j];
            last_val = i * j;
        }
    }
    
    /* Another construct with multiple clauses */
    #pragma omp parallel for simd reduction(+:red_temp) \
        linear(i:2) lastprivate(last_val) if(n > 500)
    for (int i = 0; i < n; i++) {
        red_temp += a[i];
        last_val = i;
    }
    
    printf("Internal temps test: red_temp = %f, last_val = %d\n", red_temp, last_val);
}

/* Function 4: SIMD with conditionals to generate _CONDTEMP_ */
void test_simd_conditionals(double *arr, int n) {
    double sum = 0.0;
    
    #pragma omp simd reduction(+:sum) linear(i:1)
    for (int i = 0; i < n; i++) {
        double temp = arr[i];
        if (temp > 0.5) {
            sum += temp * 2.0;
        } else {
            sum += temp;
        }
        /* Complex conditional to potentially generate _CONDTEMP_ */
        arr[i] = (i % 3 == 0) ? temp * 3.0 : 
                 (i % 2 == 0) ? temp * 2.0 : temp;
    }
    
    printf("SIMD conditionals: sum = %f\n", sum);
}

/* Function 5: Target region with complex mapping */
void test_target_complex(struct DataBlock *block) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[0:N/4]) \
        map(to: block->indices[0:N/4]) \
        reduction(+:block->result)
    for (int i = 0; i < N/4; i++) {
        block->result += block->values[i] * block->indices[i];
        block->values[i] = block->result / (i + 1);
    }
    
    printf("Target complex: result = %f\n", block->result);
}

/* Function 6: Scan directive with both inclusive and exclusive */
void test_scan_both(double *arr, int n) {
    double inc_scan = 0.0;
    double exc_scan = 0.0;
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(inc_scan)
        inc_scan += arr[i];
        
        if (i > 0) {
            #pragma omp scan exclusive(exc_scan)
            exc_scan += arr[i-1];
        }
        
        arr[i] = inc_scan + exc_scan;
    }
    
    printf("Scan both: inc_scan = %f, exc_scan = %f\n", inc_scan, exc_scan);
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock block;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
        block.values[i] = (double)i * 0.1;
        block.indices[i] = i;
    }
    block.result = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call all test functions to trigger different OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(&block);
    test_internal_temporaries(array1, array2, N);
    test_simd_conditionals(array2, N);
    test_target_complex(&block);
    test_scan_both(array1, N/2);
    
    /* Final reduction for verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        lastprivate(block) if(N > 1000)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + block.values[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
