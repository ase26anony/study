#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare analysis */
#pragma acc declare create(global_matrix, global_sum)

#define SIZE 512
int global_matrix[SIZE][SIZE];
int global_sum = 0;

/* Function with complex OpenACC data partitioning */
void process_data(int argc, char *argv[]) {
    /* Dynamic bounds to prevent constant folding */
    volatile int N = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (N > SIZE) N = SIZE;
    
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int partial_sums[16] = {0};  /* For nested reduction patterns */
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            src[i][j] = i * N + j;
            dst[i][j] = 0;
            global_matrix[i][j] = (i + j) % 256;
        }
    }
    
    /* OpenACC data region with multiple partitioning scenarios */
    #pragma acc data copy(src[0:N][0:N], dst[0:N][0:N]) \
                     copyin(global_matrix[0:N][0:N]) \
                     copyout(partial_sums)
    {
        int total_sum = 0;
        int tmp;  /* Will be made private */
        
        /* SCENARIO 1: Complex partitioning with gang+worker+vector */
        /* This should trigger multiple partitioning type analyses */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp) reduction(+:total_sum) \
                firstprivate(N) async(1)
        for (int i = 1; i < N-1; i++) {
            for (int j = 1; j < N-1; j++) {
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with private variable */
                    tmp = src[i-1][j] + src[i+1][j] + 
                          src[i][j-1] + src[i][j+1];
                    dst[i][j] = tmp / 4;
                    
                    /* Nested conditional with reduction */
                    if (tmp > 1000) {
                        total_sum += tmp;
                    }
                } else if ((i * j) % 7 == 0) {
                    /* Different access pattern */
                    dst[i][j] = global_matrix[i][j] * 2;
                    total_sum += global_matrix[i][j];
                } else {
                    dst[i][j] = src[i][j];
                }
                
                /* Worker-level private computation */
                int worker_local = i * j;
                if (worker_local % 11 == 0) {
                    dst[i][j] += worker_local % 256;
                }
            }
        }
        
        #pragma acc wait(1)
        
        /* SCENARIO 2: Different partitioning - gang+vector only */
        int row_sums[SIZE] = {0};
        #pragma acc kernels loop gang vector \
                copy(row_sums[0:N]) private(tmp) \
                firstprivate(N) async(2)
        for (int i = 0; i < N; i++) {
            tmp = 0;
            #pragma acc loop vector reduction(+:tmp)
            for (int j = 0; j < N; j++) {
                /* Conditional reduction */
                if (dst[i][j] > 128) {
                    tmp += dst[i][j];
                } else {
                    tmp -= dst[i][j] / 2;
                }
            }
            row_sums[i] = tmp;
            
            /* Update global persistent data */
            #pragma acc atomic update
            global_sum += tmp;
        }
        
        /* SCENARIO 3: Worker+vector partitioning pattern */
        int block_sum = 0;
        #pragma acc parallel loop gang worker vector collapse(2) \
                reduction(+:block_sum) firstprivate(N) \
                private(tmp) async(3)
        for (int i = 0; i < N; i += 16) {
            for (int j = 0; j < N; j += 16) {
                /* Process 16x16 block */
                tmp = 0;
                #pragma acc loop worker vector collapse(2) reduction(+:tmp)
                for (int bi = i; bi < i+16 && bi < N; bi++) {
                    for (int bj = j; bj < j+16 && bj < N; bj++) {
                        /* Mixed access patterns */
                        if ((bi + bj) % 2 == 0) {
                            tmp += dst[bi][bj] * src[bi][bj];
                        } else {
                            tmp -= dst[bi][bj] / 3;
                        }
                    }
                }
                partial_sums[(i/16) * (N/16) + (j/16)] = tmp;
                block_sum += tmp;
            }
        }
        
        #pragma acc wait(2)
        #pragma acc wait(3)
        
        /* SCENARIO 4: Fully partitioned with nested reductions */
        int final_sum = 0;
        #pragma acc parallel loop gang worker vector collapse(3) \
                reduction(+:final_sum) firstprivate(N)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < 4; k++) {  /* Artificial 3rd dimension */
                    int idx = (i + j + k) % N;
                    /* Complex conditional chain */
                    if (dst[i][j] > src[idx][idx]) {
                        final_sum += dst[i][j] - src[idx][idx];
                    } else if (dst[i][j] < 0) {
                        final_sum -= src[idx][idx];
                    } else {
                        final_sum += k * global_matrix[i][j];
                    }
                }
            }
        }
        
        /* Mix OpenACC and OpenMP */
        #pragma acc update host(dst[0:N][0:N])
        
        /* Pure OpenMP section */
        int omp_sum = 0;
        #pragma omp parallel for reduction(+:omp_sum) collapse(2) \
                schedule(dynamic, 16)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                /* Cross-check with OpenACC results */
                omp_sum += dst[i][j];
                
                /* Additional OpenMP-only computation */
                if (i % 8 == 0 && j % 8 == 0) {
                    #pragma omp atomic
                    global_sum += 1;
                }
            }
        }
        
        printf("Total sum from OpenACC: %d\n", total_sum);
        printf("Block sum: %d\n", block_sum);
        printf("Final 3D sum: %d\n", final_sum);
        printf("OpenMP verification sum: %d\n", omp_sum);
        
        /* Use results to prevent dead code elimination */
        if (total_sum != 0 || block_sum != 0 || final_sum != 0) {
            printf("Computation performed successfully\n");
        }
    }
    
    /* Additional OpenMP outside data region */
    int host_array[SIZE];
    #pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        host_array[i] = i * i;
        /* Access global data updated by OpenACC */
        if (i < N) {
            host_array[i] += global_sum % 100;
        }
    }
    
    /* Final check using host_array */
    int host_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        host_sum += host_array[i];
    }
    printf("Host array checksum: %d\n", host_sum);
}

int main(int argc, char *argv[]) {
    printf("Starting complex OpenACC partitioning test...\n");
    
    /* Initialize global data */
    #pragma acc update device(global_matrix, global_sum)
    
    /* Call processing function multiple times with different sizes */
    process_data(argc, argv);
    
    /* Second call with different parameters */
    if (argc > 2) {
        char* dummy_argv[2] = {argv[0], "256"};
        process_data(2, dummy_argv);
    }
    
    printf("Test completed.\n");
    return 0;
}
