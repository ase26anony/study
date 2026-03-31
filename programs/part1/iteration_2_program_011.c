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
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 1.5;
        partial_sum += arr[i];
    }
    
    /* Trigger OMP_CLAUSE_INCLUSIVE and OMP_CLAUSE_EXCLUSIVE */
    #pragma omp parallel
    {
        double local_sum = 0.0;
        #pragma omp for reduction(+:local_sum)
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
        }
        
        #pragma omp barrier
        
        /* Inclusive scan */
        #pragma omp scan inclusive(local_sum)
        {
            /* Do something with scanned value */
        }
        
        /* Exclusive scan */
        #pragma omp scan exclusive(local_sum)
        {
            /* Do something with scanned value */
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block[:1])
    
    #pragma omp target enter data map(to: block->values[:N]) \
                                  map(to: block->indices[:N])
    
    /* Nested pragma to increase complexity */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: block->result)
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + i;
        block->indices[i] = i % 100;
    }
    
    #pragma omp target exit data map(from: block->values[:N])
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double sum = 0.0;
    int last_val = 0;
    
    /* Complex loop with multiple clauses to potentially generate
       _LOOPTEMP_, _REDUCTEMP_, _CONDTEMP_ */
    #pragma omp parallel for collapse(2) reduction(+:sum) \
                lastprivate(last_val) linear(i:1) if(rows > 100)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            matrix[idx] = (i + j) * 0.5;
            sum += matrix[idx];
            
            /* Conditional inside loop for _CONDTEMP_ */
            if (matrix[idx] > 50.0) {
                #pragma omp atomic
                sum += 1.0;
            }
            
            last_val = idx;
        }
    }
    
    /* SIMD loop with conditionals */
    #pragma omp simd reduction(+:sum) linear(i:1) \
                simdlen(8) if(cols > 50)
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] += sum * 0.01;
        if (matrix[i] < 0) {
            matrix[i] = 0;
        }
    }
}

/* Function 4: Mixed constructs for _SCANTEMP_ generation */
void test_mixed_constructs(int *data, int n) {
    int scan_temp = 0;
    
    #pragma omp parallel
    {
        #pragma omp for reduction(+:scan_temp) nowait
        for (int i = 0; i < n; i++) {
            data[i] = i * 2;
            scan_temp += data[i];
        }
        
        /* Additional complexity with taskgroup */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                for (int i = 0; i < n/2; i++) {
                    data[i] += scan_temp;
                }
            }
            
            #pragma omp task
            {
                for (int i = n/2; i < n; i++) {
                    data[i] -= scan_temp;
                }
            }
        }
    }
}

/* Function 5: Target region with complex data environment */
void test_target_region(struct DataBlock *block1, struct DataBlock *block2) {
    #pragma omp target teams distribute parallel for \
                map(to: block1->values[:N], block2->values[:N]) \
                map(from: block1->result, block2->result) \
                reduction(+:block1->result, block2->result) \
                num_teams(4) thread_limit(64)
    for (int i = 0; i < N; i++) {
        block1->result += block1->values[i];
        block2->result += block2->values[i];
        
        /* Nested condition for more complexity */
        if (i % 2 == 0) {
            block1->values[i] *= 1.1;
        } else {
            block2->values[i] *= 0.9;
        }
    }
}

int main() {
    double *array = (double*)malloc(N * sizeof(double));
    double *matrix = (double*)malloc(N * M * sizeof(double));
    int *int_data = (int*)malloc(N * sizeof(int));
    
    struct DataBlock block1, block2;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        block1.values[i] = i * 1.1;
        block2.values[i] = i * 0.9;
        block1.indices[i] = i;
        block2.indices[i] = N - i - 1;
    }
    block1.result = 0.0;
    block2.result = 0.0;
    
    printf("Testing OpenMP clause pretty-printing coverage...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array, N);
    test_enter_data(&block1);
    test_internal_temporaries(matrix, N, M);
    test_mixed_constructs(int_data, N);
    test_target_region(&block1, &block2);
    
    /* Final computation with reduction */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                if(N > 500) schedule(dynamic, 16)
    for (int i = 0; i < N; i++) {
        final_sum += array[i] + block1.values[i] + block2.values[i];
    }
    
    printf("Final sum: %f\n", final_sum);
    printf("Block1 result: %f\n", block1.result);
    printf("Block2 result: %f\n", block2.result);
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(int_data);
    
    return 0;
}
