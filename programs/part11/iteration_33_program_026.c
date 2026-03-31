/* test_oacc_partition.c - OpenACC partitioning test for GCC coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define DIM 32

/* Case 0: gang redundant */
void test_gang_redundant(float *data) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:SIZE]) copy(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < 1; i++) {
            local_sum = 42.0f;
        }
        
        // Simple gang-redundant assignment
        data[0] = local_sum;
    }
    
    printf("Gang redundant: data[0] = %f\n", data[0]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(data[0:SIZE]) reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i * 2.0f;
        sum += data[i];
    }
    
    printf("Gang partitioned: sum = %f, data[%d] = %f\n", 
           sum, SIZE-1, data[SIZE-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *data) {
    #pragma acc parallel copy(data[0:SIZE])
    {
        #pragma acc loop gang
        for (int g = 0; g < 4; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 8; w++) {
                int idx = g * 8 + w;
                if (idx < SIZE) {
                    data[idx] = (float)(g * 100 + w);
                }
            }
        }
    }
    
    printf("Worker partitioned: data[31] = %f\n", data[31]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[DIM][DIM]) {
    #pragma acc parallel copy(data[0:DIM][0:DIM])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                data[i][j] = (float)(i * DIM + j);
            }
        }
    }
    
    printf("Gang+worker partitioned: data[%d][%d] = %f\n", 
           DIM-1, DIM-1, data[DIM-1][DIM-1]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *data) {
    #pragma acc parallel loop vector copy(data[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        data[i] = data[i] * 3.14f + 2.0f;
    }
    
    printf("Vector partitioned: data[0] = %f, data[511] = %f\n", 
           data[0], data[511]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float data[DIM][DIM]) {
    float max_val = 0.0f;
    
    #pragma acc parallel loop gang vector collapse(2) \
                copy(data[0:DIM][0:DIM]) reduction(max:max_val)
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            data[i][j] = (float)(i * j) / 10.0f;
            if (data[i][j] > max_val) max_val = data[i][j];
        }
    }
    
    printf("Gang+vector partitioned: max = %f\n", max_val);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *data) {
    #pragma acc parallel copy(data[0:SIZE])
    {
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            #pragma acc loop worker vector
            for (int i = 0; i < SIZE/2; i++) {
                int idx = g * (SIZE/2) + i;
                data[idx] = (float)idx * 1.5f;
            }
        }
    }
    
    printf("Worker+vector partitioned: data[%d] = %f\n", 
           SIZE/2, data[SIZE/2]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float data[DIM][DIM]) {
    float temp[DIM][DIM];
    
    #pragma acc data copy(data[0:DIM][0:DIM]) create(temp[0:DIM][0:DIM])
    {
        // Initialize
        #pragma acc parallel loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker
            for (int j = 0; j < DIM; j++) {
                data[i][j] = (float)(i + j);
            }
        }
        
        // Stencil computation with full partitioning
        #pragma acc parallel loop gang
        for (int i = 1; i < DIM-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < DIM-1; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    // Multi-step computation to force partitioning
                    float val = data[i][j];
                    val += data[i-1][j] * 0.25f;
                    val += data[i][j-1] * 0.25f;
                    val += data[i+1][j] * 0.25f;
                    val += data[i][j+1] * 0.25f;
                    temp[i][j] = val / (1.0f + k * 0.1f);
                }
                data[i][j] = temp[i][j];
            }
        }
    }
    
    printf("Fully partitioned: data[%d][%d] = %f\n", 
           DIM/2, DIM/2, data[DIM/2][DIM/2]);
}

/* Helper with conditional execution to ensure compiler analysis */
void conditional_test(int test_id, float *arr1, float arr2[DIM][DIM]) {
    if (test_id > 0) {  // Always true at compile time for analysis
        switch (test_id % 8) {
            case 0: test_gang_redundant(arr1); break;
            case 1: test_gang_partitioned(arr1); break;
            case 2: test_worker_partitioned(arr1); break;
            case 3: test_gang_worker_partitioned(arr2); break;
            case 4: test_vector_partitioned(arr1); break;
            case 5: test_gang_vector_partitioned(arr2); break;
            case 6: test_worker_vector_partitioned(arr1); break;
            case 7: test_fully_partitioned(arr2); break;
        }
    }
}

int main(int argc, char *argv[]) {
    float data_array[SIZE];
    float matrix[DIM][DIM];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data_array[i] = (float)i;
    }
    
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix[i][j] = (float)(i * DIM + j);
        }
    }
    
    // Execute tests based on command line argument
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    // Force compiler to analyze all paths by calling all functions
    // in conditional blocks based on argc
    for (int i = 0; i < 8; i++) {
        if (argc > i) {  // Compiler can't predict argc
            conditional_test(i, data_array, matrix);
        }
    }
    
    // Also call the specific test if requested
    if (test_to_run >= 0 && test_to_run < 8) {
        conditional_test(test_to_run, data_array, matrix);
    }
    
    // Print some results to prevent dead code elimination
    printf("Final check: data_array[0] = %f, matrix[0][0] = %f\n",
           data_array[0], matrix[0][0]);
    
    return 0;
}
