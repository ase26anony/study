/* test_oacc_partition.c - Test program for OpenACC partitioning coverage */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define DIM 32

/* Case 0: gang redundant */
void test_gang_redundant(float *arr) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:SIZE]) copyin(local_sum)
    {
        #pragma acc loop gang(1)
        for (int i = 0; i < SIZE; i++) {
            arr[i] = i * 0.5f;
        }
        local_sum = arr[0];  // Simple assignment in gang-redundant region
    }
    
    printf("Gang redundant: arr[0]=%.2f, arr[%d]=%.2f\n", 
           arr[0], SIZE-1, arr[SIZE-1]);
}

/* Case 1: gang partitioned */
void test_gang_partitioned(float *arr) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang reduction(+:sum) copy(arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        arr[i] = arr[i] * 2.0f + i;
        sum += arr[i];
    }
    
    printf("Gang partitioned: sum=%.2f\n", sum);
}

/* Case 2: worker partitioned */
void test_worker_partitioned(float arr2d[DIM][DIM]) {
    float row_sums[DIM] = {0};
    
    #pragma acc parallel copy(arr2d) copyout(row_sums)
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker reduction(+:row_sums[i])
            for (int j = 0; j < DIM; j++) {
                arr2d[i][j] = (i + j) * 0.1f;
                row_sums[i] += arr2d[i][j];
            }
        }
    }
    
    printf("Worker partitioned: row_sums[0]=%.2f, row_sums[%d]=%.2f\n",
           row_sums[0], DIM-1, row_sums[DIM-1]);
}

/* Case 3: gang+worker partitioned */
void test_gang_worker_partitioned(float arr2d[DIM][DIM]) {
    float total = 0.0f;
    
    #pragma acc parallel copy(arr2d) reduction(+:total)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                arr2d[i][j] = (arr2d[i][j] + i - j) * 1.5f;
                total += arr2d[i][j];
            }
        }
    }
    
    printf("Gang+worker partitioned: total=%.2f\n", total);
}

/* Case 4: vector partitioned */
void test_vector_partitioned(float *arr) {
    #pragma acc parallel loop vector copy(arr[0:SIZE])
    for (int i = 0; i < SIZE; i++) {
        // Element-wise operations suitable for vectorization
        arr[i] = arr[i] * arr[i] + 1.0f / (arr[i] + 0.001f);
    }
    
    printf("Vector partitioned: arr[0]=%.2f\n", arr[0]);
}

/* Case 5: gang+vector partitioned */
void test_gang_vector_partitioned(float *arr) {
    float partial_sums[16] = {0};
    
    #pragma acc parallel copy(arr[0:SIZE]) create(partial_sums)
    {
        #pragma acc loop gang vector
        for (int i = 0; i < SIZE; i++) {
            arr[i] = i * 0.25f;
            int gang_id = i / 64;
            if (gang_id < 16) {
                partial_sums[gang_id] += arr[i];
            }
        }
    }
    
    printf("Gang+vector partitioned: partial_sums[0]=%.2f\n", partial_sums[0]);
}

/* Case 6: worker+vector partitioned */
void test_worker_vector_partitioned(float arr2d[DIM][DIM]) {
    #pragma acc parallel copy(arr2d)
    {
        #pragma acc loop gang
        for (int i = 0; i < DIM; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < DIM; j++) {
                // Stencil-like computation
                float left = (j > 0) ? arr2d[i][j-1] : 0.0f;
                float up = (i > 0) ? arr2d[i-1][j] : 0.0f;
                arr2d[i][j] = (left + up) * 0.5f + 1.0f;
            }
        }
    }
    
    printf("Worker+vector partitioned: arr2d[%d][%d]=%.2f\n",
           DIM-1, DIM-1, arr2d[DIM-1][DIM-1]);
}

/* Case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned(float arr3d[8][DIM][DIM]) {
    float grand_total = 0.0f;
    
    #pragma acc parallel copy(arr3d) reduction(+:grand_total)
    {
        #pragma acc loop gang
        for (int k = 0; k < 8; k++) {
            #pragma acc loop worker
            for (int i = 0; i < DIM; i++) {
                #pragma acc loop vector
                for (int j = 0; j < DIM; j++) {
                    // Complex 3D stencil computation
                    float center = arr3d[k][i][j];
                    float left = (j > 0) ? arr3d[k][i][j-1] : center;
                    float right = (j < DIM-1) ? arr3d[k][i][j+1] : center;
                    float up = (i > 0) ? arr3d[k][i-1][j] : center;
                    float down = (i < DIM-1) ? arr3d[k][i+1][j] : center;
                    float front = (k > 0) ? arr3d[k-1][i][j] : center;
                    float back = (k < 7) ? arr3d[k+1][i][j] : center;
                    
                    arr3d[k][i][j] = (center + left + right + up + down + front + back) / 7.0f;
                    grand_total += arr3d[k][i][j];
                }
            }
        }
    }
    
    printf("Fully partitioned: grand_total=%.2f\n", grand_total);
}

/* Helper to initialize arrays */
void init_array(float *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (i % 10) * 0.1f;
    }
}

void init_2d_array(float arr[DIM][DIM]) {
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            arr[i][j] = (i * DIM + j) * 0.01f;
        }
    }
}

void init_3d_array(float arr[8][DIM][DIM]) {
    for (int k = 0; k < 8; k++) {
        for (int i = 0; i < DIM; i++) {
            for (int j = 0; j < DIM; j++) {
                arr[k][i][j] = (k * DIM * DIM + i * DIM + j) * 0.001f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    float arr1[SIZE];
    float arr2d[DIM][DIM];
    float arr3d[8][DIM][DIM];
    
    init_array(arr1, SIZE);
    init_2d_array(arr2d);
    init_3d_array(arr3d);
    
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 8;
    }
    
    /* Force compiler to analyze all OpenACC regions regardless of execution path */
    if (argc > 2) {
        /* This ensures all code paths are considered during compilation */
        test_gang_redundant(arr1);
        test_gang_partitioned(arr1);
        test_worker_partitioned(arr2d);
        test_gang_worker_partitioned(arr2d);
        test_vector_partitioned(arr1);
        test_gang_vector_partitioned(arr1);
        test_worker_vector_partitioned(arr2d);
        test_fully_partitioned(arr3d);
    } else {
        /* Runtime selection to ensure all functions are compiled */
        switch (test_case) {
            case 0: test_gang_redundant(arr1); break;
            case 1: test_gang_partitioned(arr1); break;
            case 2: test_worker_partitioned(arr2d); break;
            case 3: test_gang_worker_partitioned(arr2d); break;
            case 4: test_vector_partitioned(arr1); break;
            case 5: test_gang_vector_partitioned(arr1); break;
            case 6: test_worker_vector_partitioned(arr2d); break;
            case 7: test_fully_partitioned(arr3d); break;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Final check: arr1[0]=%.2f, arr2d[0][0]=%.2f, arr3d[0][0][0]=%.2f\n",
           arr1[0], arr2d[0][0], arr3d[0][0][0]);
    
    return 0;
}
