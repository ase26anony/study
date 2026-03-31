/* test_oacc_partition.c - Exercise OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(int argc) {
    float scalar = 0.0f;
    float arr[N];
    
    /* Initialize array */
    for (int i = 0; i < N; i++) arr[i] = i * 1.0f;
    
    /* Use argc to ensure both paths are considered */
    if (argc > 1) {
        /* Gang redundant - no associated loop or gang(1) */
        #pragma acc parallel copy(scalar, arr[0:N])
        {
            scalar = 3.14159f;
            /* Simple assignment without loop partitioning */
            #pragma acc loop gang(1)
            for (int i = 0; i < 1; i++) {
                arr[0] = scalar;
            }
        }
    } else {
        /* Host fallback path */
        scalar = 3.14159f;
        arr[0] = scalar;
    }
    
    printf("Gang redundant: scalar = %f, arr[0] = %f\n", scalar, arr[0]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float* data, int n) {
    float sum = 0.0f;
    
    #pragma acc data copy(data[0:n]) copy(sum)
    {
        /* Explicit gang partitioning on outer loop */
        #pragma acc parallel loop gang reduction(+:sum)
        for (int i = 0; i < n; i++) {
            data[i] = data[i] * 2.0f;
            sum += data[i];
        }
        
        /* Nested gang partitioning with private variable */
        #pragma acc parallel loop gang private(sum)
        for (int i = 0; i < n/2; i++) {
            float local_sum = 0.0f;
            #pragma acc loop worker
            for (int j = 0; j < 2; j++) {
                local_sum += data[i*2 + j];
            }
            data[i] = local_sum;
        }
    }
    
    printf("Gang partitioned: sum = %f, data[%d] = %f\n", sum, n-1, data[n-1]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float* data, int n) {
    /* Worker partitioning on inner loop */
    #pragma acc data copy(data[0:n])
    {
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < n/M; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    int idx = i * M + j;
                    data[idx] = data[idx] + (i + j) * 0.5f;
                }
            }
        }
    }
    
    printf("Worker partitioned: data[%d] = %f\n", M-1, data[M-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float data[M][M]) {
    float total = 0.0f;
    
    #pragma acc data copy(data[0:M][0:M]) copy(total)
    {
        /* Combined gang and worker partitioning */
        #pragma acc parallel loop gang worker reduction(+:total)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = (data[i][j] + i - j) * 1.5f;
                total += data[i][j];
            }
        }
        
        /* Alternative: explicit separate clauses */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] / (total + 1.0f);
                }
            }
        }
    }
    
    printf("Gang+worker partitioned: total = %f, data[0][0] = %f\n", 
           total, data[0][0]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float* data, int n) {
    /* Vector partitioning for element-wise operations */
    #pragma acc parallel loop vector copy(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * data[i] - 2.0f * data[i] + 1.0f;
    }
    
    /* Vector with private variables */
    #pragma acc parallel loop vector private(data[0:n])
    for (int i = 0; i < n/2; i++) {
        float temp1 = data[i*2];
        float temp2 = data[i*2 + 1];
        data[i] = temp1 * temp2;
    }
    
    printf("Vector partitioned: data[%d] = %f\n", n/2, data[n/2]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float data[M][M]) {
    #pragma acc data copy(data[0:M][0:M])
    {
        /* Combined gang and vector partitioning */
        #pragma acc parallel loop gang vector
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = (i * M + j) * 0.01f;
            }
        }
        
        /* Stencil computation with gang+vector */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < M-1; j++) {
                    data[i][j] = (data[i-1][j] + data[i][j-1] + 
                                 data[i+1][j] + data[i][j+1]) * 0.25f;
                }
            }
        }
    }
    
    printf("Gang+vector partitioned: data[%d][%d] = %f\n", 
           M/2, M/2, data[M/2][M/2]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float data[M][M]) {
    /* Worker+vector partitioning */
    #pragma acc parallel loop worker vector copy(data[0:M][0:M])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = 1.0f / (1.0f + data[i][j]);
        }
    }
    
    printf("Worker+vector partitioned: data[%d][%d] = %f\n", 
           M-1, M-1, data[M-1][M-1]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float data[M][M][P]) {
    float grand_total = 0.0f;
    
    #pragma acc data copy(data[0:M][0:M][0:P]) copy(grand_total)
    {
        /* Triple nested loop with full partitioning */
        #pragma acc parallel reduction(+:grand_total)
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    #pragma acc loop vector reduction(+:grand_total)
                    for (int k = 0; k < P; k++) {
                        /* Complex stencil-like computation */
                        float old_val = data[i][j][k];
                        float new_val = old_val;
                        
                        if (i > 0 && i < M-1 && j > 0 && j < M-1 && k > 0 && k < P-1) {
                            new_val = (data[i-1][j][k] + data[i+1][j][k] +
                                      data[i][j-1][k] + data[i][j+1][k] +
                                      data[i][j][k-1] + data[i][j][k+1]) / 6.0f;
                        }
                        
                        data[i][j][k] = new_val;
                        grand_total += new_val;
                    }
                }
            }
        }
        
        /* Another fully partitioned region with different access pattern */
        #pragma acc parallel
        {
            #pragma acc loop gang
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < M-1; j++) {
                    #pragma acc loop vector
                    for (int k = 1; k < P-1; k++) {
                        /* Cross-dimensional computation */
                        data[i][j][k] = (data[i][j][k] + 
                                        data[i-1][j][k-1] * 0.3f +
                                        data[i+1][j][k+1] * 0.3f) * 0.7f;
                    }
                }
            }
        }
    }
    
    printf("Fully partitioned: grand_total = %f, data[1][1][1] = %f\n", 
           grand_total, data[1][1][1]);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(float* data1, float data2[M][M]) {
    /* Kernels directive with automatic partitioning */
    #pragma acc kernels copy(data1[0:N], data2[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            data1[i] = i * 0.1f;
        }
        
        #pragma acc loop gang vector
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data2[i][j] = data1[i*M + j % N] * 2.0f;
            }
        }
        
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            float row_sum = 0.0f;
            #pragma acc loop vector reduction(+:row_sum)
            for (int j = 0; j < M; j++) {
                row_sum += data2[i][j];
            }
            data2[i][i] = row_sum;
        }
    }
    
    printf("Kernels partitioning: data1[%d] = %f, data2[%d][%d] = %f\n",
           N-1, data1[N-1], M/2, M/2, data2[M/2][M/2]);
}

