/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_gang_redundant = 0;  /* Should become case 0 */
float global_matrix[N][M];
static double static_partial_sum = 0.0;

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
    float A[N][M], B[N][M], C[N][M];
    float D[N][M];  /* Another matrix for different partitioning */
    int i, j, k;
    
    /* Initialize with OpenACC to trigger early partitioning analysis */
    init_matrices(A, B);
    
    /* ==================== CASE 0: Gang Redundant ==================== */
    /* Variable available redundantly to all gangs */
    int gang_redundant_var = 42;
    #pragma acc parallel num_gangs(GANGS) copy(gang_redundant_var) \
            copyin(global_gang_redundant)
    {
        /* This variable should be marked as gang redundant */
        int local_copy = gang_redundant_var + global_gang_redundant;
        #pragma acc loop gang
        for (i = 0; i < GANGS; i++) {
            /* Each gang uses the same redundant value */
            int temp = local_copy * i;
        }
    }
    
    /* ==================== CASE 1: Gang Partitioned ==================== */
    /* Variables partitioned across gangs only */
    int gang_partitioned[GANGS];
    #pragma acc parallel num_gangs(GANGS) copyout(gang_partitioned[0:GANGS])
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            /* Each gang gets its own private copy */
            gang_partitioned[g] = g * 100;
        }
    }
    
    /* ==================== CASE 2: Worker Partitioned ==================== */
    /* Using kernels region with worker-only partitioning */
    int worker_local[WORKERS];
    #pragma acc kernels copyout(worker_local[0:WORKERS]) \
            num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            worker_local[w] = w * 10;
            /* Nested vector loop - creates worker partitioning context */
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                int temp = worker_local[w] + v;
            }
        }
    }
    
    /* ==================== CASE 3: Gang+Worker Partitioned ==================== */
    /* Complex nested parallelism with gang and worker clauses */
    float gw_partitioned[GANGS][WORKERS];
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
            copyout(gw_partitioned[0:GANGS][0:WORKERS])
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                /* Each gang-worker pair gets unique value */
                gw_partitioned[g][w] = (float)(g * WORKERS + w);
            }
        }
    }
    
    /* ==================== CASE 4: Vector Partitioned ==================== */
    /* Vector-only partitioning in a kernels region */
    float vector_only[VECTOR_LEN];
    #pragma acc kernels copyout(vector_only[0:VECTOR_LEN]) \
            vector_length(VECTOR_LEN)
    {
        #pragma acc loop vector
        for (int v = 0; v < VECTOR_LEN; v++) {
            vector_only[v] = sinf((float)v * 0.1f);
        }
    }
    
    /* ==================== CASE 5: Gang+Vector Partitioned ==================== */
    /* Matrix computation with gang and vector partitioning */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
            copyin(A[0:N][0:M], B[0:N][0:M]) copyout(C[0:N][0:M])
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            /* Mixed partitioning: gang at outer, vector at inner */
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                C[i][j] = A[i][j] + B[i][j];
            }
        }
    }
    
    /* ==================== CASE 6: Worker+Vector Partitioned ==================== */
    /* Using parallel region with worker and vector clauses */
    float wv_partitioned[WORKERS][VECTOR_LEN];
    #pragma acc parallel num_workers(WORKERS) vector_length(VECTOR_LEN) \
            copyout(wv_partitioned[0:WORKERS][0:VECTOR_LEN])
    {
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                wv_partitioned[w][v] = cosf((float)(w * VECTOR_LEN + v) * 0.05f);
            }
        }
    }
    
    /* ==================== CASE 7: Fully Partitioned ==================== */
    /* Most complex: gang, worker, and vector partitioning */
    float fully_partitioned[GANGS][WORKERS][VECTOR_LEN];
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
            vector_length(VECTOR_LEN) \
            copyout(fully_partitioned[0:GANGS][0:WORKERS][0:VECTOR_LEN])
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* Each combination gets unique computation */
                    fully_partitioned[g][w][v] = 
                        tanhf((float)(g * WORKERS * VECTOR_LEN + 
                                     w * VECTOR_LEN + v) * 0.01f);
                }
            }
        }
    }
    
    /* ==================== Conditional Partitioning ==================== */
    /* Runtime-dependent partitioning to force analysis of multiple paths */
    int conditional_flag = 1;
    float conditional_result[N][M];
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
            vector_length(VECTOR_LEN) \
            copyin(conditional_flag, A[0:N][0:M], B[0:N][0:M]) \
            copyout(conditional_result[0:N][0:M])
    {
        if (conditional_flag) {
            /* One partitioning strategy */
            #pragma acc loop gang worker
            for (i = 0; i < N; i++) {
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    conditional_result[i][j] = A[i][j] * B[i][j];
                }
            }
        } else {
            /* Alternative partitioning strategy */
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                #pragma acc loop worker vector
                for (j = 0; j < M; j++) {
                    conditional_result[i][j] = A[i][j] / (B[i][j] + 1.0f);
                }
            }
        }
    }
    
    /* ==================== Stencil Computation ==================== */
    /* Multi-dimensional access pattern complicating dependency analysis */
    #pragma acc kernels copyin(C[0:N][0:M]) copyout(D[0:N][0:M]) \
            num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        #pragma acc loop gang
        for (i = 1; i < N-1; i++) {
            #pragma acc loop worker
            for (j = 1; j < M-1; j++) {
                /* Stencil access - needs careful partitioning */
                #pragma acc loop vector
                for (k = 0; k < 3; k++) {
                    D[i][j] = (C[i-1][j] + C[i+1][j] + 
                              C[i][j-1] + C[i][j+1]) * 0.25f;
                }
            }
        }
    }
    
    /* ==================== Reduction with Mixed Partitioning ==================== */
    double final_sum = 0.0;
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
            vector_length(VECTOR_LEN) copyin(D[0:N][0:M]) \
            copy(final_sum) reduction(+:final_sum)
    {
        #pragma acc loop gang reduction(+:final_sum)
        for (i = 0; i < N; i++) {
            #pragma acc loop worker reduction(+:final_sum)
            for (j = 0; j < M; j++) {
                #pragma acc loop vector reduction(+:final_sum)
                for (k = 0; k < 1; k++) {  /* Simplified for example */
                    final_sum += D[i][j];
                }
            }
        }
    }
    
    /* Verify computation and print result */
    printf("Final sum: %f\n", final_sum);
    printf("Partitioning test completed.\n");
    
    /* Print some values to prevent optimization */
    printf("Sample values:\n");
    printf("  Gang partitioned[0] = %d\n", gang_partitioned[0]);
    printf("  Worker partitioned[0] = %d\n", worker_local[0]);
    printf("  GW partitioned[0][0] = %f\n", gw_partitioned[0][0]);
    printf("  Vector only[0] = %f\n", vector_only[0]);
    printf("  WV partitioned[0][0] = %f\n", wv_partitioned[0][0]);
    printf("  Fully partitioned[0][0][0] = %f\n", fully_partitioned[0][0][0]);
    printf("  Conditional result[10][10] = %f\n", conditional_result[10][10]);
    
    return 0;
}
