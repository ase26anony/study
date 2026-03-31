/* This program is designed to trigger the OpenACC neutering/broadcast
   analysis logic for different partitioning scenarios, specifically
   targeting the switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 that maps integer codes to partitioning type strings. */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITER 8

/* Function prototypes for different OpenACC partitioning scenarios */
void gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0; /* volatile to prevent optimization */
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 3 + j * 7) % 100;
            dst[i][j] = 0;
        }
    }
    
    /* Read partitioning scenario selector */
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("Enter partitioning scenario (0-7): ");
        scanf("%d", &n);
    }
    
    /* Runtime-dependent loop bounds for inner loops */
    int inner_bound = (n % 4) * 32 + 64; /* Varies from 64 to 160 */
    
    /* Call different functions based on input to trigger various cases */
    switch (n % 8) {
        case 0:
            gang_redundant(inner_bound, src, dst);
            break;
        case 1:
            gang_partitioned(inner_bound, src, dst);
            break;
        case 2:
            worker_partitioned(inner_bound, src, dst);
            break;
        case 3:
            gang_worker_partitioned(inner_bound, src, dst);
            break;
        case 4:
            vector_partitioned(inner_bound, src, dst);
            break;
        case 5:
            gang_vector_partitioned(inner_bound, src, dst);
            break;
        case 6:
            worker_vector_partitioned(inner_bound, src, dst);
            break;
        case 7:
            fully_partitioned(inner_bound, src, dst);
            break;
        default:
            printf("Invalid scenario\n");
            return 1;
    }
    
    /* Compute checksum to ensure computation isn't optimized away */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Case 0: gang redundant - only 1 gang */
void gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                /* Runtime-dependent inner loop bound */
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * k;
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Case 1: gang partitioned - multiple gangs, workers=1, vector=1 */
void gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 1);
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Case 2: worker partitioned - 1 gang, multiple workers */
void worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 2);
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Case 3: gang+worker partitioned - multiple gangs and workers */
void gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 3);
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Case 4: vector partitioned - 1 gang, 1 worker, multiple vectors */
void vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                #pragma acc loop vector
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 4);
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Case 5: gang+vector partitioned - multiple gangs and vectors */
void gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                #pragma acc loop vector
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 5);
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Case 6: worker+vector partitioned - 1 gang, multiple workers and vectors */
void worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                #pragma acc loop vector
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 6);
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Case 7: fully partitioned - multiple gangs, workers, and vectors */
void fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                #pragma acc loop vector
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 7);
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Conditional compilation variants for different partitioning behaviors */
#ifdef VARIANT_A
/* Alternative implementation using kernels construct */
void variant_kernels(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc kernels num_gangs(2) num_workers(4) vector_length(16) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                #pragma acc loop vector
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 8);
                }
                dst[i][j] = temp;
            }
        }
    }
}
#endif

#ifdef VARIANT_B
/* Alternative with collapsed loops */
void variant_collapsed(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang collapse(2)
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                int temp = 0;
                #pragma acc loop vector
                for (k = 0; k < n; k++) {
                    temp += src[i][j] * (k + 9);
                }
                dst[i][j] = temp;
            }
        }
    }
}
#endif
