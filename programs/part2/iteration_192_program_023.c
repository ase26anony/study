#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Force compiler to consider all branches */
volatile int select_partition = 0;

/* Struct with array members for complex mapping */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
    double values[200];
};

/* Partition type mapping */
const char* partition_keywords[] = {
    "",           /* 0: gang redundant */
    "[gang]",     /* 1: gang partitioned */
    "[worker]",   /* 2: worker partitioned */
    "[gang][worker]", /* 3: gang+worker partitioned */
    "[vector]",   /* 4: vector partitioned */
    "[gang][vector]", /* 5: gang+vector partitioned */
    "[worker][vector]", /* 6: worker+vector partitioned */
    "[gang][worker][vector]" /* 7: fully partitioned */
};

int main(int argc, char** argv) {
    int i, j, k;
    int checksum = 0;
    
    /* Multi-dimensional arrays for different partition types */
    int arr3d[10][20][30];
    int arr2d[100][100];
    int arr1d[1000];
    
    /* Dynamic arrays */
    int* dyn_arr = (int*)malloc(500 * sizeof(int));
    double* dyn_matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    /* Struct instance */
    struct DataContainer container;
    
    /* Initialize all arrays */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 20; j++) {
            for (k = 0; k < 30; k++) {
                arr3d[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            arr2d[i][j] = i * j;
        }
    }
    
    for (i = 0; i < 1000; i++) {
        arr1d[i] = i;
    }
    
    for (i = 0; i < 500; i++) {
        dyn_arr[i] = i * 2;
    }
    
    for (i = 0; i < 10000; i++) {
        dyn_matrix[i] = i * 0.5;
    }
    
    memset(&container, 0, sizeof(container));
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i - j;
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int use_all_cases = 1;
    
    /* Test each partition type explicitly */
    
    /* Case 0: gang redundant (default mapping) */
    if (use_all_cases) {
        #pragma acc data copy(arr1d[0:1000])
        {
            #pragma acc parallel loop
            for (i = 0; i < 1000; i++) {
                arr1d[i] += 1;
            }
        }
    }
    
    /* Case 1: gang partitioned */
    if (use_all_cases || select_partition == 1) {
        #pragma acc data copy(arr2d[0:50][0:50][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 50; j++) {
                    arr2d[i][j] *= 2;
                }
            }
        }
    }
    
    /* Case 2: worker partitioned */
    if (use_all_cases || select_partition == 2) {
        #pragma acc data copy(arr2d[50:50][0:100][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 50; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    arr2d[i][j] -= j;
                }
            }
        }
    }
    
    /* Case 3: gang+worker partitioned */
    if (use_all_cases || select_partition == 3) {
        #pragma acc data copy(arr3d[0:5][0:10][0:20][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 5; i++) {
                #pragma acc loop vector
                for (j = 0; j < 10; j++) {
                    for (k = 0; k < 20; k++) {
                        arr3d[i][j][k] += i * j * k;
                    }
                }
            }
        }
    }
    
    /* Case 4: vector partitioned */
    if (use_all_cases || select_partition == 4) {
        #pragma acc data copy(dyn_arr[0:500][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 500; i++) {
                dyn_arr[i] = dyn_arr[i] / 2;
            }
        }
    }
    
    /* Case 5: gang+vector partitioned */
    if (use_all_cases || select_partition == 5) {
        #pragma acc data copy(container.matrix[0:25][0:25][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 25; i++) {
                for (j = 0; j < 25; j++) {
                    container.matrix[i][j] += 1000;
                }
            }
        }
    }
    
    /* Case 6: worker+vector partitioned */
    if (use_all_cases || select_partition == 6) {
        #pragma acc data copy(container.vector[0:500][worker][vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 500; i++) {
                container.vector[i] = i * 3;
            }
        }
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    if (use_all_cases || select_partition == 7) {
        #pragma acc data copy(arr3d[5:5][10:10][20:10][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 5; i < 10; i++) {
                for (j = 10; j < 20; j++) {
                    for (k = 20; k < 30; k++) {
                        arr3d[i][j][k] = arr3d[i][j][k] * 2 + 1;
                    }
                }
            }
        }
    }
    
    /* Test with nested constructs */
    if (use_all_cases) {
        #pragma acc data copy(dyn_matrix[0:5000][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                #pragma acc loop worker
                for (j = 0; j < 100; j++) {
                    int idx = i * 100 + j;
                    #pragma acc loop vector
                    for (k = 0; k < 1; k++) {  /* Force vector loop */
                        dyn_matrix[idx] += 1.0;
                    }
                }
            }
            
            /* Nested region with different partition */
            #pragma acc data copy(dyn_matrix[5000:5000][vector])
            {
                #pragma acc parallel loop vector
                for (i = 5000; i < 10000; i++) {
                    dyn_matrix[i] *= 0.5;
                }
            }
        }
    }
    
    /* Test enter/exit data with partitions */
    if (use_all_cases) {
        int* temp_arr = (int*)malloc(200 * sizeof(int));
        for (i = 0; i < 200; i++) temp_arr[i] = i;
        
        #pragma acc enter data copyin(temp_arr[0:100][gang])
        #pragma acc enter data copyin(temp_arr[100:100][worker])
        
        #pragma acc parallel loop present(temp_arr[0:100][gang])
        for (i = 0; i < 100; i++) {
            temp_arr[i] += 10;
        }
        
        #pragma acc parallel loop present(temp_arr[100:100][worker])
        for (i = 100; i < 200; i++) {
            temp_arr[i] -= 5;
        }
        
        #pragma acc exit data copyout(temp_arr[0:100][gang])
        #pragma acc exit data copyout(temp_arr[100:100][worker])
        
        free(temp_arr);
    }
    
    /* Conditional partition selection in loop */
    for (int ptype = 0; ptype < 4; ++ptype) {
        if (select_partition == ptype || use_all_cases) {
            switch (ptype) {
                case 0:
                    #pragma acc data copy(arr1d[200:300])
                    {
                        #pragma acc parallel loop
                        for (i = 200; i < 500; i++) {
                            arr1d[i] += ptype;
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(arr2d[0:30][0:30][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 30; i++) {
                            for (j = 0; j < 30; j++) {
                                arr2d[i][j] += ptype;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(container.values[0:100][worker])
                    {
                        #pragma acc parallel loop worker
                        for (i = 0; i < 100; i++) {
                            container.values[i] = ptype;
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(arr3d[0:3][0:5][0:10][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 3; i++) {
                            for (j = 0; j < 5; j++) {
                                for (k = 0; k < 10; k++) {
                                    arr3d[i][j][k] += ptype;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < 1000; i++) {
        checksum += arr1d[i];
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    for (i = 0; i < 500; i++) {
        checksum += dyn_arr[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(dyn_matrix);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
