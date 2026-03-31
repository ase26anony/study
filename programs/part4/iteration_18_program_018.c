/* OpenACC program designed to trigger the partitioning classification logic
   in omp-oacc-neuter-broadcast.cc lines 335-343 */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 512
#define PATTERN 7

/* Initialize arrays with a pattern */
void init_arrays(int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = (i * PATTERN + j) % 100;
            dst[i][j] = 0;
        }
    }
}

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) copyin(src) copyout(dst)
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(1) \
        copyin(src) copy(dst)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] += src[i][j] * 3;
            }
        }
    }
}

/* Function 3: Vector partitioned */
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_gangs(1) num_workers(1) vector_length(64) \
        copyin(src) copy(dst)
    {
        #pragma acc loop vector
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] += src[i][j] * 5;
            }
        }
    }
}

/* Function 4: Fully partitioned (gang+worker+vector) */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(16) num_workers(2) vector_length(32) \
        copyin(src) copy(dst)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] += src[i][j] * 7;
            }
        }
    }
}

/* Function 5: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        copyin(src) copy(dst)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] += src[i][j] * 11;
            }
        }
    }
}

/* Conditional compilation variants */
#ifdef VARIANT_A
#define GANG_COUNT 4
#define WORKER_COUNT 2
#else
#define GANG_COUNT 2
#define WORKER_COUNT 4
#endif

/* Function with conditional compilation */
void compute_variant_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(GANG_COUNT) num_workers(WORKER_COUNT) vector_length(32) \
        copyin(src) copy(dst)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            #ifdef COLLAPSE_LOOPS
            #pragma acc loop vector collapse(2)
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < 2; k++) {
                    dst[i][j] += src[i][j] * (k + 1);
                }
            }
            #else
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {
                dst[i][j] += src[i][j] * 13;
            }
            #endif
        }
    }
}

/* Validation checksum */
int validate(int arr[SIZE][SIZE], int n) {
    int sum = 0;
    volatile int prevent_opt = 0; /* Prevent optimization */
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < n; j++) {
            sum += arr[i][j];
            prevent_opt = arr[i][j]; /* Side effect */
        }
    }
    
    printf("Checksum: %d\n", sum);
    return sum;
}

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int n, choice;
    
    /* Read runtime parameter for loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    }
    
    /* Read choice for which partitioning to test */
    if (argc > 2) {
        choice = atoi(argv[2]);
    } else {
        printf("Enter test choice (1-5): ");
        scanf("%d", &choice);
    }
    
    init_arrays(src, dst);
    
    /* Select different partitioning scenarios */
    switch (choice) {
        case 1:
            compute_gang_redundant(src, dst, n);
            break;
        case 2:
            compute_gang_worker_partitioned(src, dst, n);
            break;
        case 3:
            compute_vector_partitioned(src, dst, n);
            break;
        case 4:
            compute_fully_partitioned(src, dst, n);
            break;
        case 5:
            compute_worker_vector_partitioned(src, dst, n);
            break;
        default:
            compute_variant_partitioned(src, dst, n);
    }
    
    /* Force side effects and validation */
    validate(dst, n);
    
    /* Additional calls to ensure multiple instances */
    if (choice == 1) {
        compute_gang_worker_partitioned(src, dst, n/2);
        printf("Secondary checksum: %d\n", validate(dst, n/2));
    }
    
    return 0;
}
