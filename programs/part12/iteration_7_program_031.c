/* test_openacc_partitions.c */
#include <stdio.h>
#include <stdlib.h>

#define N 10
#define M 20
#define P 30

/* Routine with gang partitioning */
#pragma acc routine seq
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
}

/* Routine with gang partitioning */
#pragma acc routine gang
void gang_partitioned_func(int *arr, int size, int offset) {
    #pragma acc loop gang
    for (int i = 0; i < size; i++) {
        arr[i] += offset;
    }
}

/* Routine with vector partitioning */
#pragma acc routine vector
void vector_partitioned_func(int *arr, int size, int scale) {
    #pragma acc loop vector
    for (int i = 0; i < size; i++) {
        arr[i] *= scale;
    }
}

/* Test 1: Basic partition combinations */
void test_basic_partitions(int argc) {
    int arr1[N][M][P];
    int arr2[N][M];
    int arr3[N];
    
    /* Initialize arrays */
    #pragma acc parallel loop collapse(3) gang, worker, vector copy(arr1[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr1[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    /* Gang redundant partitioning */
    #pragma acc parallel copy(arr2[0:N][0:M]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr2[i][j] = arr1[i][j][0] * 2;
            }
        }
    }
    
    /* Worker partitioned */
    #pragma acc kernels create(arr3[0:N]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            arr3[i] = 0;
            for (int j = 0; j < M; j++) {
                arr3[i] += arr2[i][j];
            }
        }
    }
    
    /* Vector partitioned with if condition */
    if (argc > 1) {
        #pragma acc parallel loop vector copy(arr3[0:N])
        for (int i = 0; i < N; i++) {
            arr3[i] += 100;
        }
    }
}

/* Test 2: Mixed partition types */
void test_mixed_partitions() {
    int matrix[N][M];
    int result[N];
    
    /* Enter data with gang partitioning */
    #pragma acc enter data copyin(matrix[0:N][0:M]) gang
    
    /* Initialize with gang+worker partitioning */
    #pragma acc parallel present(matrix) gang, worker
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                matrix[i][j] = i * M + j;
            }
        }
    }
    
    /* Process with gang+vector partitioning */
    #pragma acc parallel loop gang, vector copy(result[0:N])
    for (int i = 0; i < N; i++) {
        int sum = 0;
        #pragma acc loop vector reduction(+:sum)
        for (int j = 0; j < M; j++) {
            sum += matrix[i][j];
        }
        result[i] = sum;
    }
    
    /* Worker+vector partitioned update */
    #pragma acc kernels present(matrix) worker, vector
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                matrix[i][j] += result[i % N];
            }
        }
    }
    
    /* Fully partitioned computation */
    #pragma acc parallel loop collapse(2) gang, worker, vector copy(matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = matrix[i][j] % 256;
        }
    }
    
    #pragma acc exit data copyout(matrix[0:N][0:M])
}

/* Test 3: Nested regions with routine calls */
void test_nested_with_routines(int argc) {
    int data[N * M * P];
    int processed[N];
    
    /* Initialize with device data environment */
    #pragma acc enter data create(data[0:N*M*P], processed[0:N]) gang
    
    /* Call gang-partitioned routine */
    #pragma acc parallel present(data) gang
    {
        gang_partitioned_func(data, N * M * P, argc);
    }
    
    /* Nested conditional regions */
    if (argc > 2) {
        #pragma acc parallel present(data) worker
        {
            /* Inner region with different partition */
            #pragma acc loop vector
            for (int i = 0; i < N * M * P; i += 100) {
                data[i] *= 2;
            }
        }
    }
    
    /* Call vector-partitioned routine */
    #pragma acc parallel loop present(data, processed) vector
    for (int i = 0; i < N; i++) {
        int sum = 0;
        for (int j = 0; j < M * P; j++) {
            sum += data[i * M * P + j];
        }
        processed[i] = sum;
        vector_partitioned_func(&processed[i], 1, 3);
    }
    
    #pragma acc exit data copyout(data[0:N*M*P], processed[0:N])
}

/* Test 4: Complex multi-dimensional partitioning */
void test_complex_partitioning() {
    int cube[N][M][P];
    int slices[N][M];
    int rows[N];
    
    /* Gang partitioned initialization */
    #pragma acc parallel loop gang copy(cube[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            #pragma acc loop vector
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = (i + j + k) % 100;
            }
        }
    }
    
    /* Worker partitioned reduction */
    #pragma acc kernels create(slices[0:N][0:M]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                int sum = 0;
                for (int k = 0; k < P; k++) {
                    sum += cube[i][j][k];
                }
                slices[i][j] = sum;
            }
        }
    }
    
    /* Vector partitioned final reduction */
    #pragma acc parallel loop vector copy(rows[0:N])
    for (int i = 0; i < N; i++) {
        rows[i] = 0;
        for (int j = 0; j < M; j++) {
            rows[i] += slices[i][j];
        }
    }
    
    /* Gang+worker+vector fully partitioned update */
    #pragma acc parallel loop collapse(3) gang, worker, vector copy(cube[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                cube[i][j][k] = (cube[i][j][k] + rows[i]) % 256;
            }
        }
    }
}

int main(int argc, char **argv) {
    printf("Testing OpenACC partition coverage...\n");
    
    /* Execute tests based on argc to prevent dead code elimination */
    test_basic_partitions(argc);
    
    if (argc > 1) {
        test_mixed_partitions();
    }
    
    if (argc > 2) {
        test_nested_with_routines(argc);
    }
    
    if (argc > 3) {
        test_complex_partitioning();
    }
    
    /* Additional conditional tests to cover all paths */
    int small[5][5];
    
    /* Gang partitioned */
    #pragma acc parallel copy(small[0:5][0:5]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < 5; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 5; j++) {
                small[i][j] = i * 5 + j;
            }
        }
    }
    
    /* Worker partitioned */
    #pragma acc kernels copy(small[0:5][0:5]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < 5; i++) {
            #pragma acc loop vector
            for (int j = 0; j < 5; j++) {
                small[i][j] += 1;
            }
        }
    }
    
    /* Vector partitioned */
    #pragma acc parallel loop collapse(2) vector copy(small[0:5][0:5])
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            small[i][j] *= 2;
        }
    }
    
    printf("Test completed.\n");
    return 0;
}
