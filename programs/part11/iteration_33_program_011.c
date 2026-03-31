/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define DIM 32

/* Case 0: gang redundant */
void test_gang_redundant(float *data) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:SIZE]) copyin(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < SIZE; i++) {
            data[i] = i * 0.5f;
        }
        
        // Gang-redundant computation
        #pragma acc atomic update
        local_sum += 1.0f;
    }
    
    printf("Gang redundant: local_sum = %f\n", local_sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(data[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    printf("Gang partitioned: sum = %f\n", sum);
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
                    data[idx] = (g + w) * 1.5f;
                }
            }
        }
    }
    
    printf("Worker partitioned: data[0] = %f, data[31] = %f\n", data[0], data[31]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data2D[DIM][DIM]) {
    #pragma acc parallel copy(data2D[0:DIM][0:DIM])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                data2D[i][j] = (i + j) * 0.25f;
            }
        }
    }
    
    printf("Gang+worker partitioned: center = %f\n", data2D[DIM/2][DIM/2]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *data) {
    #pragma acc parallel loop vector copy(data[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        data[i] = data[i] * data[i] - 2.0f * data[i] + 1.0f;
    }
    
    printf("Vector partitioned: data[0] = %f\n", data[0]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *data) {
    float max_val = -1e9f;
    
    #pragma acc parallel loop gang vector reduction(max:max_val) copy(data[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        data[i] = sinf(i * 0.01f) * 100.0f;
        if (data[i] > max_val) max_val = data[i];
    }
    
    printf("Gang+vector partitioned: max = %f\n", max_val);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data2D[DIM][DIM]) {
    #pragma acc parallel copy(data2D[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int g = 0; g < 4; g++) {
            #pragma acc loop worker vector
            for (int i = g * 8; i < (g + 1) * 8 && i < DIM; i++) {
                for (int j = 0; j < DIM; j++) {
                    data2D[i][j] = cosf(i * 0.1f) * sinf(j * 0.1f);
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: corner = %f\n", data2D[0][0]);
}

/* Case 7: fully partitioned */
void test_fully_partitioned(float data3D[8][DIM][DIM]) {
    float total = 0.0f;
    
    #pragma acc parallel copy(data3D[0:8][0:DIM][0:DIM]) reduction(+:total)
    {
        #pragma acc loop gang
        for (int g = 0; g < 8; g++) {
            #pragma acc loop worker
            for (int i = 1; i < DIM - 1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < DIM - 1; j++) {
                    // Stencil computation forcing all levels
                    data3D[g][i][j] = (data3D[g][i-1][j] + 
                                      data3D[g][i][j-1] + 
                                      data3D[g][i+1][j] + 
                                      data3D[g][i][j+1]) * 0.25f;
                    total += data3D[g][i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned: total = %f\n", total);
}

/* Helper function with conditional execution */
void conditional_partition_test(float *data, int use_acc) {
    if (use_acc) {
        #pragma acc parallel loop gang copy(data[0:SIZE/2])
        for (int i = 0; i < SIZE/2; i++) {
            data[i] = i * 3.14159f;
        }
    } else {
        for (int i = 0; i < SIZE/2; i++) {
            data[i] = i * 2.71828f;
        }
    }
}

int main(int argc, char *argv[]) {
    float data[SIZE];
    float data2D[DIM][DIM];
    float data3D[8][DIM][DIM];
    
    // Initialize arrays
    memset(data, 0, sizeof(data));
    memset(data2D, 0, sizeof(data2D));
    memset(data3D, 0, sizeof(data3D));
    
    // Initialize 3D array with pattern
    for (int g = 0; g < 8; g++) {
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                data3D[g][i][j] = g * 100.0f + i * 10.0f + j;
            }
        }
    }
    
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 8;
    }
    
    // Execute based on test case or all if test_case == 0
    switch (test_case) {
        case 0:
            test_gang_redundant(data);
            break;
        case 1:
            test_gang_partitioned(data);
            break;
        case 2:
            test_worker_partitioned(data);
            break;
        case 3:
            test_gang_worker_partitioned(data2D);
            break;
        case 4:
            test_vector_partitioned(data);
            break;
        case 5:
            test_gang_vector_partitioned(data);
            break;
        case 6:
            test_worker_vector_partitioned(data2D);
            break;
        case 7:
            test_fully_partitioned(data3D);
            break;
        default:
            // Run all tests
            test_gang_redundant(data);
            test_gang_partitioned(data);
            test_worker_partitioned(data);
            test_gang_worker_partitioned(data2D);
            test_vector_partitioned(data);
            test_gang_vector_partitioned(data);
            test_worker_vector_partitioned(data2D);
            test_fully_partitioned(data3D);
            break;
    }
    
    // Conditional execution to force compiler analysis
    conditional_partition_test(data, argc > 2);
    
    // Print results to prevent dead code elimination
    printf("Final check - data[0] = %f, data[%d] = %f\n", 
           data[0], SIZE-1, data[SIZE-1]);
    printf("2D center = %f\n", data2D[DIM/2][DIM/2]);
    printf("3D sample = %f\n", data3D[0][0][0]);
    
    return 0;
}