int main(int argc, char** argv) {
    /* Allocate and initialize test data */
    float arr1[N];
    float arr2[M][M];
    float arr3[M][M][P];
    
    for (int i = 0; i < N; i++) arr1[i] = i * 0.5f;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (i + j) * 0.25f;
        }
    }
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr3[i][j][k] = (i + j + k) * 0.1f;
            }
        }
    }
    
    /* Use argc to control which tests run, ensuring all are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(argc);
            break;
        case 1:
            test_gang_partitioned(arr1, N);
            break;
        case 2:
            test_worker_partitioned(arr1, N);
            break;
        case 3:
            test_gang_worker_partitioned(arr2);
            break;
        case 4:
            test_vector_partitioned(arr1, N);
            break;
        case 5:
            test_gang_vector_partitioned(arr2);
            break;
        case 6:
            test_worker_vector_partitioned(arr2);
            break;
        case 7:
            test_fully_partitioned(arr3);
            break;
        case 8:
            test_kernels_partitioning(arr1, arr2);
            break;
        default:
            /* Run all tests sequentially */
            test_gang_redundant(argc);
            test_gang_partitioned(arr1, N);
            test_worker_partitioned(arr1, N);
            test_gang_worker_partitioned(arr2);
            test_vector_partitioned(arr1, N);
            test_gang_vector_partitioned(arr2);
            test_worker_vector_partitioned(arr2);
            test_fully_partitioned(arr3);
            test_kernels_partitioning(arr1, arr2);
            break;
    }
    
    /* Print some results to prevent dead code elimination */
    printf("Final check - arr1[0] = %f, arr2[0][0] = %f\n", 
           arr1[0], arr2[0][0]);
    
    return 0;
}
