/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables for different scoping */
int global_gang_redundant = 0;  /* Should become gang redundant (case 0) */
float global_matrix[N][M];
static int file_static_counter = 0;

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
    float A[N][M], C[N][M], D[N][M];
    int i, j, k;
    
    /* Initialize input matrices */
    init_matrices(A, C);
    
    /* Reset global for gang redundancy test */
    global_gang_redundant = 42;
    
    /*****************************************************************
     * CASE 1: Gang Partitioned
     * Variables private to each gang
     *****************************************************************/
    #pragma acc parallel num_gangs(GANGS) copyin(A[0:N][0:M]) copyout(C[0:N][0:M])
    {
        int gang_private = 0;  /* Should be gang partitioned (case 1) */
        float gang_local[M];
        
        #pragma acc loop gang private(gang_private, gang_local)
        for (i = 0; i < N; i++) {
            gang_private = i * 100;
            for (j = 0; j < M; j++) {
                gang_local[j] = A[i][j] * 2.0f;
                C[i][j] = gang_local[j] + gang_private;
            }
        }
    }
    
    /*****************************************************************
     * CASE 0: Gang Redundant
     * Variable accessible by all gangs
     *****************************************************************/
    #pragma acc parallel num_gangs(GANGS) copy(global_gang_redundant) \
        copyin(A[0:N][0:M]) copyout(D[0:N][0:M])
    {
        /* global_gang_redundant should be gang redundant (case 0) */
        int gang_id = 0;
        
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            #pragma acc atomic update
            global_gang_redundant += 1;
            
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                D[i][j] = A[i][j] + global_gang_redundant;
            }
        }
    }
    
    /*****************************************************************
     * CASE 4: Vector Partitioned
     * Variables private to each vector lane
     *****************************************************************/
    #pragma acc kernels copyin(A[0:N][0:M]) copyout(C[0:N][0:M])
    {
        float vector_private;  /* Should be vector partitioned (case 4) */
        
        #pragma acc loop independent
        for (i = 0; i < N; i++) {
            #pragma acc loop vector private(vector_private)
            for (j = 0; j < M; j++) {
                vector_private = sinf(A[i][j]);
                C[i][j] = vector_private * vector_private;
            }
        }
    }
    
    /*****************************************************************
     * CASE 2: Worker Partitioned
     * Variables private to each worker
     *****************************************************************/
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copyin(A[0:N][0:M], C[0:N][0:M]) copyout(D[0:N][0:M])
    {
        int worker_private;  /* Should be worker partitioned (case 2) */
        
        #pragma acc loop gang
        for (i = 0; i < N; i += WORKERS) {
            #pragma acc loop worker private(worker_private)
            for (int w = 0; w < WORKERS && (i + w) < N; w++) {
                worker_private = w * 1000;
                
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    D[i + w][j] = A[i + w][j] + C[i + w][j] + worker_private;
                }
            }
        }
    }
    
    /*****************************************************************
     * CASE 3: Gang+Worker Partitioned
     * Variables partitioned across both gangs and workers
     *****************************************************************/
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copyin(A[0:N][0:M]) copyout(C[0:N][0:M])
    {
        /* Complex nested variable with mixed attributes */
        int gw_partitioned;  /* Should be gang+worker partitioned (case 3) */
        float gw_array[VECTOR_LEN];
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker private(gw_partitioned, gw_array)
            for (int w = 0; w < WORKERS; w++) {
                gw_partitioned = g * 10000 + w * 100;
                
                int start_i = g * (N / GANGS) + w * (N / (GANGS * WORKERS));
                int end_i = start_i + (N / (GANGS * WORKERS));
                
                for (i = start_i; i < end_i && i < N; i++) {
                    #pragma acc loop vector
                    for (j = 0; j < M; j += VECTOR_LEN) {
                        /* Vector operation with worker-private array */
                        for (int v = 0; v < VECTOR_LEN && (j + v) < M; v++) {
                            gw_array[v] = A[i][j + v] * gw_partitioned;
                            C[i][j + v] = gw_array[v] + sqrtf(fabsf(A[i][j + v]));
                        }
                    }
                }
            }
        }
    }
    
    /*****************************************************************
     * CASE 5: Gang+Vector Partitioned
     * Variables partitioned across gangs and vector lanes
     *****************************************************************/
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
        copyin(A[0:N][0:M]) copyout(D[0:N][0:M])
    {
        float gv_partitioned;  /* Should be gang+vector partitioned (case 5) */
        
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            gv_partitioned = i * 3.14159f;
            
            #pragma acc loop vector private(gv_partitioned)
            for (j = 0; j < M; j++) {
                /* Stencil computation - complicates dependency analysis */
                float left = (j > 0) ? A[i][j-1] : 0.0f;
                float right = (j < M-1) ? A[i][j+1] : 0.0f;
                float up = (i > 0) ? A[i-1][j] : 0.0f;
                float down = (i < N-1) ? A[i+1][j] : 0.0f;
                
                D[i][j] = (left + right + up + down) * 0.25f + gv_partitioned;
            }
        }
    }
    
    /*****************************************************************
     * CASE 6: Worker+Vector Partitioned
     * Variables partitioned across workers and vector lanes
     *****************************************************************/
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyin(A[0:N][0:M], C[0:N][0:M]) copyout(D[0:N][0:M])
    {
        /* Single gang, multiple workers with vector lanes */
        float wv_partitioned[VECTOR_LEN];  /* Should be worker+vector partitioned (case 6) */
        
        #pragma acc loop worker private(wv_partitioned)
        for (int w = 0; w < WORKERS; w++) {
            int start_i = w * (N / WORKERS);
            int end_i = (w + 1) * (N / WORKERS);
            
            for (i = start_i; i < end_i && i < N; i++) {
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    int lane = j % VECTOR_LEN;
                    wv_partitioned[lane] = cosf(A[i][j]) + sinf(C[i][j]);
                    D[i][j] = wv_partitioned[lane] * expf(wv_partitioned[lane]);
                }
            }
        }
    }
    
    /*****************************************************************
     * CASE 7: Fully Partitioned
     * Variables partitioned across gangs, workers, and vectors
     *****************************************************************/
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyin(A[0:N][0:M]) copyout(C[0:N][0:M])
    {
        /* The most complex partitioning scenario */
        int fully_partitioned;  /* Should be fully partitioned (case 7) */
        float fp_array[VECTOR_LEN];
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker private(fully_partitioned, fp_array)
            for (int w = 0; w < WORKERS; w++) {
                fully_partitioned = g * WORKERS + w;
                
                int base_i = g * (N / GANGS) + w * (N / (GANGS * WORKERS));
                int chunk = N / (GANGS * WORKERS * 2);
                
                for (int chunk_idx = 0; chunk_idx < 2; chunk_idx++) {
                    int start_i = base_i + chunk_idx * chunk;
                    int end_i = start_i + chunk;
                    
                    for (i = start_i; i < end_i && i < N; i++) {
                        #pragma acc loop vector private(fully_partitioned, fp_array)
                        for (j = 0; j < M; j += VECTOR_LEN) {
                            /* Each vector lane gets its own slice */
                            for (int v = 0; v < VECTOR_LEN && (j + v) < M; v++) {
                                fp_array[v] = A[i][j + v] * fully_partitioned;
                                fully_partitioned = (fully_partitioned * 1103515245 + 12345) & 0x7fffffff;
                                C[i][j + v] = fp_array[v] + logf(fabsf(fp_array[v]) + 1.0f);
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* Conditional partitioning based on runtime values */
    int dynamic_condition = (N * M) > 100000 ? 1 : 0;
    
    #pragma acc parallel num_gangs(dynamic_condition ? 4 : 2) \
        copyin(A[0:N][0:M]) copyout(D[0:N][0:M])
    {
        int conditional_var;  /* Partitioning depends on runtime condition */
        
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            conditional_var = i * (dynamic_condition + 1);
            
            #pragma acc loop vector
            for (j = 0; j < M; j++) {
                /* Multi-dimensional stencil with conditional access */
                float sum = 0.0f;
                int count = 0;
                
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        int ni = i + di;
                        int nj = j + dj;
                        if (ni >= 0 && ni < N && nj >= 0 && nj < M) {
                            sum += A[ni][nj];
                            count++;
                        }
                    }
                }
                
                D[i][j] = (count > 0) ? sum / count + conditional_var : 0.0f;
            }
        }
    }
    
    /* Final reduction to verify computation and prevent dead code elimination */
    float checksum = 0.0f;
    #pragma acc parallel loop reduction(+:checksum) copyin(C[0:N][0:M], D[0:N][0:M])
    for (i = 0; i < N; i++) {
        #pragma acc loop vector reduction(+:checksum)
        for (j = 0; j < M; j++) {
            checksum += C[i][j] + D[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Global gang redundant value: %d\n", global_gang_redundant);
    
    return 0;
}
