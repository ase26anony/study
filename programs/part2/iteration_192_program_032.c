#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force volatile to prevent optimization */
volatile int use_partition_type = 0;

/* Struct with array members for complex mapping */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Function to compute checksum */
int compute_checksum(int *data, size_t n) {
    int sum = 0;
    for (size_t i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Multi-dimensional arrays for different partition types */
    int arr_3d[20][30][40];
    int arr_2d[100][100];
    int arr_1d[1000];
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Dynamic arrays */
    int *dyn_arr = (int*)malloc(500 * sizeof(int));
    double *dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    if (!dyn_arr || !dyn_matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    memset(arr_3d, 0, sizeof(arr_3d));
    memset(arr_2d, 0, sizeof(arr_2d));
    memset(arr_1d, 0, sizeof(arr_1d));
    memset(&container, 0, sizeof(container));
    memset(dyn_arr, 0, 500 * sizeof(int));
    memset(dyn_matrix, 0, 100 * 100 * sizeof(double));
    
    int total_checksum = 0;
    
    /* Use command line argument to control partition type if provided */
    if (argc > 1) {
        use_partition_type = atoi(argv[1]);
    }
    
    /* Case 0: Gang redundant (default mapping) */
    #pragma acc parallel loop gang copy(arr_1d[0:1000])
    for (int i = 0; i < 1000; i++) {
        arr_1d[i] += i % 7;
    }
    
    /* Case 1: Gang partitioned */
    #pragma acc parallel loop gang copy(arr_2d[0:100][gang])
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr_2d[i][j] += (i + j) % 11;
        }
    }
    
    /* Case 2: Worker partitioned */
    #pragma acc parallel loop gang worker copy(arr_1d[0:1000][worker])
    for (int i = 0; i < 1000; i++) {
        arr_1d[i] += i % 13;
    }
    
    /* Case 3: Gang+Worker partitioned */
    #pragma acc data copy(arr_3d[0:10][0:20][0:30][gang+worker])
    {
        #pragma acc parallel loop gang worker collapse(2)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 20; j++) {
                for (int k = 0; k < 30; k++) {
                    arr_3d[i][j][k] += (i * j + k) % 17;
                }
            }
        }
    }
    
    /* Case 4: Vector partitioned */
    #pragma acc parallel loop vector copy(dyn_arr[0:500][vector])
    for (int i = 0; i < 500; i++) {
        dyn_arr[i] += i % 19;
    }
    
    /* Case 5: Gang+Vector partitioned */
    #pragma acc parallel loop gang vector copy(container.matrix[0:50][gang][vector])
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            container.matrix[i][j] += (i * 31 + j) % 23;
        }
    }
    
    /* Case 6: Worker+Vector partitioned */
    #pragma acc parallel loop gang worker vector copy(container.vector[0:1000][worker+vector])
    for (int i = 0; i < 1000; i++) {
        container.vector[i] += i % 29;
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    #pragma acc data copy(dyn_matrix[0:10000][gang+worker+vector])
    {
        #pragma acc parallel loop gang worker vector collapse(2)
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                dyn_matrix[i * 100 + j] += (i * 37 + j) % 31;
            }
        }
    }
    
    /* Conditional partition selection using volatile variable */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (use_partition_type == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc parallel loop gang copy(arr_1d[0:100])
                    for (int i = 0; i < 100; i++) arr_1d[i] += 1;
                    break;
                case 1:
                    #pragma acc parallel loop gang copy(arr_2d[0:50][0:50][gang])
                    for (int i = 0; i < 50; i++) 
                        for (int j = 0; j < 50; j++) arr_2d[i][j] += 1;
                    break;
                case 2:
                    #pragma acc parallel loop gang worker copy(arr_1d[0:200][worker])
                    for (int i = 0; i < 200; i++) arr_1d[i] += 1;
                    break;
                case 3:
                    #pragma acc data copy(arr_3d[0:5][0:10][0:15][gang+worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (int i = 0; i < 5*10*15; i++) arr_3d[0][0][i] += 1;
                    }
                    break;
                case 4:
                    #pragma acc parallel loop vector copy(dyn_arr[0:100][vector])
                    for (int i = 0; i < 100; i++) dyn_arr[i] += 1;
                    break;
                case 5:
                    #pragma acc parallel loop gang vector copy(container.values[0:100][gang+vector])
                    for (int i = 0; i < 100; i++) container.values[i] += 1.0;
                    break;
                case 6:
                    #pragma acc parallel loop gang worker vector copy(container.vector[0:100][worker+vector])
                    for (int i = 0; i < 100; i++) container.vector[i] += 1;
                    break;
                case 7:
                    #pragma acc data copy(arr_2d[0:10][0:10][gang+worker+vector])
                    {
                        #pragma acc parallel loop gang worker vector
                        for (int i = 0; i < 100; i++) arr_2d[0][i] += 1;
                    }
                    break;
            }
        }
    }
    
    /* Nested constructs with structured data movement */
    #pragma acc enter data copyin(container.matrix[0:25][0:25][gang])
    
    #pragma acc parallel loop gang present(container.matrix[0:25][0:25][gang])
    for (int i = 0; i < 25; i++) {
        for (int j = 0; j < 25; j++) {
            container.matrix[i][j] *= 2;
        }
    }
    
    #pragma acc exit data copyout(container.matrix[0:25][0:25][gang])
    
    /* Complex multi-dimensional partition with different dimensions */
    #pragma acc data copy(arr_3d[0:10][gang][0:30][worker][0:40][vector])
    {
        #pragma acc parallel loop gang worker vector collapse(3)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 30; j++) {
                for (int k = 0; k < 40; k++) {
                    arr_3d[i][j][k] += (i + j + k) % 41;
                }
            }
        }
    }
    
    /* Compute checksums to ensure computations aren't optimized away */
    total_checksum += compute_checksum(&arr_1d[0], 1000);
    total_checksum += compute_checksum(&arr_2d[0][0], 100 * 100);
    total_checksum += compute_checksum(&arr_3d[0][0][0], 20 * 30 * 40);
    total_checksum += compute_checksum(&container.vector[0], 1000);
    total_checksum += compute_checksum(dyn_arr, 500);
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
