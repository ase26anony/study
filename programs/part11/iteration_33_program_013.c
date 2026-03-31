/* test_oacc_partition.c - Test OpenACC partitioning cases for coverage */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:N]) copy(sum)
        {
            #pragma acc loop gang(1)
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 2.0f;
            }
            sum = 1.0f;  /* Simple assignment in gang redundant region */
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2.0f;
        }
        sum = 1.0f;
    }
    
    printf("Gang redundant: data[0]=%.1f, data[%d]=%.1f, sum=%.1f\n", 
           data[0], N-1, data[N-1], sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:N]) copy(sum)
        {
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 3.0f;
                sum += data[i];
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 3.0f;
            sum += data[i];
        }
    }
    
    printf("Gang partitioned: sum=%.1f\n", sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int use_acc) {
    float data[M][M];
    float partial_sums[M];
    
    for (int i = 0; i < M; i++) {
        partial_sums[i] = 0.0f;
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * M + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M]) copy(partial_sums[0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker reduction(+:partial_sums[i])
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] * 1.5f;
                    partial_sums[i] += data[i][j];
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] * 1.5f;
                partial_sums[i] += data[i][j];
            }
        }
    }
    
    printf("Worker partitioned: partial_sums[0]=%.1f\n", partial_sums[0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int use_acc) {
    float data[M][M];
    float total_sum = 0.0f;
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * M + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M]) copy(total_sum)
        {
            #pragma acc loop gang worker reduction(+:total_sum)
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] * 2.0f;
                    total_sum += data[i][j];
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] * 2.0f;
                total_sum += data[i][j];
            }
        }
    }
    
    printf("Gang+worker partitioned: total_sum=%.1f\n", total_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int use_acc) {
    float data[N];
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:N])
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 4.0f + (float)(i % 8);
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 4.0f + (float)(i % 8);
        }
    }
    
    printf("Vector partitioned: data[0]=%.1f, data[%d]=%.1f\n", 
           data[0], N-1, data[N-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int use_acc) {
    float data[N];
    float partial_results[N/32];
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    for (int i = 0; i < N/32; i++) {
        partial_results[i] = 0.0f;
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:N]) copy(partial_results[0:N/32])
        {
            #pragma acc loop gang vector reduction(+:partial_results[:N/32])
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 2.5f;
                partial_results[i/32] += data[i];
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2.5f;
            partial_results[i/32] += data[i];
        }
    }
    
    printf("Gang+vector partitioned: partial_results[0]=%.1f\n", partial_results[0]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int use_acc) {
    float data[M][M];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * M + j);
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    data[i][j] = data[i][j] * 3.0f - (float)(j % 4);
                }
            }
        }
    } else {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] * 3.0f - (float)(j % 4);
            }
        }
    }
    
    printf("Worker+vector partitioned: data[0][0]=%.1f\n", data[0][0]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(int use_acc) {
    float data[M][M];
    float result[M][M];
    
    /* Initialize with simple values */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (float)(i * M + j);
            result[i][j] = 0.0f;
        }
    }
    
    if (use_acc) {
        #pragma acc parallel copy(data[0:M][0:M]) create(result[0:M][0:M])
        {
            /* Triple-nested loop with explicit partitioning */
            #pragma acc loop gang
            for (int i = 1; i < M-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < M-1; j++) {
                    #pragma acc loop vector
                    for (int k = 0; k < 4; k++) {
                        /* Stencil-like computation forcing all levels */
                        float temp = data[i-1][j] + data[i][j-1] + (float)k;
                        result[i][j] += temp * 0.5f;
                    }
                    data[i][j] = result[i][j];
                }
            }
        }
    } else {
        for (int i = 1; i < M-1; i++) {
            for (int j = 1; j < M-1; j++) {
                for (int k = 0; k < 4; k++) {
                    float temp = data[i-1][j] + data[i][j-1] + (float)k;
                    result[i][j] += temp * 0.5f;
                }
                data[i][j] = result[i][j];
            }
        }
    }
    
    printf("Fully partitioned: data[1][1]=%.1f, data[%d][%d]=%.1f\n", 
           data[1][1], M-2, M-2, data[M-2][M-2]);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        data[i] = (float)i;
    }
    
    if (use_acc) {
        #pragma acc kernels copy(data[0:N]) copy(sum)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                data[i] = data[i] * 1.1f;
            }
            
            #pragma acc loop gang reduction(+:sum)
            for (int i = 0; i < N; i++) {
                sum += data[i];
            }
        }
    } else {
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 1.1f;
        }
        for (int i = 0; i < N; i++) {
            sum += data[i];
        }
    }
    
    printf("Kernels partitioning: sum=%.1f\n", sum);
}

int main(int argc, char *argv[]) {
    int use_acc = 1;
    int test_case = 0;
    
    /* Use command line argument to control execution */
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (argc > 2) {
            use_acc = atoi(argv[2]);
        }
    }
    
    printf("Running OpenACC partitioning tests (use_acc=%d, test_case=%d)\n", 
           use_acc, test_case);
    
    /* Execute different tests based on input */
    switch (test_case) {
        case 0:
            test_gang_redundant(use_acc);
            break;
        case 1:
            test_gang_partitioned(use_acc);
            break;
        case 2:
            test_worker_partitioned(use_acc);
            break;
        case 3:
            test_gang_worker_partitioned(use_acc);
            break;
        case 4:
            test_vector_partitioned(use_acc);
            break;
        case 5:
            test_gang_vector_partitioned(use_acc);
            break;
        case 6:
            test_worker_vector_partitioned(use_acc);
            break;
        case 7:
            test_fully_partitioned(use_acc);
            break;
        case 8:
            test_kernels_partitioning(use_acc);
            break;
        default:
            /* Run all tests to ensure all code paths are compiled */
            test_gang_redundant(use_acc);
            test_gang_partitioned(use_acc);
            test_worker_partitioned(use_acc);
            test_gang_worker_partitioned(use_acc);
            test_vector_partitioned(use_acc);
            test_gang_vector_partitioned(use_acc);
            test_worker_vector_partitioned(use_acc);
            test_fully_partitioned(use_acc);
            test_kernels_partitioning(use_acc);
            break;
    }
    
    return 0;
}
