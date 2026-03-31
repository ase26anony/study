/* This program is designed to trigger the partitioning classification logic
   in GCC's OpenACC neutering/broadcast analysis, specifically targeting the
   switch statement that maps integer codes to descriptive strings for
   different gang/worker/vector partitioning scenarios. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define TILE 32

/* Function prototypes for different partitioning configurations */
void gang_redundant_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void fully_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0;
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    /* Read partitioning mode from input or command line */
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("Enter partitioning mode (0-7): ");
        scanf("%d", &n);
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Select partitioning configuration based on input */
    switch (n % 8) {
        case 0:
            gang_redundant_compute(n, src, dst);
            break;
        case 1:
            gang_partitioned_compute(n, src, dst);
            break;
        case 2:
            worker_partitioned_compute(n, src, dst);
            break;
        case 3:
            gang_worker_partitioned_compute(n, src, dst);
            break;
        case 4:
            vector_partitioned_compute(n, src, dst);
            break;
        case 5:
            gang_vector_partitioned_compute(n, src, dst);
            break;
        case 6:
            worker_vector_partitioned_compute(n, src, dst);
            break;
        case 7:
            fully_partitioned_compute(n, src, dst);
            break;
    }
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Function 1: Gang redundant (num_gangs=1, no workers/vectors specified) */
void gang_redundant_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % TILE) + TILE;  /* Runtime-dependent bound */
    
    #pragma acc parallel num_gangs(1) copyin(src) copyout(dst)
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned (only gangs specified) */
void gang_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % 64) + 64;  /* Runtime-dependent bound */
    
    #pragma acc kernels num_gangs(8) copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop seq
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                dst[i][j] = src[i][j] + i - j;
            }
        }
    }
}

/* Function 3: Worker partitioned (only workers specified) */
void worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % 128) + 128;  /* Runtime-dependent bound */
    
    #pragma acc parallel num_workers(4) copyin(src) copy(dst)
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop seq
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void gang_worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % 256) + 256;  /* Runtime-dependent bound */
    
    #pragma acc parallel num_gangs(4) num_workers(2) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                dst[i][j] = src[i][j] / (j + 1);
            }
        }
    }
}

/* Function 5: Vector partitioned (only vector specified) */
void vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % 512) + 512;  /* Runtime-dependent bound */
    
    #pragma acc kernels vector_length(32) copy(src, dst)
    {
        #pragma acc loop seq
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                dst[i][j] = src[i][j] | 0x1;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void gang_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % 384) + 384;  /* Runtime-dependent bound */
    
    #pragma acc parallel num_gangs(8) vector_length(64) copyin(src) copy(dst)
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                dst[i][j] = src[i][j] & 0xFF;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void worker_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % 192) + 192;  /* Runtime-dependent bound */
    
    #pragma acc parallel num_workers(8) vector_length(128) copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                dst[i][j] = src[i][j] ^ 0xAA;
            }
        }
    }
}

/* Function 8: Fully partitioned (gang+worker+vector) */
void fully_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    int loop_bound = (n % 1024);  /* Runtime-dependent bound */
    if (loop_bound < 256) loop_bound = 256;
    
    /* Use conditional compilation to create variant structures */
    #ifdef VARIANT_A
    #pragma acc parallel num_gangs(16) num_workers(4) vector_length(32) \
        copyin(src) copyout(dst)
    #else
    #pragma acc parallel num_gangs(8) num_workers(8) vector_length(16) \
        copy(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    #endif
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < loop_bound; j++) {  /* Runtime-dependent */
                #ifdef VARIANT_A
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    dst[i][j] += src[i][j] * k;
                }
                #else
                dst[i][j] = src[i][j] * 3 - src[i][j] / 2;
                #endif
            }
        }
    }
}
