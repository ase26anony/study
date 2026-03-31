#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Partition type mapping */
typedef enum {
    GANG_REDUNDANT = 0,
    GANG_PARTITIONED,
    WORKER_PARTITIONED,
    GANG_WORKER_PARTITIONED,
    VECTOR_PARTITIONED,
    GANG_VECTOR_PARTITIONED,
    WORKER_VECTOR_PARTITIONED,
    FULLY_PARTITIONED
} partition_type_t;

/* Struct with array members */
struct data_container {
    int arr1[100][100];
    int arr2[200][50];
    double arr3[50][50][50];
};

int main() {
    int i, j, k;
    long long checksum = 0;
    
    /* Multi-dimensional arrays */
    int md_arr1[100][100];
    int md_arr2[200][50];
    double md_arr3[50][50][50];
    
    /* Dynamic arrays */
    int *dyn_arr1 = (int*)malloc(1000 * sizeof(int));
    int *dyn_arr2 = (int*)malloc(500 * sizeof(int));
    
    /* Struct instance */
    struct data_container s;
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr1[i][j] = i + j;
            s.arr1[i][j] = i * j;
        }
    }
    
    for (i = 0; i < 200; i++) {
        for (j = 0; j < 50; j++) {
            md_arr2[i][j] = i - j;
            s.arr2[i][j] = i + 2*j;
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 50; k++) {
                md_arr3[i][j][k] = i * j * k * 0.5;
                s.arr3[i][j][k] = i + j + k;
            }
        }
    }
    
    for (i = 0; i < 1000; i++) dyn_arr1[i] = i % 100;
    for (i = 0; i < 500; i++) dyn_arr2[i] = i % 50;
    
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
    }
    
    /* Case 1: Gang partitioned */
    if (use_gang) {
        #pragma acc data copy(md_arr1[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    md_arr1[i][j] += 2;
                }
            }
        }
    }
    
    /* Case 2: Worker partitioned */
    if (use_worker) {
        #pragma acc data copy(md_arr2[0:200][worker])
        {
            #pragma acc parallel loop worker
            for (i = 0; i < 200; i++) {
                for (j = 0; j < 50; j++) {
                    md_arr2[i][j] += 3;
                }
            }
        }
    }
    
    /* Case 3: Gang+worker partitioned */
    if (use_combined) {
        #pragma acc data copy(md_arr1[0:50][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 100; j++) {
                    md_arr1[i][j] += 4;
                }
            }
        }
    }
    
    /* Case 4: Vector partitioned */
    if (use_vector) {
        #pragma acc data copy(dyn_arr1[0:1000][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < 1000; i++) {
                dyn_arr1[i] += 5;
            }
        }
    }
    
    /* Case 5: Gang+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(dyn_arr2[0:500][gang+vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < 500; i++) {
                dyn_arr2[i] += 6;
            }
        }
    }
    
    /* Case 6: Worker+vector partitioned */
    if (use_combined) {
        #pragma acc data copy(md_arr2[0:100][worker+vector])
        {
            #pragma acc parallel loop worker vector
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 50; j++) {
                    md_arr2[i][j] += 7;
                }
            }
        }
    }
    
    /* Case 7: Fully partitioned (gang+worker+vector) */
    if (use_combined) {
        #pragma acc data copy(md_arr3[0:25][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(3)
            for (i = 0; i < 25; i++) {
                for (j = 0; j < 25; j++) {
                    for (k = 0; k < 25; k++) {
                        md_arr3[i][j][k] += 8.0;
                    }
                }
            }
        }
    }
    
    /* Struct with partitioned array members */
    #pragma acc data copy(s.arr1[gang], s.arr2[vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 100; i++) {
            for (j = 0; j < 100; j++) {
                s.arr1[i][j] += 9;
            }
        }
        
        #pragma acc parallel loop vector
        for (i = 0; i < 200; i++) {
            for (j = 0; j < 50; j++) {
                s.arr2[i][j] += 10;
            }
        }
    }
    
    /* Nested constructs with enter/exit data */
    #pragma acc enter data copyin(md_arr1[0:50][gang])
    #pragma acc parallel loop gang present(md_arr1[gang])
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 100; j++) {
            md_arr1[i][j] += 11;
        }
    }
    #pragma acc exit data copyout(md_arr1[0:50][gang])
    
    /* Conditional partition selection using volatile */
    volatile int ptype;
    for (ptype = 0; ptype < 8; ++ptype) {
        if (ptype % 2 == 0) {  /* Use volatile condition */
            switch (ptype) {
                case 0:
                    #pragma acc data copy(md_arr1[gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                md_arr1[i][j] += 1;
                            }
                        }
                    }
                    break;
                case 1:
                    #pragma acc data copy(md_arr1[0:20][gang])
                    {
                        #pragma acc parallel loop gang
                        for (i = 0; i < 20; i++) {
                            for (j = 0; j < 20; j++) {
                                md_arr1[i][j] += 2;
                            }
                        }
                    }
                    break;
                case 2:
                    #pragma acc data copy(md_arr2[0:30][worker])
                    {
                        #pragma acc parallel loop worker
                        for (i = 0; i < 30; i++) {
                            for (j = 0; j < 30; j++) {
                                md_arr2[i][j] += 3;
                            }
                        }
                    }
                    break;
                case 3:
                    #pragma acc data copy(md_arr1[0:15][gang][worker])
                    {
                        #pragma acc parallel loop gang worker
                        for (i = 0; i < 15; i++) {
                            for (j = 0; j < 15; j++) {
                                md_arr1[i][j] += 4;
                            }
                        }
                    }
                    break;
                case 4:
                    #pragma acc data copy(dyn_arr1[0:100][vector])
                    {
                        #pragma acc parallel loop vector
                        for (i = 0; i < 100; i++) {
                            dyn_arr1[i] += 5;
                        }
                    }
                    break;
                case 5:
                    #pragma acc data copy(dyn_arr2[0:50][gang+vector])
                    {
                        #pragma acc parallel loop gang vector
                        for (i = 0; i < 50; i++) {
                            dyn_arr2[i] += 6;
                        }
                    }
                    break;
                case 6:
                    #pragma acc data copy(md_arr2[0:25][worker+vector])
                    {
                        #pragma acc parallel loop worker vector
                        for (i = 0; i < 25; i++) {
                            for (j = 0; j < 25; j++) {
                                md_arr2[i][j] += 7;
                            }
                        }
                    }
                    break;
                case 7:
                    #pragma acc data copy(md_arr3[0:10][gang][worker][vector])
                    {
                        #pragma acc parallel loop gang worker vector collapse(3)
                        for (i = 0; i < 10; i++) {
                            for (j = 0; j < 10; j++) {
                                for (k = 0; k < 10; k++) {
                                    md_arr3[i][j][k] += 8.0;
                                }
                            }
                        }
                    }
                    break;
            }
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            checksum += md_arr1[i][j];
            checksum += s.arr1[i][j];
        }
    }
    
    for (i = 0; i < 200; i++) {
        for (j = 0; j < 50; j++) {
            checksum += md_arr2[i][j];
            checksum += s.arr2[i][j];
        }
    }
    
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            for (k = 0; k < 50; k++) {
                checksum += (long long)md_arr3[i][j][k];
                checksum += (long long)s.arr3[i][j][k];
            }
        }
    }
    
    for (i = 0; i < 1000; i++) checksum += dyn_arr1[i];
    for (i = 0; i < 500; i++) checksum += dyn_arr2[i];
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(dyn_arr1);
    free(dyn_arr2);
    
    return 0;
}
