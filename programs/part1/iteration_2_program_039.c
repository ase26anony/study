/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define CHUNK 128

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double sum;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double prefix_sum[N];
    
    #pragma omp parallel for reduction(+:partial_sum) schedule(static, CHUNK)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    /* Trigger OMP_CLAUSE_INCLUSIVE and OMP_CLAUSE_EXCLUSIVE */
    prefix_sum[0] = arr[0];
    #pragma omp parallel for
    for (int i = 1; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum[i] = prefix_sum[i-1] + arr[i];
        
        if (i % 2 == 0) {
            double temp;
            #pragma omp scan exclusive(temp)
            temp = prefix_sum[i] * 0.5;
        }
    }
}

/* Function 2: Uses enter data with to clause */
void test_enter_data(struct DataBlock *block) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices) \
                           map(alloc: block->sum)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[N/2:N/2]) \
                map(to: block->indices)
    for (int i = N/2; i < N; i++) {
        block->values[i] = block->indices[i] * 1.5;
    }
    
    #pragma omp exit data map(from: block->sum) \
                          map(release: block->values)
}

/* Function 3: Complex loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double last_val = 0.0;
    int last_idx = 0;
    
    /* Complex reduction with lastprivate to potentially generate _LOOPTEMP_ */
    #pragma omp parallel for reduction(+:a[0:n]) \
                             lastprivate(last_val, last_idx) \
                             linear(i:1) schedule(dynamic)
    for (int i = 0; i < n; i++) {
        a[i] = a[i] * 2.0 + i;
        last_val = a[i];
        last_idx = i;
        
        /* Nested loop to increase complexity */
        #pragma omp simd reduction(+:b[i]) aligned(a, b:32)
        for (int j = 0; j < CHUNK; j++) {
            b[i] += a[i] * j;
        }
    }
    
    /* Another complex construct with conditionals */
    #pragma omp target teams distribute parallel for simd \
                reduction(max:last_val) \
                map(tofrom: a[0:n], b[0:n]) \
                if(n > 500)
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            a[i] = b[i];
        } else {
            b[i] = a[i] * 0.75;
        }
        last_val = (a[i] > last_val) ? a[i] : last_val;
    }
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ */
void test_simd_conditionals(double *arr, int n) {
    #pragma omp simd reduction(+:arr[0:n]) \
                       simdlen(8) \
                       aligned(arr:64)
    for (int i = 0; i < n; i++) {
        /* Complex conditional that might generate _CONDTEMP_ */
        arr[i] = (i % 3 == 0) ? arr[i] * 2.0 :
                 (i % 3 == 1) ? arr[i] * 1.5 :
                                arr[i] * 0.5;
        
        /* Additional conditional computation */
        if (arr[i] > 100.0) {
            arr[i] = 100.0;
        }
    }
}

/* Function 5: Nested parallelism for complex clause generation */
void test_nested_parallelism(struct DataBlock *blocks, int num_blocks) {
    #pragma omp parallel for collapse(2) \
                private(i, j) \
                shared(blocks) \
                schedule(guided)
    for (int i = 0; i < num_blocks; i++) {
        for (int j = 0; j < N; j++) {
            blocks[i].values[j] = i * N + j;
            blocks[i].indices[j] = j;
            
            /* Internal scan that might generate _SCANTEMP_ */
            #pragma omp scan inclusive(blocks[i].values)
            if (j > 0) {
                blocks[i].values[j] += blocks[i].values[j-1];
            }
        }
    }
}

int main() {
    double *array1 = (double*)aligned_alloc(64, N * sizeof(double));
    double *array2 = (double*)aligned_alloc(64, N * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(3 * sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        array1[i] = i * 1.0;
        array2[i] = i * 0.5;
    }
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array1, N);
    
    for (int i = 0; i < 3; i++) {
        test_enter_data(&blocks[i]);
    }
    
    test_internal_temporaries(array1, array2, N);
    test_simd_conditionals(array1, N);
    test_nested_parallelism(blocks, 3);
    
    /* Final reduction and output */
    double total_sum = 0.0;
    #pragma omp parallel for reduction(+:total_sum) \
                             if(N > 100) \
                             proc_bind(close)
    for (int i = 0; i < N; i++) {
        total_sum += array1[i] + array2[i];
    }
    
    for (int i = 0; i < 3; i++) {
        total_sum += blocks[i].sum;
    }
    
    printf("Total sum: %f\n", total_sum);
    printf("Test completed successfully\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(blocks);
    
    return 0;
}
