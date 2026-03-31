/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define DIM 32

/* Case 0: gang redundant */
void test_gang_redundant(float *data, int n) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:n]) copyin(n) reduction(+:local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            data[i] = i * 1.5f;
            local_sum += data[i];
        }
    }
    
    if (local_sum > 0) {
        printf("Gang redundant test completed, sum: %f\n", local_sum);
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(data[0:n]) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    if (sum > 0) {
        printf("Gang partitioned test completed, sum: %f\n", sum);
    }
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *data, int n) {
    float temp[DIM][DIM];
    
    #pragma acc parallel loop gang copy(data[0:n]) create(temp[0:DIM][0:DIM])
    for (int i = 0; i < DIM; i++) {
        #pragma acc loop worker
        for (int j = 0; j < DIM; j++) {
            temp[i][j] = (float)(i * DIM + j);
            if (i * DIM + j < n) {
                data[i * DIM + j] = temp[i][j];
            }
        }
    }
    
    printf("Worker partitioned test completed\n");
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float *data, int n) {
    float matrix[DIM][DIM];
    
    #pragma acc parallel loop gang worker copy(matrix[0:DIM][0:DIM])
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix[i][j] = (float)(i + j);
            if (i * DIM + j < n) {
                data[i * DIM + j] = matrix[i][j];
            }
        }
    }
    
    printf("Gang+worker partitioned test completed\n");
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *data, int n) {
    #pragma acc parallel loop vector copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 3.14159f;
    }
    
    printf("Vector partitioned test completed\n");
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *data, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang vector copy(data[0:n]) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        data[i] = data[i] / (i + 1.0f);
        sum += data[i];
    }
    
    if (sum > 0) {
        printf("Gang+vector partitioned test completed, sum: %f\n", sum);
    }
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *data, int n) {
    float matrix[DIM][DIM];
    
    #pragma acc parallel loop gang copy(matrix[0:DIM][0:DIM])
    for (int i = 0; i < DIM; i++) {
        #pragma acc loop worker vector
        for (int j = 0; j < DIM; j++) {
            matrix[i][j] = (float)(i * j);
            if (i * DIM + j < n) {
                data[i * DIM + j] = matrix[i][j];
            }
        }
    }
    
    printf("Worker+vector partitioned test completed\n");
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float *data, int n) {
    float matrix[DIM][DIM];
    float result[DIM][DIM];
    
    // Initialize matrix
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            matrix[i][j] = (float)(i + j);
        }
    }
    
    #pragma acc parallel copyin(matrix[0:DIM][0:DIM]) copyout(result[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker
            for (int j = 0; j < DIM; j++) {
                float temp = 0.0f;
                #pragma acc loop vector reduction(+:temp)
                for (int k = 0; k < DIM; k++) {
                    temp += matrix[i][k] * matrix[k][j];
                }
                result[i][j] = temp;
                if (i * DIM + j < n) {
                    data[i * DIM + j] = result[i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned test completed\n");
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *data, int n) {
    float local_data[SIZE];
    
    #pragma acc kernels copy(local_data[0:SIZE]) copy(data[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            local_data[i] = (float)i;
        }
        
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE/2; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < n) {
                    data[idx] = local_data[idx] * 2.0f;
                }
            }
        }
    }
    
    printf("Kernels partitioning test completed\n");
}

/* Test with conditional execution based on runtime */
void test_conditional_partitioning(float *data, int n, int condition) {
    if (condition) {
        #pragma acc parallel loop gang copy(data[0:n])
        for (int i = 0; i < n; i++) {
            data[i] = data[i] + 100.0f;
        }
    } else {
        #pragma acc parallel loop vector copy(data[0:n])
        for (int i = 0; i < n; i++) {
            data[i] = data[i] - 50.0f;
        }
    }
    
    printf("Conditional partitioning test completed\n");
}

int main(int argc, char *argv[]) {
    float data[SIZE];
    int test_case = 0;
    
    // Initialize data
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)i;
    }
    
    // Parse test case from command line if provided
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    // Execute different test functions based on input
    switch (test_case) {
        case 0:
            test_gang_redundant(data, SIZE);
            break;
        case 1:
            test_gang_partitioned(data, SIZE);
            break;
        case 2:
            test_worker_partitioned(data, SIZE);
            break;
        case 3:
            test_gang_worker_partitioned(data, SIZE);
            break;
        case 4:
            test_vector_partitioned(data, SIZE);
            break;
        case 5:
            test_gang_vector_partitioned(data, SIZE);
            break;
        case 6:
            test_worker_vector_partitioned(data, SIZE);
            break;
        case 7:
            test_fully_partitioned(data, SIZE);
            break;
        case 8:
            test_kernels_partitioning(data, SIZE);
            break;
        case 9:
            test_conditional_partitioning(data, SIZE, argc > 2);
            break;
        default:
            // Run all tests to ensure all code paths are compiled
            test_gang_redundant(data, SIZE);
            test_gang_partitioned(data, SIZE);
            test_worker_partitioned(data, SIZE);
            test_gang_worker_partitioned(data, SIZE);
            test_vector_partitioned(data, SIZE);
            test_gang_vector_partitioned(data, SIZE);
            test_worker_vector_partitioned(data, SIZE);
            test_fully_partitioned(data, SIZE);
            test_kernels_partitioning(data, SIZE);
            test_conditional_partitioning(data, SIZE, 1);
            printf("All tests executed\n");
            break;
    }
    
    // Print some results to prevent dead code elimination
    printf("First element: %f, Last element: %f\n", data[0], data[SIZE-1]);
    
    return 0;
}
