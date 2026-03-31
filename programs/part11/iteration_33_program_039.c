/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define DIM 32

/* Test 1: Gang redundant (case 0) */
void test_gang_redundant(float *data, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(data[0:n]) copyin(n) reduction(+:sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            data[i] = i * 1.5f;
            sum += data[i];
        }
    }
    
    printf("Gang redundant: sum = %f, data[0] = %f, data[%d] = %f\n", 
           sum, data[0], n-1, data[n-1]);
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(float *data, int n) {
    #pragma acc parallel loop gang copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2.0f + i;
    }
    
    float check = 0.0f;
    for (int i = 0; i < 10 && i < n; i++) {
        check += data[i];
    }
    printf("Gang partitioned: partial sum = %f\n", check);
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(float *data, int n) {
    int workers = 4;
    
    #pragma acc parallel copy(data[0:n]) num_workers(workers)
    {
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            float temp = 0.0f;
            // Inner computation that could be worker-partitioned
            for (int j = 0; j < 8; j++) {
                temp += (i + j) * 0.1f;
            }
            data[i] = temp;
        }
    }
    
    printf("Worker partitioned: data[100] = %f\n", data[100]);
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(float data[DIM][DIM]) {
    #pragma acc parallel copy(data[0:DIM][0:DIM])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                data[i][j] = (i * DIM + j) * 0.5f;
            }
        }
    }
    
    printf("Gang+worker partitioned: corner = %f, center = %f\n", 
           data[0][0], data[DIM/2][DIM/2]);
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(float *data, int n) {
    #pragma acc parallel loop vector copy(data[0:n])
    for (int i = 0; i < n; i++) {
        // Element-wise operation suitable for vectorization
        data[i] = data[i] * data[i] - 2.0f * data[i] + 1.0f;
    }
    
    printf("Vector partitioned: data[50] = %f\n", data[50]);
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(float *data, int n) {
    #pragma acc parallel loop gang vector copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = sinf(data[i] * 0.01f) * 100.0f;
    }
    
    printf("Gang+vector partitioned: data[200] = %f\n", data[200]);
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(float data[DIM][DIM]) {
    #pragma acc parallel copy(data[0:DIM][0:DIM])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                data[i][j] = (data[i][j] + i + j) * 0.25f;
            }
        }
    }
    
    printf("Worker+vector partitioned: diagonal avg = %f\n", 
           data[DIM/2][DIM/2]);
}

/* Test 8: Fully partitioned (case 7) - complex nested computation */
void test_fully_partitioned(float data[DIM][DIM]) {
    float temp[DIM][DIM];
    
    // Initialize temp array
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            temp[i][j] = (i + j) * 0.1f;
        }
    }
    
    #pragma acc data copy(data[0:DIM][0:DIM]) copyin(temp[0:DIM][0:DIM])
    {
        #pragma acc parallel
        {
            // Triple-nested loop with explicit partitioning
            #pragma acc loop gang
            for (int i = 1; i < DIM-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < DIM-1; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 4; k++) {
                        // Stencil-like computation with data dependencies
                        float neighbor_sum = temp[i-1][j] + temp[i][j-1] + 
                                            temp[i+1][j] + temp[i][j+1];
                        data[i][j] += neighbor_sum * 0.25f * (k + 1);
                    }
                }
            }
        }
    }
    
    printf("Fully partitioned: stencil result = %f\n", data[DIM/2][DIM/2]);
}

/* Test 9: Mixed partitioning with runtime condition */
void test_mixed_partitioning(float *data1, float data2[DIM][DIM], int n, int mode) {
    if (mode > 0) {
        // Force compiler to analyze both paths
        #pragma acc parallel loop gang copy(data1[0:n])
        for (int i = 0; i < n; i++) {
            data1[i] = i * mode * 0.5f;
        }
    } else {
        #pragma acc parallel loop worker copy(data1[0:n])
        for (int i = 0; i < n; i++) {
            data1[i] = i * 0.3f;
        }
    }
    
    // Always execute this part
    #pragma acc parallel loop gang worker copy(data2[0:DIM][0:DIM])
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            data2[i][j] = data1[i % n] + j;
        }
    }
    
    printf("Mixed partitioning: data1[10] = %f, data2[5][5] = %f\n", 
           data1[10], data2[5][5]);
}

int main(int argc, char *argv[]) {
    // Allocate and initialize test data
    float *data1d = (float*)malloc(SIZE * sizeof(float));
    float data2d[DIM][DIM];
    
    if (!data1d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        data1d[i] = i * 0.1f;
    }
    
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            data2d[i][j] = (i * DIM + j) * 0.05f;
        }
    }
    
    // Use command-line argument to control which tests run
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]);
    }
    
    // Execute tests based on mode
    switch (test_mode) {
        case 0:
            // Run all tests
            test_gang_redundant(data1d, SIZE);
            test_gang_partitioned(data1d, SIZE);
            test_worker_partitioned(data1d, SIZE);
            test_gang_worker_partitioned(data2d);
            test_vector_partitioned(data1d, SIZE);
            test_gang_vector_partitioned(data1d, SIZE);
            test_worker_vector_partitioned(data2d);
            test_fully_partitioned(data2d);
            test_mixed_partitioning(data1d, data2d, SIZE, 2);
            break;
            
        case 1:
            test_gang_redundant(data1d, SIZE);
            break;
            
        case 2:
            test_gang_partitioned(data1d, SIZE);
            break;
            
        case 3:
            test_worker_partitioned(data1d, SIZE);
            break;
            
        case 4:
            test_gang_worker_partitioned(data2d);
            break;
            
        case 5:
            test_vector_partitioned(data1d, SIZE);
            break;
            
        case 6:
            test_gang_vector_partitioned(data1d, SIZE);
            break;
            
        case 7:
            test_worker_vector_partitioned(data2d);
            break;
            
        case 8:
            test_fully_partitioned(data2d);
            break;
            
        case 9:
            test_mixed_partitioning(data1d, data2d, SIZE, argc);
            break;
            
        default:
            // Run a subset with conditional execution
            if (test_mode % 2 == 0) {
                test_gang_redundant(data1d, SIZE);
            }
            if (test_mode % 3 == 0) {
                test_worker_partitioned(data1d, SIZE);
            }
            if (test_mode % 5 == 0) {
                test_fully_partitioned(data2d);
            }
            break;
    }
    
    // Print final values to prevent dead code elimination
    printf("Final check - data1d[0] = %f, data1d[%d] = %f\n", 
           data1d[0], SIZE-1, data1d[SIZE-1]);
    printf("Final check - data2d[0][0] = %f, data2d[%d][%d] = %f\n", 
           data2d[0][0], DIM-1, DIM-1, data2d[DIM-1][DIM-1]);
    
    free(data1d);
    return 0;
}
