#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent dead code elimination */
volatile int v_cond = 1;
volatile int partition_type = 0;

/* Struct with array members for testing nested components */
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
    int total_checksum = 0;
    
    /* Multi-dimensional arrays for partition testing */
    int arr3d[20][30][40];
    int arr2d[100][100];
    int arr1d[1000];
    
    /* Initialize arrays */
    #pragma acc parallel loop collapse(3) gang worker vector
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            for (int k = 0; k < 40; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    #pragma acc parallel loop collapse(2) gang worker
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr2d[i][j] = i * j;
        }
    }
    
    #pragma acc parallel loop vector
    for (int i = 0; i < 1000; i++) {
        arr1d[i] = i;
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    
    /* Dynamic memory allocation */
    int *dyn_arr = (int *)malloc(500 * sizeof(int));
    for (int i = 0; i < 500; i++) {
        dyn_arr[i] = i * 2;
    }
    
    /* =========================================== */
    /* Test Case 0: Gang Redundant (default) */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(arr1d[0:1000])  /* gang redundant */
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 1000; i++) {
                arr1d[i] += 1;
            }
        }
        total_checksum += compute_checksum(arr1d, 1000);
    }
    
    /* =========================================== */
    /* Test Case 1: Gang Partitioned */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(arr2d[0:100][gang])  /* gang partitioned */
        {
            #pragma acc parallel loop gang collapse(2)
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += i;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    }
    
    /* =========================================== */
    /* Test Case 2: Worker Partitioned */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(arr2d[0:100][worker])  /* worker partitioned */
        {
            #pragma acc parallel loop worker collapse(2)
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    arr2d[i][j] += j;
                }
            }
        }
        total_checksum += compute_checksum(&arr2d[0][0], 100 * 100);
    }
    
    /* =========================================== */
    /* Test Case 3: Gang+Worker Partitioned */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(arr3d[0:20][gang][worker])  /* gang+worker partitioned */
        {
            #pragma acc parallel loop gang worker collapse(2)
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 30; j++) {
                    for (int k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 1;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 20 * 30 * 40);
    }
    
    /* =========================================== */
    /* Test Case 4: Vector Partitioned */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(dyn_arr[0:500][vector])  /* vector partitioned */
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 500; i++) {
                dyn_arr[i] += 3;
            }
        }
        total_checksum += compute_checksum(dyn_arr, 500);
    }
    
    /* =========================================== */
    /* Test Case 5: Gang+Vector Partitioned */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(arr3d[0:20][gang][vector])  /* gang+vector partitioned */
        {
            #pragma acc parallel loop gang vector collapse(2)
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 30; j++) {
                    for (int k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 2;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 20 * 30 * 40);
    }
    
    /* =========================================== */
    /* Test Case 6: Worker+Vector Partitioned */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(arr3d[0:20][worker][vector])  /* worker+vector partitioned */
        {
            #pragma acc parallel loop worker vector collapse(2)
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 30; j++) {
                    for (int k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 3;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 20 * 30 * 40);
    }
    
    /* =========================================== */
    /* Test Case 7: Fully Partitioned */
    /* =========================================== */
    if (v_cond) {
        #pragma acc data copy(arr3d[0:20][gang][worker][vector])  /* fully partitioned */
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 30; j++) {
                    for (int k = 0; k < 40; k++) {
                        arr3d[i][j][k] += 4;
                    }
                }
            }
        }
        total_checksum += compute_checksum(&arr3d[0][0][0], 20 * 30 * 40);
    }
    
    /* =========================================== */
    /* Test with struct array members */
    /* =========================================== */
    if (v_cond) {
        /* Mix different partition types in struct members */
        #pragma acc data copy(container.matrix[0:50][gang], container.vector[0:1000][vector])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    container.matrix[i][j] = i * j;
                }
            }
            
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                container.vector[i] = i;
            }
        }
        total_checksum += compute_checksum(container.vector, 1000);
    }
    
    /* =========================================== */
    /* Test with enter/exit data directives */
    /* =========================================== */
    if (v_cond) {
        int *device_arr = (int *)malloc(200 * sizeof(int));
        for (int i = 0; i < 200; i++) {
            device_arr[i] = i * 10;
        }
        
        /* Enter data with gang partition */
        #pragma acc enter data copyin(device_arr[0:200][gang])
        
        /* Use the data in a parallel region */
        #pragma acc parallel loop gang present(device_arr[0:200][gang])
        for (int i = 0; i < 200; i++) {
            device_arr[i] += 5;
        }
        
        /* Exit data */
        #pragma acc exit data copyout(device_arr[0:200])
        
        total_checksum += compute_checksum(device_arr, 200);
        free(device_arr);
    }
    
    /* =========================================== */
    /* Conditional partition selection using loop */
    /* =========================================== */
    if (v_cond) {
        int test_arr[8][100];
        
        /* Initialize test array */
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 100; j++) {
                test_arr[i][j] = j;
            }
        }
        
        /* Try to trigger different partition types based on volatile condition */
        for (int ptype = 0; ptype < 8; ptype++) {
            partition_type = ptype;  /* Volatile to prevent optimization */
            
            if (partition_type == 0) {
                #pragma acc data copy(test_arr[0][0:100])  /* gang redundant */
                {
                    #pragma acc parallel loop
                    for (int j = 0; j < 100; j++) {
                        test_arr[0][j] += 1;
                    }
                }
            } else if (partition_type == 1) {
                #pragma acc data copy(test_arr[1][0:100][gang])  /* gang partitioned */
                {
                    #pragma acc parallel loop gang
                    for (int j = 0; j < 100; j++) {
                        test_arr[1][j] += 2;
                    }
                }
            } else if (partition_type == 2) {
                #pragma acc data copy(test_arr[2][0:100][worker])  /* worker partitioned */
                {
                    #pragma acc parallel loop worker
                    for (int j = 0; j < 100; j++) {
                        test_arr[2][j] += 3;
                    }
                }
            } else if (partition_type == 3) {
                #pragma acc data copy(test_arr[3][0:100][gang][worker])  /* gang+worker partitioned */
                {
                    #pragma acc parallel loop gang worker
                    for (int j = 0; j < 100; j++) {
                        test_arr[3][j] += 4;
                    }
                }
            } else if (partition_type == 4) {
                #pragma acc data copy(test_arr[4][0:100][vector])  /* vector partitioned */
                {
                    #pragma acc parallel loop vector
                    for (int j = 0; j < 100; j++) {
                        test_arr[4][j] += 5;
                    }
                }
            } else if (partition_type == 5) {
                #pragma acc data copy(test_arr[5][0:100][gang][vector])  /* gang+vector partitioned */
                {
                    #pragma acc parallel loop gang vector
                    for (int j = 0; j < 100; j++) {
                        test_arr[5][j] += 6;
                    }
                }
            } else if (partition_type == 6) {
                #pragma acc data copy(test_arr[6][0:100][worker][vector])  /* worker+vector partitioned */
                {
                    #pragma acc parallel loop worker vector
                    for (int j = 0; j < 100; j++) {
                        test_arr[6][j] += 7;
                    }
                }
            } else if (partition_type == 7) {
                #pragma acc data copy(test_arr[7][0:100][gang][worker][vector])  /* fully partitioned */
                {
                    #pragma acc parallel loop gang worker vector
                    for (int j = 0; j < 100; j++) {
                        test_arr[7][j] += 8;
                    }
                }
            }
        }
        
        /* Compute checksum for all test arrays */
        for (int i = 0; i < 8; i++) {
            total_checksum += compute_checksum(test_arr[i], 100);
        }
    }
    
    /* =========================================== */
    /* Test with kernels construct */
    /* =========================================== */
    if (v_cond) {
        double kernel_arr[50][50];
        
        #pragma acc data copy(kernel_arr[0:50][gang][worker])
        {
            #pragma acc kernels
            {
                #pragma acc loop gang
                for (int i = 0; i < 50; i++) {
                    #pragma acc loop worker
                    for (int j = 0; j < 50; j++) {
                        kernel_arr[i][j] = i * 1.0 + j * 0.1;
                    }
                }
            }
        }
    }
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(dyn_arr);
    
    return 0;
}
