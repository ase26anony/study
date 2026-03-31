/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define DIM 32

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(int *result) {
    int local_result = 0;
    
    #pragma acc parallel copy(local_result)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < 1; i++) {
            local_result = 42;
        }
    }
    
    *result = local_result;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(float *data, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = (float)i * 1.5f;
        sum += data[i];
    }
    
    // Use sum to prevent dead code elimination
    if (sum < 0) printf("Impossible\n");
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(float *data, int n) {
    int workers_per_gang = 4;
    
    #pragma acc parallel num_gangs(2) copy(data[0:n])
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2.0f + (float)(i % 10);
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(float *data, int n) {
    #pragma acc parallel copy(data[0:n])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            float temp = data[i];
            #pragma acc loop seq
            for (int j = 0; j < 4; j++) {
                temp += 0.1f * j;
            }
            data[i] = temp;
        }
    }
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(float *data, int n) {
    #pragma acc parallel loop vector copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * data[i] - 2.0f * data[i] + 1.0f;
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(float *data, int n) {
    #pragma acc parallel loop gang vector copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = 1.0f / (1.0f + data[i]);
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(float *data, int n) {
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < n; i++) {
            data[i] = (data[i] > 0.5f) ? 1.0f : 0.0f;
        }
    }
}

/* Test 8: Fully partitioned (case 7) - complex nested computation */
void test_fully_partitioned(float matrix[DIM][DIM]) {
    float temp[DIM][DIM];
    
    #pragma acc data copy(matrix[0:DIM][0:DIM]) create(temp[0:DIM][0:DIM])
    {
        #pragma acc parallel
        {
            // Initialize temp matrix
            #pragma acc loop gang
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop worker
                for (int j = 0; j < DIM; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 4; k++) {
                        // Simulate some vector operation
                        float val = (float)(i + j + k);
                        temp[i][j] = val * 0.25f;
                    }
                }
            }
            
            // Stencil computation with dependencies
            #pragma acc loop gang
            for (int i = 1; i < DIM - 1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < DIM - 1; j++) {
                    float sum = 0.0f;
                    #pragma acc loop vector reduction(+:sum)
                    for (int di = -1; di <= 1; di++) {
                        for (int dj = -1; dj <= 1; dj++) {
                            sum += temp[i + di][j + dj];
                        }
                    }
                    matrix[i][j] = sum / 9.0f;
                }
            }
        }
    }
}

/* Test 9: Mixed partitioning with conditional execution */
void test_mixed_partitioning(float *data1, float *data2, int n, int use_gpu) {
    if (use_gpu) {
        // This region should be analyzed for partitioning
        #pragma acc parallel loop gang copy(data1[0:n])
        for (int i = 0; i < n; i++) {
            data1[i] = data1[i] * 3.14f;
        }
        
        #pragma acc parallel loop worker vector copy(data2[0:n])
        for (int i = 0; i < n; i++) {
            data2[i] = data2[i] + (float)i;
        }
    } else {
        // Host fallback path
        for (int i = 0; i < n; i++) {
            data1[i] = data1[i] * 2.0f;
            data2[i] = data2[i] - 1.0f;
        }
    }
}

int main(int argc, char *argv[]) {
    int test_to_run = 0;
    
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    // Initialize test data
    float data1[SIZE];
    float data2[SIZE];
    float matrix[DIM][DIM];
    int result = 0;
    
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (float)i / SIZE;
        data2[i] = (float)(SIZE - i) / SIZE;
    }
    
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix[i][j] = (float)(i * DIM + j);
        }
    }
    
    // Execute tests based on command line argument
    // Using switch to ensure all code paths are considered during compilation
    switch (test_to_run) {
        case 0:
            test_gang_redundant(&result);
            printf("Gang redundant test: %d\n", result);
            break;
        case 1:
            test_gang_partitioned(data1, SIZE);
            printf("Gang partitioned: data1[0]=%.2f, data1[%d]=%.2f\n", 
                   data1[0], SIZE-1, data1[SIZE-1]);
            break;
        case 2:
            test_worker_partitioned(data1, SIZE);
            printf("Worker partitioned: data1[0]=%.2f\n", data1[0]);
            break;
        case 3:
            test_gang_worker_partitioned(data1, SIZE);
            printf("Gang+worker partitioned: data1[0]=%.2f\n", data1[0]);
            break;
        case 4:
            test_vector_partitioned(data1, SIZE);
            printf("Vector partitioned: data1[0]=%.2f\n", data1[0]);
            break;
        case 5:
            test_gang_vector_partitioned(data1, SIZE);
            printf("Gang+vector partitioned: data1[0]=%.2f\n", data1[0]);
            break;
        case 6:
            test_worker_vector_partitioned(data1, SIZE);
            printf("Worker+vector partitioned: data1[0]=%.2f\n", data1[0]);
            break;
        case 7:
            test_fully_partitioned(matrix);
            printf("Fully partitioned: matrix[1][1]=%.2f, matrix[%d][%d]=%.2f\n",
                   matrix[1][1], DIM-2, DIM-2, matrix[DIM-2][DIM-2]);
            break;
        case 8:
            test_mixed_partitioning(data1, data2, SIZE, argc > 2);
            printf("Mixed partitioning: data1[0]=%.2f, data2[0]=%.2f\n",
                   data1[0], data2[0]);
            break;
        default:
            // Run all tests to ensure compilation of all OpenACC regions
            test_gang_redundant(&result);
            test_gang_partitioned(data1, SIZE);
            test_worker_partitioned(data2, SIZE);
            test_gang_worker_partitioned(data1, SIZE);
            test_vector_partitioned(data2, SIZE);
            test_gang_vector_partitioned(data1, SIZE);
            test_worker_vector_partitioned(data2, SIZE);
            test_fully_partitioned(matrix);
            test_mixed_partitioning(data1, data2, SIZE, 1);
            
            printf("All tests compiled. Sample values:\n");
            printf("  result=%d, data1[0]=%.2f, data2[0]=%.2f, matrix[1][1]=%.2f\n",
                   result, data1[0], data2[0], matrix[1][1]);
            break;
    }
    
    return 0;
}
