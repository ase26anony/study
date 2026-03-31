#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for requirement #6 */
struct DataStruct {
    int arr1[100][100];
    int arr2[200][50];
    double arr3[50][50][50];
};

/* Function to compute checksum */
int compute_checksum(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Multi-dimensional arrays for requirement #2 */
    int multi_arr[100][100];
    double deep_arr[20][20][20];
    
    /* Dynamic arrays for requirement #3 */
    int *dyn_arr = (int*)malloc(1000 * sizeof(int));
    double *dyn_2d = (double*)malloc(200 * 50 * sizeof(double));
    
    /* Struct instance for requirement #6 */
    struct DataStruct ds;
    
    /* Initialize all arrays */
    memset(multi_arr, 0, sizeof(multi_arr));
    memset(deep_arr, 0, sizeof(deep_arr));
    memset(dyn_arr, 0, 1000 * sizeof(int));
    memset(dyn_2d, 0, 200 * 50 * sizeof(double));
    memset(&ds, 0, sizeof(ds));
    
    int checksum = 0;
    
    /* Case 0: gang redundant (default mapping) */
    if (use_gang) {
        #pragma acc data copy(multi_arr)
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    multi_arr[i][j] += i + j;
                }
            }
        }
        checksum += compute_checksum(&multi_arr[0][0], 100*100);
    }
    
    /* Case 1: gang partitioned */
    if (use_gang) {
        #pragma acc data copy(multi_arr[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    multi_arr[i][j] += i * j;
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (use_worker) {
        #pragma acc data copy(dyn_arr[0:1000][worker])
        {
            #pragma acc parallel loop worker
            for (int i = 0; i < 1000; i++) {
                dyn_arr[i] += i % 17;
            }
        }
        checksum += compute_checksum(dyn_arr, 1000);
    }
    
    /* Case 3: gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(deep_arr[0:20][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (int i = 0; i < 20; i++) {
                for (int j = 0; j < 20; j++) {
                    for (int k = 0; k < 20; k++) {
                        deep_arr[i][j][k] += 1.0;
                    }
                }
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (int i = 0; i < 1000; i++) {
                dyn_arr[i] += i % 23;
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(ds.arr1[gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    ds.arr1[i][j] = i * 100 + j;
                }
            }
        }
        checksum += compute_checksum(&ds.arr1[0][0], 100*100);
    }
    
    /* Case 6: worker+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(ds.arr2[worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (int i = 0; i < 200; i++) {
                for (int j = 0; j < 50; j++) {
                    ds.arr2[i][j] = (i + j) % 256;
                }
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (use_gang && use_worker && use_vector) {
        #pragma acc data copy(ds.arr3[gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (int i = 0; i < 50; i++) {
                for (int j = 0; j < 50; j++) {
                    for (int k = 0; k < 50; k++) {
                        ds.arr3[i][j][k] = (i + j + k) * 0.5;
                    }
                }
            }
        }
    }
    
    /* Nested constructs for requirement #4 */
    #pragma acc data copy(dyn_2d[0:200*50][gang])
    {
        #pragma acc parallel loop gang
        for (int i = 0; i < 200; i++) {
            #pragma acc loop worker
            for (int j = 0; j < 50; j++) {
                dyn_2d[i*50 + j] = i * 0.1 + j * 0.01;
            }
        }
        
        /* Enter/exit data with partition clauses */
        #pragma acc enter data copyin(dyn_2d[0:100*50][worker])
        #pragma acc parallel loop present(dyn_2d[0:100*50][worker]) worker
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 50; j++) {
                dyn_2d[i*50 + j] += 1.0;
            }
        }
        #pragma acc exit data copyout(dyn_2d[0:100*50][worker])
    }
    
    /* Conditional partition selection for requirement #5 */
    const char* partition_keywords[] = {
        "gang",
        "worker", 
        "vector",
        "gang worker",
        "gang vector",
        "worker vector",
        "gang worker vector"
    };
    
    int test_arr[100];
    memset(test_arr, 0, sizeof(test_arr));
    
    /* Loop through different partition types */
    for (int iter = 0; iter < 3; iter++) {
        volatile int selector = iter % 7;
        
        if (selector == 0) {
            #pragma acc data copy(test_arr[0:100][gang])
            {
                #pragma acc parallel loop gang
                for (int i = 0; i < 100; i++) {
                    test_arr[i] += 1;
                }
            }
        } else if (selector == 1) {
            #pragma acc data copy(test_arr[0:100][worker])
            {
                #pragma acc parallel loop worker
                for (int i = 0; i < 100; i++) {
                    test_arr[i] += 2;
                }
            }
        } else if (selector == 2) {
            #pragma acc data copy(test_arr[0:100][vector])
            {
                #pragma acc parallel loop vector
                for (int i = 0; i < 100; i++) {
                    test_arr[i] += 3;
                }
            }
        } else if (selector == 3) {
            #pragma acc data copy(test_arr[0:100][gang][worker])
            {
                #pragma acc parallel loop gang worker
                for (int i = 0; i < 100; i++) {
                    test_arr[i] += 4;
                }
            }
        } else if (selector == 4) {
            #pragma acc data copy(test_arr[0:100][gang][vector])
            {
                #pragma acc parallel loop gang vector
                for (int i = 0; i < 100; i++) {
                    test_arr[i] += 5;
                }
            }
        } else if (selector == 5) {
            #pragma acc data copy(test_arr[0:100][worker][vector])
            {
                #pragma acc parallel loop worker vector
                for (int i = 0; i < 100; i++) {
                    test_arr[i] += 6;
                }
            }
        } else if (selector == 6) {
            #pragma acc data copy(test_arr[0:100][gang][worker][vector])
            {
                #pragma acc parallel loop gang worker vector
                for (int i = 0; i < 100; i++) {
                    test_arr[i] += 7;
                }
            }
        }
    }
    
    checksum += compute_checksum(test_arr, 100);
    
    /* Final checksum to prevent optimization */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_2d);
    
    return 0;
}
