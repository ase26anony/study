#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int v_cond = 1;
volatile int v_type = 0;

/* Struct with array members */
struct DataContainer {
    int grid[100][100];
    int vector_data[1000];
    int matrix[50][50][50];
};

/* Helper to compute checksum */
int compute_checksum(int *data, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays */
    int arr2d[100][100];
    int arr3d[50][50][50];
    
    /* Dynamic arrays */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    int *dyn_matrix = (int*)malloc(100 * 100 * sizeof(int));
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize all arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i + j;
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 50; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
        if (i < 100 * 100) dyn_matrix[i] = i;
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            container.grid[i][j] = i * j;
        }
    }
    
    /* ===== CASE 0: gang redundant ===== */
    if (v_cond) {
        #pragma acc data copy(arr2d)
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 1;
                }
            }
        }
    }
    
    /* ===== CASE 1: gang partitioned ===== */
    if (v_cond) {
        #pragma acc data copy(arr2d[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] += 2;
                }
            }
        }
    }
    
    /* ===== CASE 2: worker partitioned ===== */
    if (v_cond) {
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
    }
    
    /* ===== CASE 3: gang+worker partitioned ===== */
    if (v_cond) {
        #pragma acc data copy(arr3d[0:50][gang][worker])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker
                for (j = 0; j < 50; j++) {
                    for (k = 0; k < 50; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
    }
    
    /* ===== CASE 4: vector partitioned ===== */
    if (v_cond) {
        #pragma acc data copy(dyn_arr[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 1000; i++) {
                dyn_arr[i] += 5;
            }
        }
    }
    
    /* ===== CASE 5: gang+vector partitioned ===== */
    if (v_cond) {
        #pragma acc data copy(container.grid[0:100][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    container.grid[i][j] += 6;
                }
            }
        }
    }
    
    /* ===== CASE 6: worker+vector partitioned ===== */
    if (v_cond) {
        #pragma acc data copy(dyn_matrix[0:10000][worker+vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    dyn_matrix[i * 100 + j] += 7;
                }
            }
        }
    }
    
    /* ===== CASE 7: fully partitioned (gang+worker+vector) ===== */
    if (v_cond) {
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
    }
    
    /* ===== Test with conditional partition selection ===== */
    /* This creates a single code region where compiler must handle multiple possibilities */
    for (int iter = 0; iter < 8; iter++) {
        v_type = iter;  /* Volatile to prevent optimization */
        
        if (v_type == 0) {
            #pragma acc data copy(arr2d)
            {
                #pragma acc parallel loop
                for (i = 0; i < 10; i++) {
                    for (j = 0; j < 10; j++) {
                        arr2d[i][j] += v_type;
                    }
                }
            }
        } else if (v_type == 1) {
            #pragma acc data copy(arr2d[0:10][gang])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 10; i++) {
                    arr2d[i][0] += v_type;
                }
            }
        } else if (v_type == 2) {
            #pragma acc data copy(arr2d[0:10][worker])
            {
                #pragma acc parallel loop worker
                for (i = 0; i < 10; i++) {
                    arr2d[i][1] += v_type;
                }
            }
        } else if (v_type == 3) {
            #pragma acc data copy(arr2d[0:10][gang][worker])
            {
                #pragma acc parallel loop gang worker
                for (i = 0; i < 10; i++) {
                    arr2d[i][2] += v_type;
                }
            }
        } else if (v_type == 4) {
            #pragma acc data copy(arr2d[0:10][vector])
            {
                #pragma acc parallel loop vector
                for (i = 0; i < 10; i++) {
                    arr2d[i][3] += v_type;
                }
            }
        } else if (v_type == 5) {
            #pragma acc data copy(arr2d[0:10][gang][vector])
            {
                #pragma acc parallel loop gang vector
                for (i = 0; i < 10; i++) {
                    arr2d[i][4] += v_type;
                }
            }
        } else if (v_type == 6) {
            #pragma acc data copy(arr2d[0:10][worker][vector])
            {
                #pragma acc parallel loop worker vector
                for (i = 0; i < 10; i++) {
                    arr2d[i][5] += v_type;
                }
            }
        } else if (v_type == 7) {
            #pragma acc data copy(arr2d[0:10][gang][worker][vector])
            {
                #pragma acc parallel loop gang worker vector
                for (i = 0; i < 10; i++) {
                    arr2d[i][6] += v_type;
                }
            }
        }
    }
    
    /* ===== Test with enter/exit data ===== */
    int *device_arr = (int*)malloc(100 * sizeof(int));
    for (i = 0; i < 100; i++) device_arr[i] = i;
    
    #pragma acc enter data copyin(device_arr[0:100][gang])
    #pragma acc parallel loop present(device_arr[0:100][gang])
    for (i = 0; i < 100; i++) {
        device_arr[i] *= 2;
    }
    #pragma acc exit data copyout(device_arr[0:100][gang])
    
    /* ===== Compute final checksums ===== */
    checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    checksum += compute_checksum(&arr3d[0][0][0], 50 * 50 * 50);
    checksum += compute_checksum(dyn_arr, 1000);
    checksum += compute_checksum(dyn_matrix, 100 * 100);
    checksum += compute_checksum(&container.grid[0][0], 100 * 100);
    checksum += compute_checksum(device_arr, 100);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    free(device_arr);
    
    return 0;
}
