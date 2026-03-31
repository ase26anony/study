/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>

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
        local_sum = 42.0f;  // Simple assignment in gang-redundant region
    }
    
    printf("Gang redundant: data[0]=%.2f, local_sum=%.2f\n", data[0], local_sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(data[0:SIZE]) reduction(+:sum)
    for (int i = 0; i < SIZE; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    printf("Gang partitioned: sum=%.2f, data[%d]=%.2f\n", sum, SIZE-1, data[SIZE-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float data[DIM][DIM]) {
    float row_sums[DIM] = {0};
    
    #pragma acc parallel copy(data[0:DIM][0:DIM]) create(row_sums[0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker reduction(+:row_sums[i])
            for (int j = 0; j < DIM; j++) {
                data[i][j] = (i + j) * 0.1f;
                row_sums[i] += data[i][j];
            }
        }
    }
    
    printf("Worker partitioned: row_sums[0]=%.2f, data[%d][%d]=%.2f\n", 
           row_sums[0], DIM-1, DIM-1, data[DIM-1][DIM-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[DIM][DIM]) {
    float total = 0.0f;
    
    #pragma acc parallel loop gang worker copy(data[0:DIM][0:DIM]) reduction(+:total)
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            data[i][j] = (data[i][j] + i - j) * 1.5f;
            total += data[i][j];
        }
    }
    
    printf("Gang+worker partitioned: total=%.2f\n", total);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *data) {
    #pragma acc parallel loop vector copy(data[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        // Element-wise operations suitable for vectorization
        data[i] = data[i] * data[i] - 2.0f * data[i] + 1.0f;
    }
    
    printf("Vector partitioned: data[0]=%.2f, data[%d]=%.2f\n", data[0], SIZE/2, data[SIZE/2]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *data) {
    float max_val = -1e9;
    
    #pragma acc parallel loop gang vector copy(data[0:SIZE]) reduction(max:max_val)
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i % 17) * 3.14f;
        if (data[i] > max_val) max_val = data[i];
    }
    
    printf("Gang+vector partitioned: max_val=%.2f\n", max_val);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data[DIM][DIM]) {
    #pragma acc parallel copy(data[0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < DIM; j++) {
                // Stencil-like computation with vector operations
                float left = (j > 0) ? data[i][j-1] : 0.0f;
                float up = (i > 0) ? data[i-1][j] : 0.0f;
                data[i][j] = (left + up) * 0.5f + (i * j) * 0.01f;
            }
        }
    }
    
    printf("Worker+vector partitioned: data[%d][%d]=%.2f\n", 
           DIM-1, DIM-1, data[DIM-1][DIM-1]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float data[DIM][DIM][DIM]) {
    float grand_total = 0.0f;
    
    #pragma acc parallel copy(data[0:DIM][0:DIM][0:DIM]) reduction(+:grand_total)
    {
        #pragma acc loop gang
        for (int i = 1; i < DIM-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < DIM-1; j++) {
                #pragma acc loop vector reduction(+:grand_total)
                for (int k = 1; k < DIM-1; k++) {
                    // 3D stencil computation requiring all partitioning levels
                    data[i][j][k] = (data[i-1][j][k] + data[i][j-1][k] + 
                                    data[i][j][k-1]) * 0.333f;
                    grand_total += data[i][j][k];
                }
            }
        }
    }
    
    printf("Fully partitioned: grand_total=%.2f, center=%.2f\n", 
           grand_total, data[DIM/2][DIM/2][DIM/2]);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float *data1, float data2[DIM][DIM]) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    #pragma acc kernels copy(data1[0:SIZE], data2[0:DIM][0:DIM]) \
                         copyout(sum1, sum2)
    {
        #pragma acc loop gang reduction(+:sum1)
        for (int i = 0; i < SIZE; i++) {
            data1[i] = i * 0.25f;
            sum1 += data1[i];
        }
        
        #pragma acc loop gang worker reduction(+:sum2)
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                data2[i][j] = (i - j) * 0.5f;
                sum2 += data2[i][j];
            }
        }
    }
    
    printf("Kernels partitioning: sum1=%.2f, sum2=%.2f\n", sum1, sum2);
}

int main(int argc, char *argv[]) {
    // Initialize test data
    float data1[SIZE];
    float data2[DIM][DIM];
    float data3[DIM][DIM][DIM];
    
    for (int i = 0; i < SIZE; i++) data1[i] = i * 1.0f;
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            data2[i][j] = (i + j) * 1.0f;
    for (int i = 0; i < DIM; i++)
        for (int j = 0; j < DIM; j++)
            for (int k = 0; k < DIM; k++)
                data3[i][j][k] = (i + j + k) * 0.1f;
    
    // Use argc to control which tests run, ensuring all code paths are compiled
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(data1);
            break;
        case 1:
            test_gang_partitioned(data1);
            break;
        case 2:
            test_worker_partitioned(data2);
            break;
        case 3:
            test_gang_worker_partitioned(data2);
            break;
        case 4:
            test_vector_partitioned(data1);
            break;
        case 5:
            test_gang_vector_partitioned(data1);
            break;
        case 6:
            test_worker_vector_partitioned(data2);
            break;
        case 7:
            test_fully_partitioned(data3);
            break;
        case 8:
            test_kernels_partitioning(data1, data2);
            break;
        default:
            // Run all tests in sequence when no specific test is requested
            test_gang_redundant(data1);
            test_gang_partitioned(data1);
            test_worker_partitioned(data2);
            test_gang_worker_partitioned(data2);
            test_vector_partitioned(data1);
            test_gang_vector_partitioned(data1);
            test_worker_vector_partitioned(data2);
            test_fully_partitioned(data3);
            test_kernels_partitioning(data1, data2);
    }
    
    // Print final values to prevent dead code elimination
    printf("Final check: data1[0]=%.2f, data2[0][0]=%.2f\n", data1[0], data2[0][0]);
    
    return 0;
}
