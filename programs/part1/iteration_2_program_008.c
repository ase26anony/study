/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
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
    double partial_sum = 0.0;
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
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
        exclusive_prefix = local_sum;
        
        /* Nested to increase complexity */
        #pragma omp for
        for (int i = 0; i < n/2; i++) {
            /* Trigger OMP_CLAUSE_EXCLUSIVE */
            #pragma omp scan exclusive(local_sum)
            arr[i] += local_sum;
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block->values[0:N]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + i;
        block->indices[i] = i % 100;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values)
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* Complex combination likely to generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) \
                linear(i:1) schedule(dynamic, 16)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] = (i + j) * 0.1;
            total += matrix[i * cols + j];
            if (i == rows - 1 && j == cols - 1) {
                last_val = i * cols + j;
            }
        }
    }
    
    /* Additional complexity with SIMD */
    #pragma omp parallel for simd reduction(+:total) \
                linear(k:2) simdlen(8) if(rows > 500)
    for (int k = 0; k < rows * cols; k += 2) {
        matrix[k] = matrix[k] * 1.5;
        total += matrix[k];
        /* Conditional inside SIMD may generate _CONDTEMP_ */
        if (matrix[k] > 100.0) {
            matrix[k] = 100.0;
        }
    }
}

/* Function 4: Mixed directives to trigger _SCANTEMP_ */
void test_scan_temp(double *arr, int n) {
    double scan_var = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for simd reduction(inscan,+:scan_var)
        for (int i = 0; i < n; i++) {
            /* This should generate _SCANTEMP_ during lowering */
            scan_var += arr[i];
            #pragma omp scan inclusive(scan_var)
            arr[i] = scan_var;
        }
    }
    
    /* Nested with taskloop */
    #pragma omp taskloop grainsize(64) reduction(+:scan_var) \
                if(n > 1000) final(n > 5000)
    for (int i = 0; i < n; i++) {
        scan_var += arr[i] * 0.5;
    }
}

/* Function 5: Target regions with complex data clauses */
void test_target_complex(struct DataBlock *block1, struct DataBlock *block2) {
    /* Complex mapping likely to generate various temporaries */
    #pragma omp target teams distribute parallel for \
                map(to: block1->values[0:N/2]) \
                map(tofrom: block2->values[N/2:N/2]) \
                map(alloc: block1->indices[0:N]) \
                reduction(+:block1->result) \
                depend(inout: block1) \
                if(N > 1000)
    for (int i = 0; i < N/2; i++) {
        block1->values[i] = block2->values[N/2 + i] * 3.0;
        block1->result += block1->values[i];
    }
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock block1, block2;
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        block1.values[i] = i * 0.25;
        block1.indices[i] = i;
        block2.values[i] = i * 0.33;
        block2.indices[i] = N - i;
    }
    block1.result = 0.0;
    block2.result = 0.0;
    
    /* Call all test functions to trigger different OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(&block1);
    test_internal_temporaries(matrix, N, M);
    test_scan_temp(array1, N);
    test_target_complex(&block1, &block2);
    
    /* Final reduction for verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                if(N > 500) num_threads(4)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + block1.values[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Block1 result: %f\n", block1.result);
    
    free(array1);
    free(matrix);
    
    return 0;
}
