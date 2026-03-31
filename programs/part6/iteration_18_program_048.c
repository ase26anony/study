/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
static float global_matrix[N][M];        /* Base for complex partitioning */

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

/* Main function with complex partitioning scenarios */
int main() {
    float A[N][M], B[N][M], C[N][M], D[N][M];
    int i, j;
    
    /* Initialize data */
    init_matrices(A, B);
    
    /* ============================================
       SCENARIO 1: Gang-only partitioning (case 1)
       ============================================ */
    {
        int gang_private_scalar;  /* Will be gang partitioned */
        float gang_private_array[GANGS][VECTOR_LEN];
        
        #pragma acc parallel num_gangs(GANGS) copyin(A[0:N][0:M]) \
                private(gang_private_scalar, gang_private_array)
        {
            int gang_id = __pgi_gangidx();
            gang_private_scalar = gang_id * 100;
            
            #pragma acc loop gang
            for (i = 0; i < GANGS; i++) {
                if (gang_id == i) {
                    for (j = 0; j < VECTOR_LEN; j++) {
                        gang_private_array[i][j] = (float)(gang_id + j);
                    }
                }
            }
            
            /* Simple computation to use variables */
            #pragma acc loop gang
            for (i = 0; i < N; i += N/GANGS) {
                int local_i = i + gang_id * (N/GANGS);
                if (local_i < N) {
                    C[local_i][0] = A[local_i][0] + gang_private_scalar;
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 2: Vector-only partitioning (case 4)
       ============================================ */
    {
        float vector_private[M];  /* Will be vector partitioned */
        
        #pragma acc kernels copyin(B[0:N][0:M]) copyout(C[0:N][0:M]) \
                private(vector_private)
        {
            #pragma acc loop independent vector(VECTOR_LEN)
            for (i = 0; i < N; i++) {
                /* Vector-private variable */
                for (j = 0; j < M; j++) {
                    vector_private[j] = sinf((float)j * 0.1f);
                    C[i][j] = B[i][j] * vector_private[j];
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 3: Worker-only partitioning (case 2)
       ============================================ */
    {
        int worker_private_var;  /* Worker partitioned */
        
        #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(1) \
                copy(C[0:N][0:M]) private(worker_private_var)
        {
            int worker_id = __pgi_workeridx();
            worker_private_var = worker_id * 50;
            
            #pragma acc loop worker
            for (i = worker_id; i < N; i += WORKERS) {
                C[i][0] += (float)worker_private_var;
            }
        }
    }
    
    /* ============================================
       SCENARIO 4: Complex combined partitioning
       Targeting cases 3, 5, 6, 7
       ============================================ */
    {
        /* Variables for different partitioning combinations */
        int gang_worker_partitioned;      /* Case 3 */
        float gang_vector_partitioned[VECTOR_LEN];  /* Case 5 */
        int worker_vector_partitioned;    /* Case 6 */
        float fully_partitioned[N][M];    /* Case 7 */
        
        /* Initialize D matrix */
        #pragma acc parallel loop gang vector collapse(2) copyout(D[0:N][0:M])
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                D[i][j] = 0.0f;
            }
        }
        
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                vector_length(VECTOR_LEN) \
                copyin(A[0:N][0:M], B[0:N][0:M]) copy(D[0:N][0:M]) \
                private(gang_worker_partitioned, gang_vector_partitioned, \
                       worker_vector_partitioned) \
                create(fully_partitioned[0:N][0:M])
        {
            int gang_id = __pgi_gangidx();
            int worker_id = __pgi_workeridx();
            int vector_id = __pgi_vectoridx();
            
            /* Gang+Worker partitioned (case 3) */
            gang_worker_partitioned = gang_id * 1000 + worker_id * 100;
            
            /* Gang+Vector partitioned (case 5) */
            if (worker_id == 0) {
                gang_vector_partitioned[vector_id] = 
                    (float)(gang_id * VECTOR_LEN + vector_id);
            }
            
            /* Worker+Vector partitioned (case 6) */
            worker_vector_partitioned = worker_id * VECTOR_LEN + vector_id;
            
            /* Fully partitioned (case 7) - each thread gets its own slice */
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    fully_partitioned[i][j] = 
                        A[i][j] + B[i][j] + 
                        (float)gang_worker_partitioned * 0.001f +
                        gang_vector_partitioned[vector_id % VECTOR_LEN] * 0.01f +
                        (float)worker_vector_partitioned * 0.0001f;
                }
            }
            
            /* Use the global gang-redundant variable (case 0) */
            if (gang_id == 0 && worker_id == 0 && vector_id == 0) {
                global_gang_redundant++;
            }
            
            /* Final computation combining all partitioning types */
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    D[i][j] = fully_partitioned[i][j] * 
                             (1.0f + (float)global_gang_redundant * 0.00001f);
                }
            }
        }
    }
    
    /* ============================================
       SCENARIO 5: Conditional partitioning
       ============================================ */
    {
        int conditionally_partitioned;
        float temp_result[N][M];
        
        #pragma acc kernels copyin(C[0:N][0:M]) copyout(temp_result[0:N][0:M])
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    /* Runtime condition affecting partitioning */
                    if (C[i][j] > 100.0f) {
                        conditionally_partitioned = 1;
                        temp_result[i][j] = C[i][j] * 2.0f;
                    } else {
                        conditionally_partitioned = 0;
                        temp_result[i][j] = C[i][j] * 0.5f;
                    }
                    
                    /* Stencil computation to complicate analysis */
                    if (i > 0 && j > 0) {
                        temp_result[i][j] += 
                            temp_result[i-1][j] * 0.3f +
                            temp_result[i][j-1] * 0.3f;
                    }
                }
            }
        }
        
        /* Final reduction to prevent dead code elimination */
        float checksum = 0.0f;
        #pragma acc parallel loop reduction(+:checksum) \
                copyin(temp_result[0:N][0:M]) copyout(checksum)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                checksum += temp_result[i][j] + D[i][j];
            }
        }
        
        printf("Final checksum: %f\n", checksum);
        printf("Global gang redundant value: %d\n", global_gang_redundant);
    }
    
    return 0;
}
