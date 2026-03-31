#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[50][50];
    int linear[1000];
    double values[200];
};

/* Function to compute checksum */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for requirement #2 */
    int md_arr1[100][100];
    int md_arr2[100][100];
    int md_arr3[100][100];
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr1[i][j] = i + j;
            md_arr2[i][j] = i * j;
            md_arr3[i][j] = i - j;
        }
    }
    
    /* Dynamic data for requirement #3 */
    int N = 1000;
    int *dyn_arr = (int *)malloc(N * sizeof(int));
    for (i = 0; i < N; i++) {
        dyn_arr[i] = i % 100;
    }
    
    /* Struct instance for requirement #6 */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i * 50 + j;
        }
    }
    for (i = 0; i < 1000; i++) {
        container.linear[i] = i;
    }
    
    printf("Starting OpenACC partition coverage test...\n");
    
    /* Case 0: Gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(md_arr1)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    md_arr1[i][j] += 1;
                }
            }
        }
        checksum += compute_checksum(&md_arr1[0][0], 100*100);
    }
    
    /* Case 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(md_arr1[0:50][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 100; j++) {
                    md_arr1[i][j] += 2;
                }
            }
        }
        checksum += compute_checksum(&md_arr1[0][0], 100*100);
    }
    
    /* Case 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(md_arr2[0:100][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker
                for (j = 0; j < 100; j++) {
                    md_arr2[i][j] += 3;
                }
            }
        }
        checksum += compute_checksum(&md_arr2[0][0], 100*100);
    }
    
    /* Case 3: Gang+worker partitioned - using 2D array with both dimensions partitioned */
    if (use_combined) {
        #pragma acc data copy(md_arr3[0:50][gang][0:50][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker
                for (j = 0; j < 50; j++) {
                    md_arr3[i][j] += 4;
                }
            }
        }
        checksum += compute_checksum(&md_arr3[0][0], 100*100);
    }
    
    /* Case 4: Vector partitioned - using dynamic array */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:N][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < N; i++) {
                dyn_arr[i] += 5;
            }
        }
        checksum += compute_checksum(dyn_arr, N);
    }
    
    /* Case 5: Gang+vector partitioned */
    if (use_gang && use_vector) {
        #pragma acc data copy(md_arr1[0:100][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    md_arr1[i][j] += 6;
                }
            }
        }
        checksum += compute_checksum(&md_arr1[0][0], 100*100);
    }
    
    /* Case 6: Worker+vector partitioned */
    if (use_worker && use_vector) {
        #pragma acc data copy(md_arr2[0:100][worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    md_arr2[i][j] += 7;
                }
            }
        }
        checksum += compute_checksum(&md_arr2[0][0], 100*100);
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) - using 3D-like access pattern */
    if (use_gang && use_worker && use_vector) {
        int arr3d[10][10][10];
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                for (k = 0; k < 10; k++) {
                    arr3d[i][j][k] = i + j + k;
                }
            }
        }
        
        #pragma acc data copy(arr3d[0:10][gang][0:10][worker][0:10][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 10; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 10; j++) {
                    #pragma acc loop vector
                    for (k = 0; k < 10; k++) {
                        arr3d[i][j][k] += 8;
                    }
                }
            }
        }
        checksum += compute_checksum(&arr3d[0][0][0], 10*10*10);
    }
    
    /* Struct member partitions for requirement #6 */
    #pragma acc data copy(container.matrix[0:50][gang], container.linear[0:1000][vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            #pragma acc loop vector
            for (j = 0; j < 50; j++) {
                container.matrix[i][j] += 9;
            }
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 1000; i++) {
            container.linear[i] += 10;
        }
    }
    checksum += compute_checksum(&container.matrix[0][0], 50*50);
    checksum += compute_checksum(container.linear, 1000);
    
    /* Nested constructs for requirement #4 */
    #pragma acc enter data copyin(md_arr3[0:100][gang])
    #pragma acc parallel loop gang present(md_arr3[0:100][gang])
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr3[i][j] += 11;
        }
    }
    #pragma acc exit data copyout(md_arr3[0:100][gang])
    checksum += compute_checksum(&md_arr3[0][0], 100*100);
    
    /* Conditional partition selection for requirement #5 */
    for (int iter = 0; iter < 3; iter++) {
        if (use_gang) {
            #pragma acc data copy(md_arr1[0:50][gang])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 50; i++) {
                    for (j = 0; j < 100; j++) {
                        md_arr1[i][j] += 12;
                    }
                }
            }
        }
        
        if (use_worker) {
            #pragma acc data copy(md_arr2[0:100][worker])
            {
                #pragma acc parallel loop worker
                for (i = 0; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        md_arr2[i][j] += 13;
                    }
                }
            }
        }
    }
    
    /* Final checksum computation */
    checksum += compute_checksum(&md_arr1[0][0], 100*100);
    checksum += compute_checksum(&md_arr2[0][0], 100*100);
    checksum += compute_checksum(dyn_arr, N);
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    free(dyn_arr);
    return 0;
}
