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
    double partial_sum = 0.0;
    double prefix_sum[N] = {0};
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    /* Trigger OMP_CLAUSE_INCLUSIVE and OMP_CLAUSE_EXCLUSIVE */
    double scan_temp = 0.0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:scan_temp)
        for (int i = 0; i < n; i++) {
            scan_temp += arr[i];
            #pragma omp scan inclusive(scan_temp)
            prefix_sum[i] = scan_temp;
        }
        
        /* Exclusive scan in separate loop */
        double excl_scan = 0.0;
        #pragma omp for reduction(+:excl_scan)
        for (int i = 0; i < n; i++) {
            #pragma omp scan exclusive(excl_scan)
            prefix_sum[i] = excl_scan;
            excl_scan += arr[i];
        }
    }
    
    printf("Scan test completed. Final sum: %f\n", partial_sum);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block[:1])
    #pragma omp enter data map(to: block->values[:N])
    
    /* Use the data */
    #pragma omp target teams distribute parallel for map(tofrom: block->values[:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0;
    }
    
    #pragma omp exit data map(from: block->values[:N])
    #pragma omp exit data map(from: block[:1])
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* Complex loop with multiple clauses - may generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:total) lastprivate(last_val) \
            linear(i:1) schedule(dynamic, 4) if(rows > 100)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            total += matrix[i * cols + j];
            last_val = i * cols + j;
            
            /* Nested OpenMP region inside loop - increases complexity */
            if (j % 10 == 0) {
                #pragma omp atomic
                total += 0.5;
            }
        }
    }
    
    /* SIMD loop with conditionals - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:total) simdlen(8) \
            linear(i:1) if(rows > 50)
    for (int i = 0; i < rows; i++) {
        double row_sum = 0.0;
        for (int j = 0; j < cols; j++) {
            double val = matrix[i * cols + j];
            /* Conditional inside SIMD - may trigger _CONDTEMP_ generation */
            row_sum += (val > 0.5) ? val : 0.0;
        }
        total += row_sum;
    }
    
    printf("Internal temporaries test: total = %f, last = %d\n", total, last_val);
}

/* Function 4: Target regions with complex data environment */
void test_target_complex(struct DataBlock *blocks, int num_blocks) {
    /* Complex mapping with structure array */
    #pragma omp target enter data map(to: blocks[:num_blocks])
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: blocks[:num_blocks]) \
            reduction(+:blocks[0].result) \
            if(num_blocks > 1)
    for (int b = 0; b < num_blocks; b++) {
        double block_sum = 0.0;
        for (int i = 0; i < N; i++) {
            block_sum += blocks[b].values[i] * blocks[b].indices[i];
        }
        blocks[b].result = block_sum;
        
        /* Scan operation inside target region */
        double scan_val = 0.0;
        #pragma omp scan inclusive(scan_val)
        scan_val += block_sum;
    }
    
    #pragma omp target exit data map(from: blocks[:num_blocks])
}

/* Function 5: Mixed directives to increase coverage */
void test_mixed_directives() {
    int data[M][M];
    double accum = 0.0;
    
    /* Parallel region with if clause */
    #pragma omp parallel if(M > 50) default(none) shared(data, accum)
    {
        /* Workshare with reduction */
        #pragma omp for reduction(+:accum) nowait
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = i * j;
                accum += data[i][j];
            }
        }
        
        /* Barrier then single */
        #pragma omp barrier
        
        #pragma omp single
        {
            printf("Mixed directives: accum = %f\n", accum);
        }
        
        /* Task with dependencies */
        #pragma omp task depend(inout: accum) if(omp_get_thread_num() == 0)
        {
            accum *= 2.0;
        }
        
        #pragma omp taskwait
    }
}

int main() {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Initialize test data */
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(2 * sizeof(struct DataBlock));
    
    if (!array || !matrix || !blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
        blocks[0].values[i] = (double)i / N;
        blocks[0].indices[i] = i;
        blocks[1].values[i] = (double)(N - i) / N;
        blocks[1].indices[i] = N - i;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (double)(i % 100) / 100.0;
    }
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array, N);
    test_enter_data(&blocks[0]);
    test_internal_temporaries(matrix, N, M);
    test_target_complex(blocks, 2);
    test_mixed_directives();
    
    /* Final reduction and output */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
            schedule(static) if(N > 500)
    for (int i = 0; i < N; i++) {
        final_result += array[i] + blocks[0].values[i];
    }
    
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(blocks);
    
    return 0;
}
