#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += BLOCK) {
            int i_end = i + BLOCK < SIZE ? i + BLOCK : SIZE;
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                if (j % n == 0) {  /* Runtime-dependent condition */
                    dst[i][j] = src[i][j] * 2;
                } else {
                    dst[i][j] = src[i][j] + 1;
                }
            }
        }
    }
}

/* Function 2: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += BLOCK) {
            int i_end = i + BLOCK < SIZE ? i + BLOCK : SIZE;
            #pragma acc loop worker
            for (j = 0; j < SIZE; j += n) {  /* Runtime-dependent stride */
                int j_end = j + n < SIZE ? j + n : SIZE;
                #pragma acc loop vector
                for (int k = j; k < j_end; k++) {
                    dst[i][k] = src[i][k] * 3 - src[i][k] / 2;
                }
            }
        }
    }
}

/* Function 3: Fully partitioned */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(16) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += n) {  /* Runtime-dependent chunk size */
            int i_end = i + n < SIZE ? i + n : SIZE;
            #pragma acc loop worker
            for (j = 0; j < SIZE; j += BLOCK) {
                int j_end = j + BLOCK < SIZE ? j + BLOCK : SIZE;
                #pragma acc loop vector
                for (int k = j; k < j_end; k++) {
                    for (int l = i; l < i_end; l++) {
                        dst[l][k] = src[l][k] + src[k][l] * (n % 3);
                    }
                }
            }
        }
    }
}

/* Function 4: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc kernels num_workers(8) vector_length(128) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j += (n % 4 + 1)) {  /* Runtime-dependent vectorization */
                int j_end = j + (n % 4 + 1) < SIZE ? j + (n % 4 + 1) : SIZE;
                for (int k = j; k < j_end; k++) {
                    dst[i][k] = src[i][k] * (i % (n + 1)) + k;
                }
            }
        }
    }
}

/* Function 5: Gang partitioned */
void compute_gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            int chunk = (SIZE / n) + 1;  /* Runtime-dependent partitioning */
            int start = (i % n) * chunk;
            int end = start + chunk < SIZE ? start + chunk : SIZE;
            for (j = start; j < end; j++) {
                dst[i][j] = src[i][j] * 5 - src[j][i];
            }
        }
    }
}

/* Conditional compilation variants */
#ifdef VARIANT_A
void compute_variant_a(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(8) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i += (n % 8 + 4)) {
            int i_end = i + (n % 8 + 4) < SIZE ? i + (n % 8 + 4) : SIZE;
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                for (int k = i; k < i_end; k++) {
                    dst[k][j] = src[k][j] * (j % (n + 2));
                }
            }
        }
    }
}
#elif defined(VARIANT_B)
void compute_variant_b(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc kernels num_gangs(1) num_workers(16) vector_length(256) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + (i * j) % (n + 5);
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0;
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 23) % 100;
        }
    }
    
    /* Read runtime parameter */
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("Enter partitioning mode (1-5): ");
        scanf("%d", &n);
    }
    
    /* Ensure n is in valid range */
    if (n < 1) n = 1;
    if (n > 5) n = 5;
    
    /* Call different functions based on input to trigger different partitioning */
    switch (n) {
        case 1:
            compute_gang_redundant(src, dst, n);
            break;
        case 2:
            compute_gang_worker_partitioned(src, dst, n);
            break;
        case 3:
            compute_fully_partitioned(src, dst, n);
            break;
        case 4:
            compute_worker_vector_partitioned(src, dst, n);
            break;
        case 5:
            compute_gang_partitioned(src, dst, n);
            break;
    }
    
#ifdef VARIANT_A
    compute_variant_a(src, dst, n);
#elif defined(VARIANT_B)
    compute_variant_b(src, dst, n);
#endif
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < SIZE; i += 64) {
        for (j = 0; j < SIZE; j += 64) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force side effects */
    if (checksum > 1000000) {
        printf("Large checksum detected\n");
    }
    
    return 0;
}
