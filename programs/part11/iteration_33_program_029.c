/* test_oacc_partition.c - Test OpenACC partitioning for coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 32
#define P 16

/* Case 0: gang redundant */
void test_gang_redundant(float *arr, int n) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyout(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < n; i++) {
            arr[i] = i * 1.5f;
        }
        local_sum = arr[0] + arr[n-1];
    }
    
    printf("Gang redundant: arr[0]=%.2f, arr[%d]=%.2f, sum=%.2f\n", 
           arr[0], n-1, arr[n-1], local_sum);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:n]) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + i * 0.1f;
        sum += arr[i];
    }
    
    printf("Gang partitioned: sum=%.2f, arr[0]=%.2f\n", sum, arr[0]);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr[M][M], int m) {
    float row_sums[M] = {0};
    
    #pragma acc parallel copy(arr[0:m][0:m]) copyout(row_sums[0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = (i + j) * 0.5f;
                row_sums[i] += arr[i][j];
            }
        }
    }
    
    printf("Worker partitioned: row_sums[0]=%.2f, row_sums[%d]=%.2f\n",
           row_sums[0], m-1, row_sums[m-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr[M][M], int m) {
    float total = 0.0f;
    
    #pragma acc parallel loop gang worker copy(arr[0:m][0:m]) reduction(+:total)
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = (arr[i][j] + i - j) * 1.1f;
            total += arr[i][j];
        }
    }
    
    printf("Gang+worker partitioned: total=%.2f\n", total);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * arr[i] - 2.0f * arr[i] + 1.0f;
    }
    
    printf("Vector partitioned: arr[0]=%.2f, arr[%d]=%.2f\n", 
           arr[0], n-1, arr[n-1]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float arr[M][M], int m) {
    float col_max[M] = {0};
    
    #pragma acc parallel copy(arr[0:m][0:m]) copyout(col_max[0:m])
    {
        #pragma acc loop gang vector
        for (int j = 0; j < m; j++) {
            float max_val = arr[0][j];
            #pragma acc loop seq
            for (int i = 0; i < m; i++) {
                if (arr[i][j] > max_val) max_val = arr[i][j];
            }
            col_max[j] = max_val;
        }
    }
    
    printf("Gang+vector partitioned: col_max[0]=%.2f, col_max[%d]=%.2f\n",
           col_max[0], m-1, col_max[m-1]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr[M][M], int m) {
    float diag_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:m][0:m]) reduction(+:diag_sum)
    {
        #pragma acc loop gang
        for (int k = 0; k < m; k++) {
            #pragma acc loop worker vector
            for (int i = 0; i < m; i++) {
                int j = (i + k) % m;
                arr[i][j] = arr[i][j] * 0.9f + k * 0.1f;
                if (i == j) diag_sum += arr[i][j];
            }
        }
    }
    
    printf("Worker+vector partitioned: diag_sum=%.2f\n", diag_sum);
}

/* Case 7: fully partitioned - complex nested computation */
void test_fully_partitioned(float arr3d[M][M][P], int m, int p) {
    float layer_avg[P] = {0};
    
    #pragma acc parallel copy(arr3d[0:m][0:m][0:p]) copyout(layer_avg[0:p])
    {
        #pragma acc loop gang
        for (int k = 0; k < p; k++) {
            float layer_sum = 0.0f;
            #pragma acc loop worker
            for (int i = 1; i < m-1; i++) {
                #pragma acc loop vector
                for (int j = 1; j < m-1; j++) {
                    /* Stencil computation with dependencies */
                    arr3d[i][j][k] = (arr3d[i-1][j][k] + 
                                     arr3d[i][j-1][k] + 
                                     arr3d[i][j][k]) * 0.333f;
                    layer_sum += arr3d[i][j][k];
                }
            }
            layer_avg[k] = layer_sum / ((m-2)*(m-2));
        }
    }
    
    printf("Fully partitioned: layer_avg[0]=%.2f, layer_avg[%d]=%.2f\n",
           layer_avg[0], p-1, layer_avg[p-1]);
}

/* Helper to initialize arrays */
void init_array(float *arr, int n, float val) {
    for (int i = 0; i < n; i++) arr[i] = val + i * 0.01f;
}

void init_2d_array(float arr[M][M], int m, float base) {
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            arr[i][j] = base + i * 0.1f + j * 0.01f;
}

void init_3d_array(float arr[M][M][P], int m, int p, float base) {
    for (int i = 0; i < m; i++)
        for (int j = 0; j < m; j++)
            for (int k = 0; k < p; k++)
                arr[i][j][k] = base + i * 0.1f + j * 0.01f + k * 0.001f;
}

int main(int argc, char *argv[]) {
    float arr1[N];
    float arr2[M][M];
    float arr3[M][M][P];
    
    /* Initialize arrays */
    init_array(arr1, N, 1.0f);
    init_2d_array(arr2, M, 2.0f);
    init_3d_array(arr3, M, P, 0.5f);
    
    /* Use argc to control execution, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) % 8 : 0;
    
    switch(test_case) {
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
            test_gang_vector_partitioned(arr2, M);
            break;
        case 6:
            test_worker_vector_partitioned(arr2, M);
            break;
        case 7:
            test_fully_partitioned(arr3, M, P);
            break;
        default:
            /* Execute all tests in sequence when no specific case */
            test_gang_redundant(arr1, N);
            init_array(arr1, N, 1.0f);
            
            test_gang_partitioned(arr1, N);
            init_2d_array(arr2, M, 2.0f);
            
            test_worker_partitioned(arr2, M);
            init_2d_array(arr2, M, 2.0f);
            
            test_gang_worker_partitioned(arr2, M);
            init_array(arr1, N, 1.0f);
            
            test_vector_partitioned(arr1, N);
            init_2d_array(arr2, M, 2.0f);
            
            test_gang_vector_partitioned(arr2, M);
            init_2d_array(arr2, M, 2.0f);
            
            test_worker_vector_partitioned(arr2, M);
            init_3d_array(arr3, M, P, 0.5f);
            
            test_fully_partitioned(arr3, M, P);
            break;
    }
    
    /* Print final values to prevent dead code elimination */
    printf("Final check: arr1[0]=%.2f, arr2[0][0]=%.2f, arr3[0][0][0]=%.2f\n",
           arr1[0], arr2[0][0], arr3[0][0][0]);
    
    return 0;
}
