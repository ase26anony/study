/* test_oacc_partition.c - Exercise OpenACC partitioning cases */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyout(sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5f;
        }
        sum = arr[0] + arr[n-1];
    }
    
    printf("Gang redundant: sum = %f\n", sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float reduction_sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:reduction_sum) copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i;
        reduction_sum += arr[i];
    }
    
    printf("Gang partitioned: reduction = %f\n", reduction_sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M], int m) {
    float temp[M];
    
    #pragma acc parallel copy(arr[0:m][0:m]) create(temp[0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                temp[j] = arr[i][j] * 0.5f;
                arr[i][j] = temp[j] + i + j;
            }
        }
    }
    
    printf("Worker partitioned: arr[0][0] = %f\n", arr[0][0]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M], int m) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:m][0:m]) reduction(+:local_sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                arr[i][j] = (arr[i][j] + i - j) * 1.1f;
                local_sum += arr[i][j];
            }
        }
    }
    
    printf("Gang+worker partitioned: sum = %f\n", local_sum);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        // Element-wise operations suitable for vectorization
        arr[i] = arr[i] * arr[i] - arr[i] / 3.0f + i * 0.01f;
    }
    
    printf("Vector partitioned: arr[10] = %f\n", arr[10]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *arr, int n) {
    float partial_sums[16];
    
    #pragma acc data copy(arr[0:n]) create(partial_sums[0:16])
    {
        #pragma acc parallel loop gang vector
        for (int i = 0; i < n; i++) {
            int gang_id = i / (n/16);
            arr[i] = arr[i] + i * 0.5f;
            #pragma acc atomic update
            partial_sums[gang_id] += arr[i];
        }
    }
    
    printf("Gang+vector partitioned: partial_sums[0] = %f\n", partial_sums[0]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            #pragma acc loop worker vector
            for (int idx = 0; idx < m*m/4; idx++) {
                int i = (block * m/4) + (idx / m);
                int j = idx % m;
                if (i < m && j < m) {
                    arr[i][j] = (arr[i][j] + 1.0f) / (j + 2.0f);
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: arr[5][5] = %f\n", arr[5][5]);
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float arr3d[P][M][M], int p, int m) {
    float reduction_total = 0.0f;
    
    #pragma acc parallel copy(arr3d[0:p][0:m][0:m]) reduction(+:reduction_total)
    {
        #pragma acc loop gang
        for (int k = 0; k < p; k++) {
            #pragma acc loop worker
            for (int i = 1; i < m-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < m-1; j++) {
                    // Stencil computation with data dependencies
                    float neighbor_sum = arr3d[k][i-1][j] + arr3d[k][i][j-1] +
                                         arr3d[k][i+1][j] + arr3d[k][i][j+1];
                    arr3d[k][i][j] = neighbor_sum * 0.25f + k * 0.1f;
                    reduction_total += arr3d[k][i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned: total = %f, arr3d[1][10][10] = %f\n", 
           reduction_total, arr3d[1][10][10]);
}

/* Helper to initialize arrays */
void init_array(float *arr, int n, float val) {
    for (int i = 0; i < n; i++) {
        arr[i] = val + i * 0.1f;
    }
}

void init_2d_array(float arr[M][M], int m, float val) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = val + i * 0.5f + j * 0.3f;
        }
    }
}

void init_3d_array(float arr[P][M][M], int p, int m, float val) {
    for (int k = 0; k < p; k++) {
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < m; j++) {
                arr[k][i][j] = val + k * 0.7f + i * 0.5f + j * 0.3f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    float arr1[N];
    float arr2[M][M];
    float arr3d[P][M][M];
    
    /* Initialize data */
    init_array(arr1, N, 1.0f);
    init_2d_array(arr2, M, 2.0f);
    init_3d_array(arr3d, P, M, 0.5f);
    
    /* Use argc to control execution, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) % 8 : 0;
    
    switch (test_case) {
        case 0:
            test_gang_redundant(arr1, N);
            break;
        case 1:
            test_gang_partitioned(arr1, N);
            break;
        case 2:
            test_worker_partitioned(arr2, M);
            break;
        case 3:
            test_gang_worker_partitioned(arr2, M);
            break;
        case 4:
            test_vector_partitioned(arr1, N);
            break;
        case 5:
            test_gang_vector_partitioned(arr1, N);
            break;
        case 6:
            test_worker_vector_partitioned(arr2, M);
            break;
        case 7:
            test_fully_partitioned(arr3d, P, M);
            break;
        default:
            /* Execute all tests if no specific case requested */
            test_gang_redundant(arr1, N);
            test_gang_partitioned(arr1, N);
            test_worker_partitioned(arr2, M);
            test_gang_worker_partitioned(arr2, M);
            test_vector_partitioned(arr1, N);
            test_gang_vector_partitioned(arr1, N);
            test_worker_vector_partitioned(arr2, M);
            test_fully_partitioned(arr3d, P, M);
            break;
    }
    
    /* Print some results to prevent dead code elimination */
    printf("Final check - arr1[0]=%f, arr1[%d]=%f\n", arr1[0], N-1, arr1[N-1]);
    printf("arr2[%d][%d]=%f\n", M-1, M-1, arr2[M-1][M-1]);
    
    return 0;
}
