/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define CHUNK_SIZE 128

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
    
    #pragma omp parallel for reduction(+:prefix_sum) schedule(static, CHUNK_SIZE)
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i / n;
    }
    
    /* Test scan inclusive clause */
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += arr[i];
        arr[i] = prefix_sum;
    }
    
    /* Test scan exclusive clause */
    #pragma omp parallel for reduction(+:exclusive_sum)
    for (int i = 0; i < n; i++) {
        double temp = exclusive_sum;
        #pragma omp scan exclusive(exclusive_sum)
        exclusive_sum += arr[i];
        arr[i] = temp;
    }
}

/* Function 2: Test enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Create enter data with to clause */
    #pragma omp enter data map(to: block[:1])
    
    #pragma omp target enter data map(to: block->values[:N], block->indices[:N])
    
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0;
        block->indices[i] = i;
    }
    
    #pragma omp target exit data map(from: block->values[:N])
    #pragma omp exit data map(release: block[:1])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, double *c, int n) {
    double reduction_var = 0.0;
    int lastprivate_var = 0;
    
    /* Complex loop with multiple clauses to generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:reduction_var) lastprivate(lastprivate_var) \
                linear(i:1) schedule(dynamic, 16) collapse(2) if(n > 500)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            a[idx] = b[idx] + c[idx];
            reduction_var += a[idx];
            lastprivate_var = idx;
        }
    }
    
    /* SIMD loop with conditionals to potentially generate _CONDTEMP_ */
    #pragma omp simd reduction(+:reduction_var) linear(i:1) \
                simdlen(8) safelen(16)
    for (int i = 0; i < n * n; i++) {
        if (a[i] > 0.5) {
            a[i] = a[i] * 0.5;
        } else {
            a[i] = a[i] * 2.0;
        }
        reduction_var += a[i];
    }
    
    /* Nested parallel regions with task dependency */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: a[0:n*n/2])
            for (int i = 0; i < n * n / 2; i++) {
                a[i] = a[i] + 1.0;
            }
            
            #pragma omp task depend(in: a[0:n*n/2]) depend(out: a[n*n/2:n*n])
            for (int i = n * n / 2; i < n * n; i++) {
                a[i] = a[i] - 1.0;
            }
        }
    }
}

/* Function 4: Test scan clauses with reduction for _SCANTEMP_ */
void test_scan_with_reduction(double *arr, int n) {
    double scan_var = 0.0;
    
    #pragma omp parallel for reduction(inscan, +:scan_var)
    for (int i = 0; i < n; i++) {
        double temp = arr[i];
        #pragma omp scan exclusive(scan_var)
        arr[i] = scan_var;
        scan_var += temp;
    }
    
    /* Another scan with inclusive */
    scan_var = 0.0;
    #pragma omp parallel for reduction(inscan, +:scan_var)
    for (int i = 0; i < n; i++) {
        scan_var += arr[i];
        #pragma omp scan inclusive(scan_var)
        arr[i] = scan_var;
    }
}

/* Function 5: Complex target region with multiple clauses */
void test_target_region(struct DataBlock *block, double *output) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: block->values[:N]) \
                map(to: block->indices[:N]) \
                reduction(+:block->result) \
                num_teams(4) thread_limit(64) \
                if(N > 100)
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] + block->indices[i];
        block->result += block->values[i];
    }
    
    #pragma omp target update from(block->result)
    
    *output = block->result;
}

int main() {
    double *array1 = (double*)malloc(N * N * sizeof(double));
    double *array2 = (double*)malloc(N * N * sizeof(double));
    double *array3 = (double*)malloc(N * N * sizeof(double));
    struct DataBlock *data_block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    /* Initialize arrays */
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N * N; i++) {
        array1[i] = (double)rand() / RAND_MAX;
        array2[i] = (double)rand() / RAND_MAX;
        array3[i] = (double)rand() / RAND_MAX;
        if (i < N) {
            data_block->values[i] = (double)i;
            data_block->indices[i] = i;
        }
    }
    data_block->result = 0.0;
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(data_block->values, N);
    
    test_enter_data(data_block);
    
    test_internal_temporaries(array1, array2, array3, N);
    
    test_scan_with_reduction(data_block->values, N);
    
    double final_result = 0.0;
    test_target_region(data_block, &final_result);
    
    /* Final reduction and output */
    double total_sum = 0.0;
    #pragma omp parallel for reduction(+:total_sum) schedule(guided)
    for (int i = 0; i < N; i++) {
        total_sum += data_block->values[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Total sum: %f\n", total_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(data_block);
    
    return 0;
}
