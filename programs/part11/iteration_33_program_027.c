/* test_oacc_partition.c - Exercise OpenACC partitioning cases */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Case 0: gang redundant */
void test_gang_redundant(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    #pragma acc parallel if(use_acc) copy(data[0:N]) copy(sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < N; i++) {
            data[i] = i * 0.5f;
        }
        sum = data[N-1];
    }
    
    printf("Gang redundant: sum = %f\n", sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(int use_acc) {
    float data[N];
    float sum = 0.0f;
    
    #pragma acc parallel if(use_acc) copy(data[0:N]) copy(sum)
    {
        #pragma acc loop gang reduction(+:sum)
        for (int i = 0; i < N; i++) {
            data[i] = i * 1.5f;
            sum += data[i];
        }
    }
    
    printf("Gang partitioned: sum = %f\n", sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(int use_acc) {
    float data[M][M];
    float partial_sums[M];
    
    #pragma acc parallel if(use_acc) copy(data[0:M][0:M]) create(partial_sums[0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                data[i][j] = (i + j) * 0.1f;
            }
            
            partial_sums[i] = 0.0f;
            #pragma acc loop worker reduction(+:partial_sums[i])
            for (int j = 0; j < M; j++) {
                partial_sums[i] += data[i][j];
            }
        }
    }
    
    printf("Worker partitioned: partial_sum[0] = %f\n", partial_sums[0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(int use_acc) {
    float data[M][M];
    float sum = 0.0f;
    
    #pragma acc parallel if(use_acc) copy(data[0:M][0:M]) copy(sum)
    {
        #pragma acc loop gang worker collapse(2) reduction(+:sum)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                data[i][j] = (i * M + j) * 0.25f;
                sum += data[i][j];
            }
        }
    }
    
    printf("Gang+worker partitioned: sum = %f\n", sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(int use_acc) {
    float data[N];
    
    #pragma acc parallel if(use_acc) copy(data[0:N])
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            data[i] = data[i] * 2.0f + 1.0f;
        }
    }
    
    printf("Vector partitioned: data[0] = %f, data[%d] = %f\n", 
           data[0], N-1, data[N-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(int use_acc) {
    float data[N];
    float max_val = 0.0f;
    
    #pragma acc parallel if(use_acc) copy(data[0:N]) copy(max_val)
    {
        #pragma acc loop gang vector reduction(max:max_val)
        for (int i = 0; i < N; i++) {
            data[i] = i * 3.14f;
            if (data[i] > max_val) max_val = data[i];
        }
    }
    
    printf("Gang+vector partitioned: max = %f\n", max_val);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(int use_acc) {
    float data[M][M];
    
    #pragma acc parallel if(use_acc) copy(data[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                data[i][j] = (i * 0.3f) + (j * 0.7f);
            }
        }
    }
    
    printf("Worker+vector partitioned: data[%d][%d] = %f\n", 
           M/2, M/2, data[M/2][M/2]);
}

/* Case 7: fully partitioned */
void test_fully_partitioned(int use_acc) {
    float data[M][M];
    float result[M][M];
    
    /* Initialize data */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            data[i][j] = (i + j) * 0.5f;
            result[i][j] = 0.0f;
        }
    }
    
    #pragma acc parallel if(use_acc) copy(data[0:M][0:M]) copy(result[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                float sum = 0.0f;
                #pragma acc loop vector reduction(+:sum)
                for (int k = -1; k <= 1; k++) {
                    for (int l = -1; l <= 1; l++) {
                        sum += data[i+k][j+l];
                    }
                }
                result[i][j] = sum / 9.0f;
            }
        }
    }
    
    printf("Fully partitioned: result[1][1] = %f, result[%d][%d] = %f\n",
           result[1][1], M-2, M-2, result[M-2][M-2]);
}

/* Additional test with kernels directive */
void test_kernels_partitioning(int use_acc) {
    float a[N], b[N], c[N];
    
    #pragma acc kernels if(use_acc) copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            a[i] = i * 0.1f;
            b[i] = i * 0.2f;
        }
        
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    printf("Kernels partitioning: c[0] = %f, c[%d] = %f\n", c[0], N-1, c[N-1]);
}

int main(int argc, char *argv[]) {
    int test_case = 0;
    int use_acc = 1;
    
    /* Use command line argument to select test case */
    if (argc > 1) {
        test_case = atoi(argv[1]);
        if (argc > 2) {
            use_acc = atoi(argv[2]);
        }
    }
    
    printf("Running test case %d with use_acc=%d\n", test_case, use_acc);
    
    /* Execute selected test case */
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
            /* Run all tests */
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
