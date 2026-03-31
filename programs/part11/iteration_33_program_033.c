/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define DIM 32

/* Case 0: gang redundant */
void test_gang_redundant(float *data) {
    int i;
    #pragma acc parallel copy(data[0:SIZE])
    {
        #pragma acc loop gang(1)
        for (i = 0; i < SIZE; i++) {
            data[i] = i * 1.0f;
        }
    }
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *data) {
    int i;
    float sum = 0.0f;
    
    #pragma acc parallel copy(data[0:SIZE]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            data[i] = data[i] * 2.0f;
            sum += data[i];
        }
    }
    
    if (argc > 1) {  /* Runtime condition to ensure compilation */
        printf("Gang partitioned sum: %f\n", sum);
    }
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float *data) {
    int i, j;
    float local_sum;
    
    #pragma acc parallel copy(data[0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < DIM; i++) {
            local_sum = 0.0f;
            #pragma acc loop worker
            for (j = 0; j < DIM; j++) {
                int idx = i * DIM + j;
                data[idx] = data[idx] + 1.0f;
                local_sum += data[idx];
            }
        }
    }
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data2d[DIM][DIM]) {
    int i, j;
    
    #pragma acc parallel copy(data2d[0:DIM][0:DIM])
    {
        #pragma acc loop gang worker
        for (i = 1; i < DIM-1; i++) {
            for (j = 1; j < DIM-1; j++) {
                /* Simple stencil computation */
                data2d[i][j] = (data2d[i-1][j] + data2d[i][j-1]) * 0.5f;
            }
        }
    }
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *data) {
    int i;
    
    #pragma acc parallel copy(data[0:SIZE])
    {
        #pragma acc loop vector
        for (i = 0; i < SIZE; i++) {
            data[i] = data[i] * data[i];  /* Element-wise squaring */
        }
    }
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float data2d[DIM][DIM]) {
    int i, j;
    
    #pragma acc parallel copy(data2d[0:DIM][0:DIM])
    {
        #pragma acc loop gang vector
        for (i = 0; i < DIM; i++) {
            for (j = 0; j < DIM; j++) {
                data2d[i][j] = i * j * 0.01f;
            }
        }
    }
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float *data) {
    int i, j;
    
    #pragma acc parallel copy(data[0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < DIM; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < DIM; j++) {
                int idx = i * DIM + j;
                data[idx] = sinf(data[idx]) + cosf(data[idx]);
            }
        }
    }
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float data3d[DIM][DIM][DIM]) {
    int i, j, k;
    
    #pragma acc parallel copy(data3d[0:DIM][0:DIM][0:DIM])
    {
        #pragma acc loop gang
        for (i = 1; i < DIM-1; i++) {
            #pragma acc loop worker
            for (j = 1; j < DIM-1; j++) {
                #pragma acc loop vector
                for (k = 1; k < DIM-1; k++) {
                    /* 3D stencil computation */
                    data3d[i][j][k] = (data3d[i-1][j][k] + 
                                      data3d[i][j-1][k] + 
                                      data3d[i][j][k-1]) / 3.0f;
                }
            }
        }
    }
}

/* Mixed partitioning with data clauses */
void test_mixed_partitioning(float *a, float *b, float *c) {
    int i;
    
    #pragma acc data copyin(a[0:SIZE], b[0:SIZE]) copyout(c[0:SIZE])
    {
        #pragma acc parallel present(a, b, c)
        {
            #pragma acc loop gang worker
            for (i = 0; i < SIZE; i++) {
                c[i] = a[i] + b[i];
            }
            
            /* Additional vector partitioned section */
            #pragma acc loop vector
            for (i = 0; i < SIZE; i++) {
                c[i] = c[i] * 0.5f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    float data1[SIZE];
    float data2d[DIM][DIM];
    float data3d[DIM][DIM][DIM];
    float a[SIZE], b[SIZE], c[SIZE];
    int i, j, k;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        data1[i] = (float)i;
        a[i] = i * 0.5f;
        b[i] = i * 1.5f;
    }
    
    for (i = 0; i < DIM; i++) {
        for (j = 0; j < DIM; j++) {
            data2d[i][j] = (float)(i + j);
            for (k = 0; k < DIM; k++) {
                data3d[i][j][k] = (float)(i + j + k);
            }
        }
    }
    
    /* Use argc to conditionally execute different tests
       This ensures all OpenACC constructs are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(data1);
            break;
        case 1:
            test_gang_partitioned(data1);
            break;
        case 2:
            test_worker_partitioned(data1);
            break;
        case 3:
            test_gang_worker_partitioned(data2d);
            break;
        case 4:
            test_vector_partitioned(data1);
            break;
        case 5:
            test_gang_vector_partitioned(data2d);
            break;
        case 6:
            test_worker_vector_partitioned(data1);
            break;
        case 7:
            test_fully_partitioned(data3d);
            break;
        case 8:
            test_mixed_partitioning(a, b, c);
            break;
        default:
            /* Execute all tests in sequence */
            test_gang_redundant(data1);
            test_gang_partitioned(data1);
            test_worker_partitioned(data1);
            test_gang_worker_partitioned(data2d);
            test_vector_partitioned(data1);
            test_gang_vector_partitioned(data2d);
            test_worker_vector_partitioned(data1);
            test_fully_partitioned(data3d);
            test_mixed_partitioning(a, b, c);
            break;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results check:\n");
    printf("data1[0]=%f, data1[%d]=%f\n", data1[0], SIZE-1, data1[SIZE-1]);
    printf("data2d[0][0]=%f, data2d[%d][%d]=%f\n", 
           data2d[0][0], DIM-1, DIM-1, data2d[DIM-1][DIM-1]);
    printf("data3d[0][0][0]=%f\n", data3d[0][0][0]);
    
    if (argc > 2) {
        printf("c[0]=%f, c[%d]=%f\n", c[0], SIZE-1, c[SIZE-1]);
    }
    
    return 0;
}
