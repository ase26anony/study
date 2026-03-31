#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define VOLATILE volatile
#else
#define VOLATILE
#endif

/* Struct with array members for testing nested partitions */
struct DataContainer {
    int matrix[50][50];
    float vector[1000];
    double grid[20][20][20];
};

/* Function to prevent dead code elimination */
void use_result(int result) {
    VOLATILE int dummy = result;
    if (dummy == 0) printf(""); /* Prevent optimization */
}

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for different partition types */
    int arr1d[1000];
    int arr2d[100][100];
    int arr3d[50][50][50];
    
    /* Dynamic arrays for pointer-based mappings */
    int *dyn_arr = (int*)malloc(2000 * sizeof(int));
    double *dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    /* Struct with array members */
    struct DataContainer container;
    
    /* Initialize arrays */
    for (i = 0; i < 1000; i++) arr1d[i] = i % 100;
    for (i = 0; i < 100; i++)
        for (j = 0; j < 100; j++)
            arr2d[i][j] = i + j;
    
    for (i = 0; i < 50; i++)
        for (j = 0; j < 50; j++)
            for (k = 0; k < 50; k++)
                arr3d[i][j][k] = i * j * k;
    
    for (i = 0; i < 2000; i++) dyn_arr[i] = i % 255;
    for (i = 0; i < 10000; i++) dyn_matrix[i] = i * 0.1;
    
    for (i = 0; i < 50; i++)
        for (j = 0; j < 50; j++)
            container.matrix[i][j] = i * 100 + j;
    
    /* Use volatile to prevent constant propagation and dead code elimination */
    VOLATILE int partition_selector = 0;
    if (argc > 1) partition_selector = atoi(argv[1]);
    
    /* Case 0: gang redundant (default mapping) */
    #pragma acc data copy(arr1d[0:1000])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 1000; i++) {
            arr1d[i] += 1;
        }
    }
    
    /* Case 1: gang partitioned */
    #pragma acc data copy(arr2d[0:100][gang])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 2;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    #pragma acc data copy(arr2d[0:100][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 100; i++) {
            #pragma acc loop worker
            for (j = 0; j < 100; j++) {
                arr2d[i][j] += 3;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc data copy(arr3d[0:50][gang][worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker
            for (j = 0; j < 50; j++) {
                for (k = 0; k < 50; k++) {
                    arr3d[i][j][k] += 4;
                }
            }
        }
    }
    
    /* Case 4: vector partitioned */
    #pragma acc data copy(dyn_arr[0:2000][vector])
    {
        #pragma acc parallel loop vector
        for (i = 0; i < 2000; i++) {
            dyn_arr[i] += 5;
        }
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc data copy(dyn_matrix[0:10000][gang+vector])
    {
        #pragma acc parallel loop gang vector
        for (i = 0; i < 100; i++) {
            #pragma acc loop vector
            for (j = 0; j < 100; j++) {
                dyn_matrix[i*100 + j] += 6;
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc data copy(container.matrix[0:50][worker+vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] += 7;
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc data copy(arr3d[0:50][gang][worker][vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 50; j++) {
                #pragma acc loop vector
                for (k = 0; k < 50; k++) {
                    arr3d[i][j][k] += 8;
                }
            }
        }
    }
    
    /* Test with enter/exit data for structured data movement */
    int *structured_arr = (int*)malloc(500 * sizeof(int));
    for (i = 0; i < 500; i++) structured_arr[i] = i;
    
    #pragma acc enter data copyin(structured_arr[0:500][gang])
    #pragma acc parallel loop gang present(structured_arr[0:500][gang])
    for (i = 0; i < 500; i++) {
        structured_arr[i] += 9;
    }
    #pragma acc exit data copyout(structured_arr[0:500][gang])
    
    /* Conditional partition selection to force compiler to consider multiple paths */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (partition_selector == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr1d[0:100])
                    { /* gang redundant */ }
                    break;
                case 1:
                    #pragma acc data copy(arr1d[0:100][gang])
                    { /* gang partitioned */ }
                    break;
                case 2:
                    #pragma acc data copy(arr1d[0:100][worker])
                    { /* worker partitioned */ }
                    break;
                case 3:
                    #pragma acc data copy(arr2d[0:100][gang][worker])
                    { /* gang+worker partitioned */ }
                    break;
                case 4:
                    #pragma acc data copy(arr1d[0:100][vector])
                    { /* vector partitioned */ }
                    break;
                case 5:
                    #pragma acc data copy(arr2d[0:100][gang+vector])
                    { /* gang+vector partitioned */ }
                    break;
                case 6:
                    #pragma acc data copy(arr2d[0:100][worker+vector])
                    { /* worker+vector partitioned */ }
                    break;
                case 7:
                    #pragma acc data copy(arr3d[0:50][gang][worker][vector])
                    { /* fully partitioned */ }
                    break;
            }
        }
    }
    
    /* Compute checksum to ensure computations aren't optimized away */
    for (i = 0; i < 1000; i++) checksum += arr1d[i];
    for (i = 0; i < 100; i++)
        for (j = 0; j < 100; j++)
            checksum += arr2d[i][j];
    
    for (i = 0; i < 2000; i++) checksum += dyn_arr[i];
    for (i = 0; i < 500; i++) checksum += structured_arr[i];
    
    use_result(checksum);
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    free(structured_arr);
    
    return 0;
}
