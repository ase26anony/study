/* test_oacc_partition.c - OpenACC Partitioning Test for Coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(int use_acc) {
    float scalar = 3.14f;
    float arr[N];
    
    for (int i = 0; i < N; i++) arr[i] = (float)i;
    
    if (use_acc) {
        #pragma acc parallel copy(arr[0:N]) copy(scalar)
        {
            scalar = 42.0f;  /* gang redundant operation */
            #pragma acc loop gang(1)
            for (int i = 0; i < N; i++) {
                arr[i] = arr[i] + scalar;
            }
        }
    } else {
        scalar = 42.0f;
        for (int i = 0; i < N; i++) arr[i] = arr[i] + scalar;
    }
    
    printf("Gang redundant: arr[0]=%.2f, arr[%d]=%.2f\n", arr[0], N-1, arr[N-1]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    for (int i = 0; i < N; i++) data[i] = (float)(i % 10);
    
    if (use_acc) {
        #pragma acc parallel loop gang reduction(+:sum) copy(data[0:N])
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2.0f;
            sum += data[i];
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2.0f;
            sum += data[i];
        }
    }
    
    printf("Gang partitioned: sum=%.2f, data[100]=%.2f\n", sum, data[100]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int use_acc) {
    float matrix[M][M];
    float temp[M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i * M + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:M][0:M]) create(temp[0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    temp[j] = matrix[i][j] * 0.5f;
                }
                
                #pragma acc loop worker
                for (int j = 0; j < M; j++) {
                    matrix[i][j] = temp[j] + (float)i;
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                temp[j] = matrix[i][j] * 0.5f;
            }
            for (int j = 0; j < M; j++) {
                matrix[i][j] = temp[j] + (float)i;
            }
        }
    }
    
    printf("Worker partitioned: matrix[0][0]=%.2f, matrix[%d][%d]=%.2f\n", 
           matrix[0][0], M-1, M-1, matrix[M-1][M-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int use_acc) {
    float data[M][M];
    float partial_sums[M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M]) copyout(partial_sums[0:M])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < M; i++) {
                float row_sum = 0.0f;
                #pragma acc loop vector reduction(+:row_sum)
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] + 1.0f;
                    row_sum += data[i][j];
                }
                partial_sums[i] = row_sum;
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            float row_sum = 0.0f;
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] + 1.0f;
                row_sum += data[i][j];
            }
            partial_sums[i] = row_sum;
        }
    }
    
    printf("Gang+worker partitioned: partial_sums[0]=%.2f, partial_sums[%d]=%.2f\n",
           partial_sums[0], M-1, partial_sums[M-1]);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int use_acc) {
    float vec1[N], vec2[N], result[N];
    
    for (int i = 0; i < N; i++) {
        vec1[i] = (float)i;
        vec2[i] = (float)(N - i);
    }
    
    if (use_acc) {
        #pragma acc parallel loop vector copyin(vec1[0:N], vec2[0:N]) copyout(result[0:N])
        for (int i = 0; i < N; i++) {
            result[i] = vec1[i] * vec2[i] + sinf((float)i * 0.01f);
        }
    } else {
        for (int i = 0; i < N; i++) {
            result[i] = vec1[i] * vec2[i] + sinf((float)i * 0.01f);
        }
    }
    
    printf("Vector partitioned: result[0]=%.2f, result[%d]=%.2f\n", 
           result[0], N-1, result[N-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int use_acc) {
    float matrix[M][M];
    float col_sums[M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (float)(i + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(matrix[0:M][0:M]) copyout(col_sums[0:M])
        {
            #pragma acc loop gang vector
            for (int j = 0; j < M; j++) {
                float col_sum = 0.0f;
                #pragma acc loop worker reduction(+:col_sum)
                for (int i = 0; i < M; i++) {
                    matrix[i][j] = matrix[i][j] * 2.0f;
                    col_sum += matrix[i][j];
                }
                col_sums[j] = col_sum;
            }
        }
    } else {
        for (int j = 0; j < M; j++) {
            float col_sum = 0.0f;
            for (int i = 0; i < M; i++) {
                matrix[i][j] = matrix[i][j] * 2.0f;
                col_sum += matrix[i][j];
            }
            col_sums[j] = col_sum;
        }
    }
    
    printf("Gang+vector partitioned: col_sums[0]=%.2f, col_sums[%d]=%.2f\n",
           col_sums[0], M-1, col_sums[M-1]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int use_acc) {
    float data[M][M];
    float transformed[M][M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copyin(data[0:M][0:M]) copyout(transformed[0:M][0:M])
        {
            #pragma acc loop gang
            for (int block = 0; block < 4; block++) {
                int start = block * (M/4);
                int end = start + (M/4);
                
                #pragma acc loop worker vector
                for (int i = start; i < end; i++) {
                    for (int j = 0; j < M; j++) {
                        transformed[i][j] = data[i][j] * data[i][j] 
                                          - data[i][j] * 0.5f;
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                transformed[i][j] = data[i][j] * data[i][j] 
                                  - data[i][j] * 0.5f;
            }
        }
    }
    
    printf("Worker+vector partitioned: transformed[0][0]=%.2f, transformed[%d][%d]=%.2f\n",
           transformed[0][0], M-1, M-1, transformed[M-1][M-1]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int use_acc) {
    float volume[M][M][P];
    float smoothed[M][M][P];
    
    /* Initialize 3D volume */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                volume[i][j][k] = (float)(i + j * 2 + k * 3);
            }
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copyin(volume[0:M][0:M][0:P]) copyout(smoothed[0:M][0:M][0:P])
        {
            #pragma acc loop gang
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < M-1; j++) {
                    #pragma acc loop vector
                    for (int k = 1; k < P-1; k++) {
                        /* 3D stencil computation */
                        smoothed[i][j][k] = 
                            (volume[i-1][j][k] + volume[i+1][j][k] +
                             volume[i][j-1][k] + volume[i][j+1][k] +
                             volume[i][j][k-1] + volume[i][j][k+1]) / 6.0f;
                    }
                }
            }
        }
    } else {
        for (int i = 1; i < M-1; i++) {
            for (int j = 1; j < M-1; j++) {
                for (int k = 1; k < P-1; k++) {
                    smoothed[i][j][k] = 
                        (volume[i-1][j][k] + volume[i+1][j][k] +
                         volume[i][j-1][k] + volume[i][j+1][k] +
                         volume[i][j][k-1] + volume[i][j][k+1]) / 6.0f;
                }
            }
        }
    }
    
    printf("Fully partitioned: smoothed[1][1][1]=%.2f, smoothed[%d][%d][%d]=%.2f\n",
           smoothed[1][1][1], M-2, M-2, P-2, smoothed[M-2][M-2][P-2]);
}

/* Additional test using kernels directive */
void test_kernels_partitioning(int use_acc) {
    float a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = (float)i;
        b[i] = (float)(N - i);
    }
    
    if (use_acc) {
        #pragma acc kernels copyin(a[0:N], b[0:N]) copyout(c[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i += 64) {
                #pragma acc loop worker vector
                for (int j = i; j < i + 64 && j < N; j++) {
                    c[j] = a[j] + b[j] * 0.5f;
                }
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i] * 0.5f;
        }
    }
    
    printf("Kernels partitioning: c[0]=%.2f, c[%d]=%.2f\n", c[0], N-1, c[N-1]);
}

int main(int argc, char *argv[]) {
    int use_acc = 1;
    
    /* Use command-line argument to control execution */
    if (argc > 1 && strcmp(argv[1], "noacc") == 0) {
        use_acc = 0;
    }
    
    printf("Testing OpenACC partitioning with use_acc=%d\n\n", use_acc);
    
    /* Execute all test functions to trigger different partitioning cases */
    test_gang_redundant(use_acc);
    test_gang_partitioned(use_acc);
    test_worker_partitioned(use_acc);
    test_gang_worker_partitioned(use_acc);
    test_vector_partitioned(use_acc);
    test_gang_vector_partitioned(use_acc);
    test_worker_vector_partitioned(use_acc);
    test_fully_partitioned(use_acc);
    test_kernels_partitioning(use_acc);
    
    printf("\nAll partitioning tests completed.\n");
    return 0;
}
