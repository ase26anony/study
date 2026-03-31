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
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    double exclusive_prefix = 0.0;
    
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
            arr[i] = prefix_sum;
        }
        
        #pragma omp for reduction(+:exclusive_prefix)
        for (int i = 0; i < n; i++) {
            #pragma omp scan exclusive(exclusive_prefix)
            double temp = exclusive_prefix;
            exclusive_prefix += arr[i];
            arr[i] = temp;
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block[:1])
    
    #pragma omp target enter data map(to: block->values[:N]) \
        map(to: block->indices[:N])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 2.0;
        block->indices[i] = i;
    }
    
    #pragma omp target exit data map(from: block->values[:N])
}

/* Function 3: Complex loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double sum = 0.0;
    double last_val = 0.0;
    
    /* This complex construct should generate _LOOPTEMP_ and _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:sum) lastprivate(last_val) \
        linear(i:1) collapse(2) if(n > 500)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            a[i * M + j] = (i + j) * 0.5;
            sum += a[i * M + j];
            if (i == n - 1 && j == M - 1) {
                last_val = a[i * M + j];
            }
        }
    }
    
    /* Nested reduction with private variables */
    #pragma omp parallel reduction(+:sum)
    {
        double local_sum = 0.0;
        #pragma omp for nowait
        for (int i = 0; i < n * M; i++) {
            b[i] = a[i] * 2.0;
            local_sum += b[i];
        }
        sum += local_sum;
    }
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ */
void test_simd_conditionals(double *arr, int n) {
    #pragma omp simd reduction(+:arr[:n]) linear(i:1)
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            arr[i] = arr[i] * 3.0 + 1.0;
        } else {
            arr[i] = arr[i] * 2.0 - 1.0;
        }
    }
    
    /* Scan directive within SIMD region */
    double scan_var = 0.0;
    #pragma omp simd reduction(inscan,+:scan_var)
    for (int i = 0; i < n; i++) {
        arr[i] += scan_var;
        #pragma omp scan exclusive(scan_var)
        scan_var += i;
    }
}

/* Function 5: Target region with complex data clauses */
void test_target_complex(struct DataBlock *block) {
    double local_buf[M];
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[0:N/2]) \
        map(alloc: local_buf[:M]) \
        reduction(+:block->result)
    for (int i = 0; i < N/2; i++) {
        block->values[i] = block->values[i] * 2.0;
        block->result += block->values[i];
        if (i < M) {
            local_buf[i] = block->values[i];
        }
    }
    
    /* Use scan in target region */
    double prefix = 0.0;
    #pragma omp target teams distribute parallel for \
        reduction(inscan,+:prefix)
    for (int i = 0; i < N/2; i++) {
        block->values[i+N/2] = prefix;
        #pragma omp scan inclusive(prefix)
        prefix += block->values[i];
    }
}

int main() {
    double *array1 = (double*)malloc(N * M * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !block) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N * M; i++) {
        array1[i] = 0.0;
        array2[i] = 0.0;
    }
    block->result = 0.0;
    
    printf("Starting OpenMP tests...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array1, N * M);
    
    test_enter_data(block);
    
    test_internal_temporaries(array1, array2, N);
    
    test_simd_conditionals(array1, N * M);
    
    test_target_complex(block);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N * M; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Block result: %f\n", block->result);
    
    /* Cleanup */
    #pragma omp target exit data map(release: block->indices[:N])
    #pragma omp exit data map(release: block[:1])
    
    free(array1);
    free(array2);
    free(block);
    
    return 0;
}
