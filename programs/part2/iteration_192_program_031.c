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
    int vector_data[1000];
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
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for requirement #2 */
    int arr3d[10][20][30];
    int arr2d[100][100];
    
    /* Initialize arrays */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 20; j++) {
            for (k = 0; k < 30; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i * 100 + j;
        }
    }
    
    /* Requirement #1: Trigger all partition types */
    
    /* Case 0: gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc parallel loop gang copy(arr2d[0:50][0:50])
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (use_gang) {
        #pragma acc parallel loop gang copy(arr2d[0:50][0:50][gang])
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 2;
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (use_worker) {
        #pragma acc parallel loop gang worker copy(arr2d[0:50][0:50][worker])
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 3;
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:5][0:10][0:15][gang+worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 5; i++) {
                for (j = 0; j < 10; j++) {
                    for (k = 0; k < 15; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (use_vector) {
        #pragma acc parallel loop vector copy(arr2d[0:50][0:50][vector])
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 5;
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (use_combined) {
        #pragma acc parallel loop gang vector copy(arr2d[0:50][0:50][gang+vector])
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 6;
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (use_combined) {
        #pragma acc parallel loop worker vector copy(arr2d[0:50][0:50][worker+vector])
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                arr2d[i][j] += 7;
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (use_combined) {
        #pragma acc data copy(arr3d[0:5][0:10][0:15][gang+worker+vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 5; i++) {
                for (j = 0; j < 10; j++) {
                    for (k = 0; k < 15; k++) {
                        arr3d[i][j][k] += 8;
                    }
                }
            }
        }
    }
    
    /* Requirement #3: Dynamic data with pointer-based mappings */
    int N = 1000;
    int *dyn_arr = (int *)malloc(N * sizeof(int));
    for (i = 0; i < N; i++) {
        dyn_arr[i] = i;
    }
    
    /* Test various partition types with dynamic data */
    #pragma acc parallel loop gang copy(dyn_arr[0:N][gang])
    for (i = 0; i < N; i++) {
        dyn_arr[i] += 9;
    }
    
    #pragma acc parallel loop worker copy(dyn_arr[0:N][worker])
    for (i = 0; i < N; i++) {
        dyn_arr[i] += 10;
    }
    
    #pragma acc parallel loop vector copy(dyn_arr[0:N][vector])
    for (i = 0; i < N; i++) {
        dyn_arr[i] += 11;
    }
    
    /* Requirement #4: Nested and combined constructs */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Enter data with partition clause */
    #pragma acc enter data copyin(container.matrix[0:25][0:25][gang])
    
    #pragma acc parallel loop gang present(container.matrix[0:25][0:25][gang])
    for (i = 0; i < 25; i++) {
        for (j = 0; j < 25; j++) {
            container.matrix[i][j] = i * j;
        }
    }
    
    #pragma acc exit data copyout(container.matrix[0:25][0:25][gang])
    
    /* Requirement #5: Conditional partition selection */
    /* Array mapping partition codes to keywords */
    const char* partition_keywords[] = {
        "",
        "[gang]",
        "[worker]",
        "[gang+worker]",
        "[vector]",
        "[gang+vector]",
        "[worker+vector]",
        "[gang+worker+vector]"
    };
    
    int test_arr[100];
    for (i = 0; i < 100; i++) {
        test_arr[i] = i;
    }
    
    /* Loop through partition types - compiler may analyze all possibilities */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (argc > 1) {  /* Use argc to make condition non-constant */
            switch (ptype) {
                case 0:
                    #pragma acc parallel loop gang copy(test_arr[0:100])
                    for (i = 0; i < 100; i++) test_arr[i] += 1;
                    break;
                case 1:
                    #pragma acc parallel loop gang copy(test_arr[0:100][gang])
                    for (i = 0; i < 100; i++) test_arr[i] += 2;
                    break;
                case 2:
                    #pragma acc parallel loop worker copy(test_arr[0:100][worker])
                    for (i = 0; i < 100; i++) test_arr[i] += 3;
                    break;
                case 3:
                    #pragma acc parallel loop gang worker copy(test_arr[0:100][gang+worker])
                    for (i = 0; i < 100; i++) test_arr[i] += 4;
                    break;
                case 4:
                    #pragma acc parallel loop vector copy(test_arr[0:100][vector])
                    for (i = 0; i < 100; i++) test_arr[i] += 5;
                    break;
                case 5:
                    #pragma acc parallel loop gang vector copy(test_arr[0:100][gang+vector])
                    for (i = 0; i < 100; i++) test_arr[i] += 6;
                    break;
                case 6:
                    #pragma acc parallel loop worker vector copy(test_arr[0:100][worker+vector])
                    for (i = 0; i < 100; i++) test_arr[i] += 7;
                    break;
                case 7:
                    #pragma acc parallel loop gang worker vector copy(test_arr[0:100][gang+worker+vector])
                    for (i = 0; i < 100; i++) test_arr[i] += 8;
                    break;
            }
        }
    }
    
    /* Requirement #6: Struct with array members and partition clauses */
    struct DataContainer *container_ptr = &container;
    
    #pragma acc data copy(container_ptr->matrix[0:30][0:30][gang], \
                          container_ptr->vector_data[0:500][vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 30; i++) {
            for (j = 0; j < 30; j++) {
                container_ptr->matrix[i][j] += i + j;
            }
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 500; i++) {
            container_ptr->vector_data[i] += i * 2;
        }
    }
    
    /* Compute final checksums */
    checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    checksum += compute_checksum(&arr3d[0][0][0], 10 * 20 * 30);
    checksum += compute_checksum(dyn_arr, N);
    checksum += compute_checksum(test_arr, 100);
    checksum += compute_checksum(container.matrix[0], 50 * 50);
    
    printf("Final checksum: %d\n", checksum);
    
    free(dyn_arr);
    return 0;
}
