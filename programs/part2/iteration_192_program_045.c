/* Test program to cover partition neutering switch cases in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Use volatile to prevent optimization and dead code elimination */
volatile int use_gang = 1;
volatile int use_worker = 1;
volatile int use_vector = 1;
volatile int use_combined = 1;

/* Struct with array members for complex mapping */
struct DataContainer {
    int matrix[50][50];
    int vector[1000];
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
    
    /* Multi-dimensional arrays for different partition types */
    int md_arr1[100][100];
    int md_arr2[100][100];
    int md_arr3[100][100];
    int md_arr4[100][100];
    
    /* Initialize arrays */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            md_arr1[i][j] = i + j;
            md_arr2[i][j] = i * j;
            md_arr3[i][j] = i - j;
            md_arr4[i][j] = i ^ j;
        }
    }
    
    /* Dynamic arrays */
    int N = 1000;
    int *dyn_arr1 = (int *)malloc(N * sizeof(int));
    int *dyn_arr2 = (int *)malloc(N * sizeof(int));
    for (i = 0; i < N; i++) {
        dyn_arr1[i] = i % 100;
        dyn_arr2[i] = i % 50;
    }
    
    /* Struct instance */
    struct DataContainer container;
    memset(&container, 0, sizeof(container));
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            container.matrix[i][j] = i * 50 + j;
        }
    }
    for (i = 0; i < 1000; i++) {
        container.vector[i] = i;
    }
    
    /* ===== CASE 0: gang redundant (default mapping) ===== */
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
    
    /* ===== CASE 1: gang partitioned ===== */
    if (use_gang) {
        #pragma acc data copy(md_arr1[0:100][gang])
        {
            #pragma acc parallel loop gang
            for (i = 0; i < 100; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < 100; j++) {
                    md_arr1[i][j] += 2;
                }
            }
        }
        checksum += compute_checksum(&md_arr1[0][0], 100*100);
    }
    
    /* ===== CASE 2: worker partitioned ===== */
    if (use_worker) {
        #pragma acc data copy(md_arr2[0:100][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 100; i++) {
                #pragma acc loop vector
                for (j = 0; j < 100; j++) {
                    md_arr2[i][j] += 3;
                }
            }
        }
        checksum += compute_checksum(&md_arr2[0][0], 100*100);
    }
    
    /* ===== CASE 3: gang+worker partitioned ===== */
    if (use_combined) {
        #pragma acc data copy(md_arr3[0:50][gang][worker])
        {
            #pragma acc parallel loop gang worker
            for (i = 0; i < 50; i++) {
                #pragma acc loop vector
                for (j = 0; j < 50; j++) {
                    md_arr3[i][j] += 4;
                }
            }
        }
        checksum += compute_checksum(&md_arr3[0][0], 100*100);
    }
    
    /* ===== CASE 4: vector partitioned ===== */
    if (use_vector) {
        #pragma acc data copy(dyn_arr1[0:N][vector])
        {
            #pragma acc parallel loop vector
            for (i = 0; i < N; i++) {
                dyn_arr1[i] += 5;
            }
        }
        checksum += compute_checksum(dyn_arr1, N);
    }
    
    /* ===== CASE 5: gang+vector partitioned ===== */
    if (use_combined) {
        #pragma acc data copy(dyn_arr2[0:N][gang][vector])
        {
            #pragma acc parallel loop gang vector
            for (i = 0; i < N; i++) {
                dyn_arr2[i] += 6;
            }
        }
        checksum += compute_checksum(dyn_arr2, N);
    }
    
    /* ===== CASE 6: worker+vector partitioned ===== */
    if (use_combined) {
        #pragma acc data copy(md_arr4[0:100][worker+vector])
        {
            #pragma acc parallel loop gang worker vector
            for (i = 0; i < 100; i++) {
                for (j = 0; j < 100; j++) {
                    md_arr4[i][j] += 7;
                }
            }
        }
        checksum += compute_checksum(&md_arr4[0][0], 100*100);
    }
    
    /* ===== CASE 7: fully partitioned (gang+worker+vector) ===== */
    if (use_combined) {
        /* Using struct member with full partitioning */
        #pragma acc data copy(container.matrix[0:50][gang][worker][vector])
        {
            #pragma acc parallel loop gang worker vector collapse(2)
            for (i = 0; i < 50; i++) {
                for (j = 0; j < 50; j++) {
                    container.matrix[i][j] += 8;
                }
            }
        }
        checksum += compute_checksum(&container.matrix[0][0], 50*50);
    }
    
    /* ===== Additional complex cases ===== */
    
    /* Nested constructs with partitioned data */
    #pragma acc enter data copyin(container.vector[0:1000][gang])
    
    #pragma acc parallel loop gang present(container.vector[gang])
    for (i = 0; i < 1000; i++) {
        container.vector[i] += 9;
    }
    
    #pragma acc exit data copyout(container.vector[0:1000][gang])
    checksum += compute_checksum(container.vector, 1000);
    
    /* Conditional partition selection */
    for (int iter = 0; iter < 3; iter++) {
        if (use_gang && (iter == 0)) {
            #pragma acc data copy(md_arr1[0:100][gang])
            {
                #pragma acc parallel loop gang
                for (i = 0; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        md_arr1[i][j] += 10;
                    }
                }
            }
        } else if (use_worker && (iter == 1)) {
            #pragma acc data copy(md_arr2[0:100][worker])
            {
                #pragma acc parallel loop worker
                for (i = 0; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        md_arr2[i][j] += 11;
                    }
                }
            }
        } else if (use_vector && (iter == 2)) {
            #pragma acc data copy(md_arr3[0:100][vector])
            {
                #pragma acc parallel loop vector
                for (i = 0; i < 100; i++) {
                    for (j = 0; j < 100; j++) {
                        md_arr3[i][j] += 12;
                    }
                }
            }
        }
    }
    
    /* Combined mapping with multiple partitions */
    #pragma acc data copy(md_arr1[0:50][gang], md_arr2[0:50][worker], md_arr3[0:50][vector])
    {
        #pragma acc parallel loop gang
        for (i = 0; i < 50; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < 50; j++) {
                md_arr1[i][j] += md_arr2[i][j] + md_arr3[i][j];
            }
        }
    }
    
    /* Final checksum computation */
    checksum += compute_checksum(&md_arr1[0][0], 100*100);
    checksum += compute_checksum(&md_arr2[0][0], 100*100);
    checksum += compute_checksum(&md_arr3[0][0], 100*100);
    checksum += compute_checksum(&md_arr4[0][0], 100*100);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(dyn_arr1);
    free(dyn_arr2);
    
    return 0;
}
