#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define SMALL_SIZE 256

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - (i * j);
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & 0x7F;
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 + i - j;
            }
        }
    }
}

/* Variant with different clause combinations using conditional compilation */
#ifdef VARIANT_A
void compute_variant_a(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc kernels num_gangs(16) num_workers(2) vector_length(8) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop collapse(2)
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] << 1;
            }
        }
    }
}
#elif defined(VARIANT_B)
void compute_variant_b(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(2) num_workers(16) vector_length(4) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] >> 1;
            }
        }
    }
}
#else
void compute_variant_default(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc kernels num_gangs(1) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + 100;
            }
        }
    }
}
#endif

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
    
    /* Read runtime-dependent loop bound */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SMALL_SIZE;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SMALL_SIZE;
    }
    
    /* Call different functions based on input to trigger various partitioning scenarios */
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
    
    /* Also call variant function */
#ifdef VARIANT_A
    compute_variant_a(n, src, dst);
#elif defined(VARIANT_B)
    compute_variant_b(n, src, dst);
#else
    compute_variant_default(n, src, dst);
#endif
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force side effect */
    volatile int dummy = checksum;
    
    return 0;
}
