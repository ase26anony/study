/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex scoping */
int global_gang_redundant = 0;  /* Should become case 0 */
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
    float A[N][M], B[N][M], C[N][M], D[N][M];
    int i, j, k;
    
    /* Initialize with test data */
    init_matrices(A, B);
    memset(C, 0, sizeof(C));
    memset(D, 0, sizeof(D));
    
    /* ============================================
       CASE 0: "gang redundant"
       Variable available redundantly to all gangs
       ============================================ */
    {
        int gang_redundant_var = 42;  /* Should be marked gang redundant */
        
        #pragma acc parallel num_gangs(GANGS) copyin(gang_redundant_var) \
                copy(A[0:N][0:M], B[0:N][0:M]) copyout(C[0:N][0:M])
        {
            /* This variable is read-only and same for all gangs */
            int local_copy = gang_redundant_var;
            
            #pragma acc loop gang independent
            for (int g = 0; g < N; g += N/GANGS) {
                /* Each gang processes a chunk */
                #pragma acc loop vector independent
                for (int i = g; i < g + N/GANGS && i < N; i++) {
                    #pragma acc loop vector independent
                    for (int j = 0; j < M; j++) {
                        C[i][j] = A[i][j] + B[i][j] + local_copy;
                    }
                }
            }
        }
    }
    
    /* ============================================
       CASE 1: "gang partitioned"
       Variable partitioned across gangs only
       ============================================ */
    {
        int gang_partitioned = 0;  /* Should be gang partitioned */
        
        #pragma acc parallel num_gangs(GANGS) private(gang_partitioned) \
                copy(A[0:N][0:M], B[0:N][0:M]) copyout(D[0:N][0:M])
        {
            /* Each gang gets its own copy */
            gang_partitioned = __pgi_gangidx() * 100;
            
            #pragma acc loop gang independent
            for (int g = 0; g < N; g += N/GANGS) {
                int gang_local = gang_partitioned + g;
                
                #pragma acc loop vector independent
                for (int i = g; i < g + N/GANGS && i < N; i++) {
                    #pragma acc loop vector independent
                    for (int j = 0; j < M; j++) {
                        D[i][j] = A[i][j] * B[i][j] + gang_local;
                    }
                }
            }
        }
    }
    
    /* ============================================
       CASE 2: "worker partitioned" 
       CASE 4: "vector partitioned"
       Using kernels region with worker/vector loops
       ============================================ */
    {
        float worker_partitioned[M];  /* Should be worker partitioned (case 2) */
        float vector_partitioned;     /* Should be vector partitioned (case 4) */
        
        #pragma acc kernels copyin(A[0:N][0:M]) copyout(C[0:N][0:M]) \
                create(worker_partitioned[0:M])
        {
            /* Worker-level computation */
            #pragma acc loop worker independent
            for (int w = 0; w < M; w++) {
                /* Each worker initializes its slice */
                worker_partitioned[w] = w * 0.5f;
                
                #pragma acc loop vector independent
                for (int i = 0; i < N; i++) {
                    vector_partitioned = i * 0.25f;  /* Vector partitioned */
                    C[i][w] = A[i][w] + worker_partitioned[w] + vector_partitioned;
                }
            }
        }
    }
    
    /* ============================================
       CASE 3: "gang+worker partitioned"
       CASE 5: "gang+vector partitioned"  
       CASE 6: "worker+vector partitioned"
       CASE 7: "fully partitioned"
       Complex nested parallelism with all levels
       ============================================ */
    {
        int gw_partitioned;      /* Should be gang+worker partitioned (case 3) */
        int gv_partitioned;      /* Should be gang+vector partitioned (case 5) */
        int wv_partitioned;      /* Should be worker+vector partitioned (case 6) */
        int fully_partitioned;   /* Should be fully partitioned (case 7) */
        
        /* Multi-dimensional stencil computation */
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copy(A[0:N][0:M]) copyout(D[0:N][0:M]) \
                private(gw_partitioned, gv_partitioned, wv_partitioned, fully_partitioned)
        {
            /* Gang+Worker partitioned variable */
            gw_partitioned = __pgi_gangidx() * 1000 + __pgi_workeridx() * 100;
            
            #pragma acc loop gang independent
            for (int g = 0; g < N; g += N/GANGS) {
                /* Gang+Vector partitioned variable */
                gv_partitioned = __pgi_gangidx() * 500 + __pgi_vectoridx();
                
                #pragma acc loop worker independent
                for (int w = 0; w < WORKERS; w++) {
                    /* Worker+Vector partitioned variable */
                    wv_partitioned = __pgi_workeridx() * 200 + __pgi_vectoridx();
                    
                    #pragma acc loop vector independent
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        /* Fully partitioned variable - unique to each parallel element */
                        fully_partitioned = __pgi_gangidx() * 1000000 + 
                                          __pgi_workeridx() * 10000 + 
                                          __pgi_vectoridx() * 100 + v;
                        
                        /* Compute global indices */
                        int i = g + __pgi_workeridx() * (N/GANGS/WORKERS) + v;
                        int j = __pgi_vectoridx() * (M/VECTOR_LEN) + w;
                        
                        if (i < N && j < M) {
                            /* Stencil computation accessing neighbors */
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
                            
                            D[i][j] = sum / count + 
                                     gw_partitioned * 0.001f +
                                     gv_partitioned * 0.0001f +
                                     wv_partitioned * 0.00001f +
                                     fully_partitioned * 0.000001f;
                        }
                    }
                }
            }
        }
    }
    
    /* ============================================
       Mixed constructs with conditional partitioning
       ============================================ */
    {
        int conditional_var;
        float temp_matrix[N][M];
        
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                copy(A[0:N][0:M], B[0:N][0:M]) copyout(temp_matrix[0:N][0:M]) \
                private(conditional_var)
        {
            /* Conditional partitioning based on runtime value */
            if (__pgi_gangidx() % 2 == 0) {
                conditional_var = 1;  /* May affect partitioning analysis */
            } else {
                conditional_var = 2;
            }
            
            #pragma acc loop gang independent
            for (int g = 0; g < N; g += N/GANGS) {
                #pragma acc loop worker independent
                for (int w = 0; w < WORKERS; w++) {
                    #pragma acc loop vector independent
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        int i = g + w * VECTOR_LEN + v;
                        if (i < N) {
                            #pragma acc loop vector independent
                            for (int j = 0; j < M; j++) {
                                /* Complex access pattern */
                                temp_matrix[i][j] = A[i][j] * conditional_var + 
                                                   B[(i + j) % N][j] * (1.0f / conditional_var);
                            }
                        }
                    }
                }
            }
        }
        
        /* Final reduction to prevent dead code elimination */
        float checksum = 0.0f;
        #pragma acc parallel loop reduction(+:checksum) \
                copyin(temp_matrix[0:N][0:M]) copyout(checksum)
        for (int i = 0; i < N; i++) {
            #pragma acc loop reduction(+:checksum)
            for (int j = 0; j < M; j++) {
                checksum += temp_matrix[i][j];
            }
        }
        
        printf("Final checksum: %f\n", checksum);
    }
    
    return 0;
}
