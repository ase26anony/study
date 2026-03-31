/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

/* Global variables for different scoping */
int global_gang_redundant = 42;
float global_matrix[N][M];
static double static_partitioned[P];

/* Function to trigger complex partitioning analysis */
void test_partitioning_states() {
    int i, j, k;
    
    /* Initialize test data */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i + j);
        }
    }
    
    for (k = 0; k < P; k++) {
        static_partitioned[k] = k * 1.5;
    }
    
    /* ============================================
       CASE 0: "gang redundant"
       Global variable accessed read-only by all gangs
       ============================================ */
    #pragma acc parallel num_gangs(4) copyout(global_matrix[0:N][0:M])
    {
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            /* global_gang_redundant is read-only, should be gang redundant */
            float base = global_gang_redundant;
            #pragma acc loop worker vector
            for (j = 0; j < M; j++) {
                global_matrix[i][j] += base;
            }
        }
    }
    
    /* ============================================
       CASE 1: "gang partitioned"
       Variable private to each gang
       ============================================ */
    int gang_private_var = 100;
    #pragma acc parallel num_gangs(8) private(gang_private_var) \
        copy(global_matrix[0:N][0:M])
    {
        gang_private_var = __pgi_gangidx() * 10;
        
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            int worker_temp = 0;
            #pragma acc loop worker reduction(+:worker_temp)
            for (j = 0; j < M; j++) {
                worker_temp += (int)global_matrix[i][j];
            }
            global_matrix[i][0] = gang_private_var + worker_temp;
        }
    }
    
    /* ============================================
       CASE 2: "worker partitioned" 
       CASE 4: "vector partitioned"
       Using kernels with explicit loop directives
       ============================================ */
    float worker_partitioned[M];
    int vector_partitioned[N];
    
    #pragma acc kernels copyout(worker_partitioned[0:M], vector_partitioned[0:N])
    {
        /* Worker partitioned array */
        #pragma acc loop gang worker
        for (j = 0; j < M; j++) {
            worker_partitioned[j] = j * 2.0f;
        }
        
        /* Vector partitioned array */
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            vector_partitioned[i] = i * 3;
        }
    }
    
    /* ============================================
       CASE 3: "gang+worker partitioned"
       Nested parallelism with gang and worker private vars
       ============================================ */
    int gw_partitioned[N][M];
    
    #pragma acc parallel num_gangs(4) num_workers(8) vector_length(32) \
        copyout(gw_partitioned[0:N][0:M])
    {
        int gang_worker_private;
        
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            gang_worker_private = i * 100;
            
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                int worker_local = gang_worker_private + j;
                
                /* This creates gang+worker partitioning */
                gw_partitioned[i][j] = worker_local;
                
                #pragma acc loop vector
                for (k = 0; k < 4; k++) {
                    gw_partitioned[i][j] += k;
                }
            }
        }
    }
    
    /* ============================================
       CASE 5: "gang+vector partitioned"
       ============================================ */
    float gv_partitioned[N][32];
    
    #pragma acc parallel num_gangs(8) vector_length(32) \
        copyout(gv_partitioned[0:N][0:32])
    {
        #pragma acc loop gang vector
        for (i = 0; i < N; i++) {
            for (k = 0; k < 32; k++) {
                gv_partitioned[i][k] = __pgi_gangidx() * 100.0f + 
                                      __pgi_vectoridx() * 1.0f;
            }
        }
    }
    
    /* ============================================
       CASE 6: "worker+vector partitioned"
       ============================================ */
    int wv_partitioned[16][64];
    
    #pragma acc parallel num_workers(16) vector_length(64) \
        copyout(wv_partitioned[0:16][0:64])
    {
        #pragma acc loop worker vector
        for (j = 0; j < 16; j++) {
            for (k = 0; k < 64; k++) {
                wv_partitioned[j][k] = __pgi_workeridx() * 1000 + 
                                      __pgi_vectoridx() * 10;
            }
        }
    }
    
    /* ============================================
       CASE 7: "fully partitioned"
       All three levels: gang, worker, and vector
       ============================================ */
    int fully_partitioned[8][16][32];
    
    #pragma acc parallel num_gangs(8) num_workers(16) vector_length(32) \
        copyout(fully_partitioned[0:8][0:16][0:32])
    {
        #pragma acc loop gang
        for (int g = 0; g < 8; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 16; w++) {
                #pragma acc loop vector
                for (int v = 0; v < 32; v++) {
                    fully_partitioned[g][w][v] = 
                        g * 10000 + w * 100 + v;
                }
            }
        }
    }
    
    /* ============================================
       Complex conditional partitioning
       Runtime-dependent partitioning decisions
       ============================================ */
    int conditional_matrix[N][M];
    int threshold = 50;
    
    #pragma acc parallel num_gangs(4) num_workers(8) vector_length(16) \
        copyout(conditional_matrix[0:N][0:M]) copyin(threshold)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            int gang_specific = i * 10;
            
            #pragma acc loop worker
            for (j = 0; j < M; j++) {
                int partitioning_type;
                
                /* Conditional creates different partitioning paths */
                if (j < threshold) {
                    partitioning_type = 1;  /* gang partitioned */
                } else {
                    partitioning_type = 3;  /* gang+worker partitioned */
                }
                
                #pragma acc loop vector
                for (k = 0; k < 8; k++) {
                    conditional_matrix[i][j] = 
                        gang_specific + j + k + partitioning_type;
                }
            }
        }
    }
    
    /* ============================================
       Multi-dimensional stencil computation
       Complicates dependency analysis
       ============================================ */
    float stencil_input[N][M];
    float stencil_output[N][M];
    
    /* Initialize stencil input */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            stencil_input[i][j] = (float)(i * M + j);
        }
    }
    
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(stencil_input[0:N][0:M]) copyout(stencil_output[0:N][0:M])
    {
        #pragma acc loop gang
        for (i = 1; i < N-1; i++) {
            #pragma acc loop worker
            for (j = 1; j < M-1; j++) {
                float sum = 0.0f;
                
                /* 5-point stencil - creates complex data access pattern */
                #pragma acc loop vector reduction(+:sum)
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        if (di == 0 && dj == 0) continue;
                        sum += stencil_input[i+di][j+dj];
                    }
                }
                
                stencil_output[i][j] = sum / 8.0f;
            }
        }
    }
    
    /* ============================================
       Reduction with mixed partitioning
       ============================================ */
    double total_sum = 0.0;
    float partial_sums[8][16];  /* gang x worker partial sums */
    
    #pragma acc parallel num_gangs(8) num_workers(16) \
        copyin(global_matrix[0:N][0:M]) copyout(partial_sums[0:8][0:16]) \
        reduction(+:total_sum)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        partial_sums[gang_id][worker_id] = 0.0f;
        
        #pragma acc loop gang worker
        for (i = 0; i < N; i++) {
            float row_sum = 0.0f;
            
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                row_sum += global_matrix[i][j];
            }
            
            partial_sums[gang_id][worker_id] += row_sum;
        }
        
        total_sum += partial_sums[gang_id][worker_id];
    }
    
    /* Print verification output */
    printf("Verification output:\n");
    printf("Total sum: %f\n", total_sum);
    printf("Sample values:\n");
    printf("  global_matrix[10][10] = %f\n", global_matrix[10][10]);
    printf("  gw_partitioned[5][5] = %d\n", gw_partitioned[5][5]);
    printf("  fully_partitioned[2][3][4] = %d\n", fully_partitioned[2][3][4]);
    printf("  stencil_output[20][20] = %f\n", stencil_output[20][20]);
}

int main() {
    printf("Testing OpenACC partitioning states...\n");
    
    test_partitioning_states();
    
    /* Additional OpenMP offloading test for completeness */
    #ifdef _OPENMP
    {
        int omp_array[100];
        
        #pragma omp target teams distribute parallel for \
            map(tofrom: omp_array[0:100])
        for (int i = 0; i < 100; i++) {
            omp_array[i] = i * 2;
        }
        
        printf("OpenMP offloading test completed.\n");
    }
    #endif
    
    printf("Test completed successfully.\n");
    return 0;
}
