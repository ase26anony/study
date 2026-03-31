#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Partition type mapping for conditional selection */
static const char* partition_keywords[] = {
    "", /* gang redundant */
    "[gang]",
    "[worker]",
    "[gang+worker]",
    "[vector]",
    "[gang+vector]",
    "[worker+vector]",
    "[gang+worker+vector]"
};

/* Struct with array members for requirement #6 */
struct DataContainer {
    int matrix[50][50];
    int linear[1000];
    double values[200];
};

/* Volatile variables to prevent optimization and enable conditional paths */
volatile int select_partition = 0;
volatile int use_dynamic = 1;
volatile int use_struct = 1;

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
    
    /* Requirement #3: Dynamic data */
    int *dyn_arr = (int *)malloc(1000 * sizeof(int));
    for (i = 0; i < 1000; i++) {
        dyn_arr[i] = i % 100;
    }
    
    /* Requirement #6: Struct with array members */
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
    
    /* Case 0: Gang redundant (default mapping) */
    #pragma acc parallel loop gang copy(md_arr1[0:100][0:100])
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr1[i][j] += 1;
        }
    }
    
    /* Case 1: Gang partitioned */
    #pragma acc parallel loop gang copy(md_arr1[0:100][0:100][gang])
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr1[i][j] += 2;
        }
    }
    
    /* Case 2: Worker partitioned */
    #pragma acc parallel loop gang worker copy(md_arr2[0:100][0:100][worker])
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr2[i][j] += 3;
        }
    }
    
    /* Case 3: Gang+Worker partitioned - using multi-dimensional array section */
    #pragma acc data copy(md_arr3[0:50][0:50][gang+worker])
    {
        #pragma acc parallel loop gang worker
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                md_arr3[i][j] += 4;
            }
        }
    }
    
    /* Case 4: Vector partitioned - using dynamic array */
    if (use_dynamic) {
        #pragma acc parallel loop vector copy(dyn_arr[0:1000][vector])
        for (i = 0; i < 1000; i++) {
            dyn_arr[i] += 5;
        }
    }
    
    /* Case 5: Gang+Vector partitioned */
    #pragma acc parallel loop gang vector copy(md_arr1[0:100][0:100][gang+vector])
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr1[i][j] += 6;
        }
    }
    
    /* Case 6: Worker+Vector partitioned */
    #pragma acc parallel loop gang worker vector copy(md_arr2[0:100][0:100][worker+vector])
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr2[i][j] += 7;
        }
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) - 3D conceptual partitioning */
    #pragma acc data copy(md_arr3[0:100][0:100][gang+worker+vector])
    {
        #pragma acc parallel loop gang worker vector
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                md_arr3[i][j] += 8;
            }
        }
    }
    
    /* Requirement #4: Nested and combined constructs with enter/exit data */
    int *base_arr = (int *)malloc(500 * sizeof(int));
    for (i = 0; i < 500; i++) base_arr[i] = i;
    
    #pragma acc enter data copyin(base_arr[0:500][gang])
    #pragma acc parallel loop gang present(base_arr[0:500][gang])
    for (i = 0; i < 500; i++) {
        base_arr[i] *= 2;
    }
    #pragma acc exit data copyout(base_arr[0:500])
    
    /* Requirement #5: Conditional partition selection */
    for (int ptype = 0; ptype < 8; ++ptype) {
        if (select_partition == ptype) {
            switch (ptype) {
                case 0:
                    #pragma acc parallel loop gang copy(md_arr1[0:10][0:10])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
                case 1:
                    #pragma acc parallel loop gang copy(md_arr1[0:10][0:10][gang])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
                case 2:
                    #pragma acc parallel loop gang worker copy(md_arr1[0:10][0:10][worker])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
                case 3:
                    #pragma acc parallel loop gang worker copy(md_arr1[0:10][0:10][gang+worker])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
                case 4:
                    #pragma acc parallel loop vector copy(md_arr1[0:10][0:10][vector])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
                case 5:
                    #pragma acc parallel loop gang vector copy(md_arr1[0:10][0:10][gang+vector])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
                case 6:
                    #pragma acc parallel loop gang worker vector copy(md_arr1[0:10][0:10][worker+vector])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
                case 7:
                    #pragma acc parallel loop gang worker vector copy(md_arr1[0:10][0:10][gang+worker+vector])
                    for (i = 0; i < 10; i++) {
                        for (j = 0; j < 10; j++) {
                            md_arr1[i][j] += ptype;
                        }
                    }
                    break;
            }
        }
    }
    
    /* Struct with partitioned array members */
    if (use_struct) {
        #pragma acc data copy(container.matrix[0:50][0:50][gang], container.linear[0:1000][vector])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 50; j++) {
                    container.matrix[i][j] += 10;
                }
            }
            
            #pragma acc parallel loop vector
            for (i = 0; i < 1000; i++) {
                container.linear[i] += 20;
            }
        }
    }
    
    /* Compute checksum for observable side effect */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += md_arr1[i][j] + md_arr2[i][j] + md_arr3[i][j];
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += dyn_arr[i];
    }
    
    for (i = 0; i < 500; i++) {
        checksum += base_arr[i];
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            checksum += container.matrix[i][j];
        }
    }
    
    for (i = 0; i < 1000; i++) {
        checksum += container.linear[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr);
    free(base_arr);
    
    return 0;
}
