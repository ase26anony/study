/* This program is designed to trigger the partitioning classification logic
   in GCC's OpenACC neutering/broadcast analysis, specifically targeting the
   switch statement that maps integer values 0-7 to descriptive strings. */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define SMALL_SIZE 128

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang redundant: only num_gangs(1) specified */
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound */
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang partitioned: only num_gangs specified */
    #pragma acc kernels num_gangs(8) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker partitioned: only num_workers specified */
    #pragma acc parallel num_workers(4) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+worker partitioned: both specified */
    #pragma acc kernels num_gangs(4) num_workers(2) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Vector partitioned: only vector_length specified */
    #pragma acc parallel vector_length(32) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+vector partitioned: num_gangs and vector_length */
    #pragma acc kernels num_gangs(8) vector_length(64) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0x01;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker+vector partitioned: num_workers and vector_length */
    #pragma acc parallel num_workers(2) vector_length(128) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & 0xFE;
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Fully partitioned: all three specified */
    #pragma acc kernels num_gangs(16) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Variant function with conditional compilation */
#ifdef VARIANT
void compute_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Different partitioning in variant */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] ^ 0xFF;
            }
        }
    }
}
#endif

#ifdef VARIANT2
void compute_variant2(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Another variant with collapsed loops */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = (src[i][j] << 1) | (src[i][j] >> 7);
            }
        }
    }
}
#endif

/* Helper function to compute checksum with side effects */
volatile int checksum_result;
void compute_checksum(int arr[SIZE][SIZE], int n) {
    int sum = 0;
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < n; j++) {
            sum += arr[i][j];
        }
    }
    checksum_result = sum;
    printf("Checksum: %d\n", sum);  /* Side effect to prevent optimization */
}

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int i, j, n;
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 100;
        }
    }
    
    /* Read runtime-dependent loop bound */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SIZE;
    }
    
    /* Call different functions based on input to trigger various partitioning cases */
    switch (n % 8) {
        case 0:
            compute_gang_redundant(n, src, dst);
            break;
        case 1:
            compute_gang_partitioned(n, src, dst);
            break;
        case 2:
            compute_worker_partitioned(n, src, dst);
            break;
        case 3:
            compute_gang_worker_partitioned(n, src, dst);
            break;
        case 4:
            compute_vector_partitioned(n, src, dst);
            break;
        case 5:
            compute_gang_vector_partitioned(n, src, dst);
            break;
        case 6:
            compute_worker_vector_partitioned(n, src, dst);
            break;
        case 7:
            compute_fully_partitioned(n, src, dst);
            break;
    }
    
#ifdef VARIANT
    compute_variant(n, src, dst);
#endif
    
#ifdef VARIANT2
    compute_variant2(n, src, dst);
#endif
    
    /* Validation with side effects */
    compute_checksum(dst, n);
    
    return 0;
}
