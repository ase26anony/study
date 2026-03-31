/* test_openmp_coverage.c
 * Designed to trigger uncovered pretty-printing logic for OpenMP clauses
 * Compile with: gcc -O1 -fopenmp -fdump-tree-omplower -fdump-tree-gimple test_openmp_coverage.c -o test_openmp_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        exclusive_sum = prefix_sum;
        
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] += exclusive_sum;
        
        prefix_sum += arr[i];
        
        #pragma omp scan inclusive(prefix_sum)
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    #pragma omp target enter data map(to: block->values[N/2:N/2]) \
                                  map(to: block)
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + block->indices[i];
    }
    
    #pragma omp target exit data map(from: block->values)
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temps(double *a, double *b, int n) {
    double last_val = 0.0;
    double reduction_sum = 0.0;
    
    /* This complex construct should generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:reduction_sum) lastprivate(last_val) \
                         linear(i:1) collapse(2) schedule(dynamic)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            double temp = a[i] * b[j];
            reduction_sum += temp;
            if (j == M-1) last_val = temp;
        }
    }
    
    /* Nested parallelism to increase complexity */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:reduction_sum)
        for (int i = 0; i < n; i++) {
            a[i] += reduction_sum;
        }
        
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:reduction_sum) grainsize(10)
            for (int i = 0; i < n/2; i++) {
                reduction_sum += i;
            }
        }
    }
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ */
void test_simd_conditionals(double *arr, int n, double threshold) {
    int count = 0;
    
    #pragma omp simd reduction(+:count) linear(i:1) \
                     simdlen(8) if(n > 1000)
    for (int i = 0; i < n; i++) {
        double val = arr[i];
        
        /* Complex conditional that might generate _CONDTEMP_ */
        if (val > threshold) {
            arr[i] = val * 2.0;
            count++;
        } else if (val < -threshold) {
            arr[i] = val / 2.0;
            count--;
        } else {
            arr[i] = 0.0;
        }
    }
    
    /* Scan directive in SIMD context */
    double scan_temp = 0.0;
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(scan_temp)
        arr[i] += scan_temp;
        scan_temp = arr[i];
    }
}

/* Function 5: Target regions with complex data clauses */
void test_target_complex(struct DataBlock *blocks, int num_blocks) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: blocks[0:num_blocks]) \
                reduction(+:blocks[0].result) \
                if(num_blocks > 1)
    for (int i = 0; i < num_blocks; i++) {
        double block_sum = 0.0;
        for (int j = 0; j < N; j++) {
            block_sum += blocks[i].values[j] * blocks[i].indices[j];
        }
        blocks[i].result = block_sum;
        
        /* Nested loop with private variables */
        #pragma omp simd private(block_sum)
        for (int j = 0; j < N/2; j++) {
            blocks[i].values[j] = block_sum / (j + 1);
        }
    }
}

/* Main function orchestrates all tests */
int main() {
    /* Initialize data */
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    struct DataBlock *blocks = (struct DataBlock*)malloc(2 * sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
        if (i < 2) {
            for (int j = 0; j < N; j++) {
                blocks[i].values[j] = (double)(i + j) / N;
                blocks[i].indices[j] = j;
            }
            blocks[i].result = 0.0;
        }
    }
    
    /* Conditional compilation path */
    int use_parallel = N > 1000;
    
    /* Test 1: Scan clauses */
    #pragma omp parallel if(use_parallel)
    {
        #pragma omp single
        test_scan_clauses(array1, N);
    }
    
    /* Test 2: Enter data with to mapper */
    test_enter_data(&blocks[0]);
    
    /* Test 3: Internal temporary generation */
    #pragma omp parallel sections
    {
        #pragma omp section
        test_internal_temps(array1, array2, N/2);
        
        #pragma omp section
        test_simd_conditionals(array2, N, 0.5);
    }
    
    /* Test 4: Complex target regions */
    test_target_complex(blocks, 2);
    
    /* Final reduction and output */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                         schedule(guided) num_threads(4)
    for (int i = 0; i < N; i++) {
        final_result += array1[i] + array2[i];
    }
    
    for (int i = 0; i < 2; i++) {
        final_result += blocks[i].result;
    }
    
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    #pragma omp target exit data map(release: blocks[0:2])
    
    free(array1);
    free(array2);
    free(blocks);
    
    return 0;
}
