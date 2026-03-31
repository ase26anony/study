/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_gang_redundant = 42;           /* Should become gang redundant (case 0) */
float global_matrix[N][M];                /* Multi-dimensional array */
static int module_level_var = 100;

/* Function with complex partitioning scenarios */
void test_partitioning_states() {
    int i, j, k;
    
    /* Initialize data */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j);
        }
    }
    
    /* ========== CASE 0: Gang Redundant ========== */
    /* Variable accessible by all gangs without partitioning */
    #pragma acc parallel num_gangs(GANGS) copy(global_gang_redundant)
    {
        /* global_gang_redundant should be marked as gang redundant */
        int local_gang_id = __pgi_gangidx();
        if (local_gang_id == 0) {
            global_gang_redundant += 1;
        }
    }
    
    /* ========== CASE 1: Gang Partitioned ========== */
    /* Scalar private to each gang */
    #pragma acc parallel num_gangs(GANGS) copyout(global_matrix[0:N][0:M])
    {
        int gang_private = __pgi_gangidx();  /* Should be gang partitioned */
        
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop vector independent
            for (j = 0; j < M; j++) {
                global_matrix[i][j] += (float)gang_private;
            }
        }
    }
    
    /* ========== CASE 2: Worker Partitioned ========== */
    /* Using worker-only parallelism */
    #pragma acc parallel num_workers(WORKERS) vector_length(1) \
                copy(global_matrix[0:N][0:M])
    {
        int worker_private = __pgi_workeridx();  /* Should be worker partitioned */
        
        #pragma acc loop worker independent
        for (i = 0; i < N; i++) {
            global_matrix[i][0] += (float)worker_private;
        }
    }
    
    /* ========== CASE 3: Gang+Worker Partitioned ========== */
    /* Nested gang-worker parallelism with mixed attributes */
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(VECTOR_LEN) \
                copy(global_matrix[0:N][0:M])
    {
        int gw_partitioned = __pgi_gangidx() * 10 + __pgi_workeridx();
        int gang_only = __pgi_gangidx();
        int worker_only = __pgi_workeridx();
        
        /* Complex access pattern to force partitioning analysis */
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop worker independent
            for (j = 0; j < M; j++) {
                if ((i + j) % 2 == 0) {
                    global_matrix[i][j] += (float)gw_partitioned;
                } else {
                    global_matrix[i][j] += (float)(gang_only + worker_only);
                }
            }
        }
    }
    
    /* ========== CASE 4: Vector Partitioned ========== */
    /* Vector-only parallelism */
    #pragma acc kernels copy(global_matrix[0:N][0:M])
    {
        #pragma acc loop independent
        for (i = 0; i < N; i++) {
            float vector_private[M];  /* Should be vector partitioned */
            
            #pragma acc loop vector independent
            for (j = 0; j < M; j++) {
                vector_private[j] = global_matrix[i][j] * 2.0f;
            }
            
            #pragma acc loop vector independent
            for (j = 0; j < M; j++) {
                global_matrix[i][j] = vector_private[j];
            }
        }
    }
    
    /* ========== CASE 5: Gang+Vector Partitioned ========== */
    /* Gang-vector parallelism without workers */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
                copy(global_matrix[0:N][0:M])
    {
        int gv_partitioned = __pgi_gangidx() * 100 + __pgi_vectoridx();
        
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop vector independent
            for (j = 0; j < M; j++) {
                /* Stencil computation complicates analysis */
                float left = (j > 0) ? global_matrix[i][j-1] : 0.0f;
                float right = (j < M-1) ? global_matrix[i][j+1] : 0.0f;
                global_matrix[i][j] = (left + right) / 2.0f + gv_partitioned;
            }
        }
    }
    
    /* ========== CASE 6: Worker+Vector Partitioned ========== */
    /* Worker-vector parallelism */
    #pragma acc parallel num_workers(WORKERS) vector_length(VECTOR_LEN) \
                copy(global_matrix[0:N][0:M])
    {
        int wv_partitioned = __pgi_workeridx() * 50 + __pgi_vectoridx();
        int shared_across_workers = 0;  /* Will have different partitioning */
        
        #pragma acc loop worker independent
        for (i = 0; i < N; i++) {
            #pragma acc loop vector independent private(shared_across_workers)
            for (j = 0; j < M; j++) {
                shared_across_workers = (i + j) % 256;
                global_matrix[i][j] += (float)(wv_partitioned + shared_across_workers);
            }
        }
    }
    
    /* ========== CASE 7: Fully Partitioned ========== */
    /* Full gang-worker-vector parallelism with complex data flow */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
                copy(global_matrix[0:N][0:M])
    {
        int fully_partitioned = __pgi_gangidx() * 1000 + 
                               __pgi_workeridx() * 100 + 
                               __pgi_vectoridx();
        int temp_buffer[16];  /* Local buffer with complex partitioning */
        
        /* Initialize temp buffer */
        #pragma acc loop vector independent
        for (k = 0; k < 16; k++) {
            temp_buffer[k] = fully_partitioned + k;
        }
        
        /* Complex nested loops with conditional partitioning */
        #pragma acc loop gang independent
        for (i = 0; i < N; i += 2) {
            #pragma acc loop worker independent
            for (j = 0; j < M; j += 2) {
                #pragma acc loop vector independent
                for (k = 0; k < 4; k++) {
                    int idx_i = i + (k / 2);
                    int idx_j = j + (k % 2);
                    if (idx_i < N && idx_j < M) {
                        /* Multi-dimensional stencil access */
                        float sum = 0.0f;
                        int neighbors = 0;
                        
                        for (int di = -1; di <= 1; di++) {
                            for (int dj = -1; dj <= 1; dj++) {
                                int ni = idx_i + di;
                                int nj = idx_j + dj;
                                if (ni >= 0 && ni < N && nj >= 0 && nj < M) {
                                    sum += global_matrix[ni][nj];
                                    neighbors++;
                                }
                            }
                        }
                        
                        if (neighbors > 0) {
                            global_matrix[idx_i][idx_j] = sum / neighbors + 
                                                         temp_buffer[k % 16] +
                                                         module_level_var;
                        }
                    }
                }
            }
        }
    }
    
    /* Conditional partitioning based on runtime values */
    int dynamic_size = global_gang_redundant % 128;
    float* dynamic_array = (float*)malloc(dynamic_size * sizeof(float));
    
    #pragma acc enter data create(dynamic_array[0:dynamic_size])
    
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(8) \
                present(dynamic_array[0:dynamic_size])
    {
        int conditional_partition;
        if (__pgi_gangidx() == 0) {
            conditional_partition = 1;  /* Different partitioning per condition */
        } else {
            conditional_partition = 2;
        }
        
        #pragma acc loop gang worker vector independent
        for (i = 0; i < dynamic_size; i++) {
            dynamic_array[i] = (float)(i * conditional_partition);
        }
    }
    
    #pragma acc exit data copyout(dynamic_array[0:dynamic_size]) delete(dynamic_array[0:dynamic_size])
    
    free(dynamic_array);
}

