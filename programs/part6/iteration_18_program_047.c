/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex scoping scenarios */
int global_gang_redundant = 0;        /* Should become case 0 */
float global_matrix[N][M];
static double module_scalar = 3.14159;

/* Function to initialize matrices */
void init_matrices(float A[N][M], float B[N][M]) {
    #pragma acc parallel loop gang vector collapse(2) copyout(A[0:N][0:M], B[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            A[i][j] = (float)(i + j) * 0.1f;
            B[i][j] = (float)(i * j) * 0.01f;
        }
    }
}

int main() {
    float A[N][M], B[N][M], C[N][M];
    int host_scalar = 42;
    float host_array[N];
    
    /* Initialize with test data */
    init_matrices(A, B);
    
    /* Case 1: Gang partitioned variable */
    printf("Starting gang-partitioned computation...\n");
    #pragma acc parallel num_gangs(GANGS) copyin(A[0:N][0:M], B[0:N][0:M]) copyout(C[0:N][0:M])
    {
        int gang_private = 0;  /* Should become case 1 */
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector independent
            for (int j = 0; j < M; j++) {
                C[i][j] = A[i][j] + B[i][j] + gang_private;
            }
        }
    }
    
    /* Case 4: Vector partitioned variable in kernels region */
    printf("Starting vector-partitioned computation...\n");
    float vector_private = 0.0f;
    #pragma acc kernels copyin(A[0:N][0:M]) copyout(C[0:N][0:M])
    {
        #pragma acc loop independent
        for (int i = 0; i < N; i++) {
            float vector_local = (float)i * 0.5f;  /* Should become case 4 */
            #pragma acc loop vector independent private(vector_local)
            for (int j = 0; j < M; j++) {
                C[i][j] = A[i][j] * vector_local;
            }
        }
    }
    
    /* Complex nested parallelism for combined partitioning states */
    printf("Starting complex nested parallelism...\n");
    int combined_states[8] = {0};
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyin(A[0:N][0:M], B[0:N][0:M]) copyout(C[0:N][0:M]) \
                copy(combined_states[0:8])
    {
        /* Case 0: Gang redundant (shared across all gangs) */
        int gang_redundant_var = global_gang_redundant;
        
        /* Case 2: Worker partitioned */
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            int gang_partitioned = g * 100;  /* Case 1 */
            
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                int worker_partitioned = w * 10;  /* Case 2 */
                
                /* Case 3: Gang+Worker partitioned */
                int gang_worker_partitioned = gang_partitioned + worker_partitioned;
                
                #pragma acc loop vector independent
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* Case 4: Vector partitioned */
                    int vector_partitioned = v;
                    
                    /* Case 5: Gang+Vector partitioned */
                    int gang_vector_partitioned = gang_partitioned + v;
                    
                    /* Case 6: Worker+Vector partitioned */
                    int worker_vector_partitioned = worker_partitioned + v;
                    
                    /* Case 7: Fully partitioned */
                    int fully_partitioned = gang_partitioned + worker_partitioned + v;
                    
                    /* Use all variables to prevent optimization */
                    int idx = (g * WORKERS * VECTOR_LEN + w * VECTOR_LEN + v) % 8;
                    combined_states[idx] += gang_redundant_var + gang_worker_partitioned +
                                          vector_partitioned + gang_vector_partitioned +
                                          worker_vector_partitioned + fully_partitioned;
                }
            }
        }
    }
    
    /* Conditional partitioning based on runtime values */
    printf("Starting conditional partitioning...\n");
    int conditional_flag = 1;
    float D[N][M];
    
    #pragma acc parallel num_gangs(GANGS) copyin(A[0:N][0:M], B[0:N][0:M], conditional_flag) \
                copyout(D[0:N][0:M])
    {
        int conditional_var;
        if (conditional_flag) {
            conditional_var = 100;  /* Different partitioning based on condition */
        } else {
            conditional_var = 200;
        }
        
        #pragma acc loop gang independent private(conditional_var)
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector independent
            for (int j = 0; j < M; j++) {
                /* Stencil computation to complicate dependency analysis */
                float left = (j > 0) ? A[i][j-1] : 0.0f;
                float right = (j < M-1) ? A[i][j+1] : 0.0f;
                float up = (i > 0) ? A[i-1][j] : 0.0f;
                float down = (i < N-1) ? A[i+1][j] : 0.0f;
                
                D[i][j] = (A[i][j] + left + right + up + down) * 0.2f + conditional_var;
            }
        }
    }
    
    /* Multi-dimensional array with explicit partitioning clauses */
    printf("Starting multi-dimensional explicit partitioning...\n");
    float E[N][M];
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copyin(A[0:N][0:M], B[0:N][0:M]) copyout(E[0:N][0:M])
    {
        /* Mix of shared and private variables at different levels */
        int gang_level_shared;
        #pragma acc loop gang independent private(gang_level_shared)
        for (int i = 0; i < N; i++) {
            gang_level_shared = i * 10;
            
            #pragma acc loop worker independent
            for (int j = 0; j < M; j++) {
                int worker_level_private = j * 5;
                
                #pragma acc loop vector independent
                for (int k = 0; k < 4; k++) {  /* Small inner loop for vector partitioning */
                    int vector_level_private = k * 2;
                    E[i][j] = A[i][j] + B[i][j] + gang_level_shared + 
                             worker_level_private + vector_level_private;
                }
            }
        }
    }
    
    /* Reduction to verify computation and prevent dead code elimination */
    float checksum = 0.0f;
    #pragma acc parallel loop reduction(+:checksum) copyin(C[0:N][0:M], D[0:N][0:M], E[0:N][0:M])
    for (int i = 0; i < N; i++) {
        #pragma acc loop reduction(+:checksum)
        for (int j = 0; j < M; j++) {
            checksum += C[i][j] + D[i][j] + E[i][j];
        }
    }
    
    printf("Computation complete. Checksum: %f\n", checksum);
    printf("Combined states array sum: %d\n", combined_states[0] + combined_states[7]);
    
    return 0;
}