/* Additional function with different parallel constructs */
void test_kernels_directive() {
    int i, j;
    float local_matrix[256][256];
    float reduction_var = 0.0f;
    
    /* Initialize */
    for (i = 0; i < 256; i++) {
        for (j = 0; j < 256; j++) {
            local_matrix[i][j] = (float)(i * j);
        }
    }
    
    /* kernels directive with explicit loop modifiers */
    #pragma acc kernels copy(local_matrix) copyout(reduction_var)
    {
        float gang_private[256];  /* Will have different partitioning in different loops */
        
        #pragma acc loop gang independent
        for (i = 0; i < 256; i++) {
            gang_private[i] = 0.0f;
            
            #pragma acc loop worker vector reduction(+:gang_private[i])
            for (j = 0; j < 256; j++) {
                gang_private[i] += local_matrix[i][j];
            }
        }
        
        #pragma acc loop gang worker independent reduction(+:reduction_var)
        for (i = 0; i < 256; i++) {
            reduction_var += gang_private[i];
        }
    }
    
    printf("Reduction result: %f\n", reduction_var);
}

int main() {
    printf("Testing OpenACC partitioning states...\n");
    
    /* Call functions with different parallel patterns */
    test_partitioning_states();
    test_kernels_directive();
    
    /* Compute checksum to verify execution */
    float checksum = 0.0f;
    #pragma acc parallel loop reduction(+:checksum) copyin(global_matrix)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += global_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Global gang redundant value: %d\n", global_gang_redundant);
    
    return 0;
}
