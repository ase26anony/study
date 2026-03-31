/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables to create complex scoping */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Base for multi-dimensional access */
static double static_partial[M];         /* Static storage for worker partitioning */

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.001f;
        }
    }
    for (int j = 0; j < M; j++) {
        static_partial[j] = 0.0;
    }
}

/* Main function with complex nested parallelism */
int main() {
    int result = 0;
    float local_matrix[N][M];
    float temp_buffer[N];
    int gang_private_scalar;            /* Should become gang partitioned (case 1) */
    
    init_data();
    
    /* Copy global to local for modification */
    #pragma acc kernels copyin(global_matrix) copyout(local_matrix)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                local_matrix[i][j] = global_matrix[i][j];
            }
        }
    }
    
    /* CASE 0: Gang Redundant variable */
    /* Global variable accessed by all gangs without partitioning */
    #pragma acc parallel num_gangs(GANGS) copy(result) present(global_matrix)
    {
        int gang_id = __pgi_gangidx();
        /* global_gang_redundant is the same for all gangs */
        if (gang_id == 0) {
            result += global_gang_redundant;
        }
    }
    
    /* CASE 1: Gang Partitioned */
    /* Each gang gets its own copy */
    #pragma acc parallel num_gangs(GANGS) private(gang_private_scalar) copy(result)
    {
        gang_private_scalar = __pgi_gangidx() * 10;
        #pragma acc atomic
        result += gang_private_scalar;
    }
    
    /* CASE 2: Worker Partitioned */
    /* Using kernels with worker-level parallelism */
    #pragma acc kernels copyout(temp_buffer)
    {
        float worker_local[M];  /* Should be worker partitioned */
        
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            float sum = 0.0f;
            #pragma acc loop vector reduction(+:sum)
            for (int j = 0; j < M; j++) {
                sum += local_matrix[i][j];
            }
            worker_local[i % M] = sum;  /* Worker-specific array */
            temp_buffer[i] = sum;
        }
    }
    
    /* CASE 3: Gang+Worker Partitioned */
    /* Complex nested region with explicit clauses */
    #pragma acc parallel num_gangs(2) num_workers(4) vector_length(32) \
                copy(result) copyin(local_matrix)
    {
        int gw_partitioned[16];  /* Should be gang+worker partitioned */
        
        #pragma acc loop gang worker
        for (int g = 0; g < 2; g++) {
            for (int w = 0; w < 4; w++) {
                int idx = g * 4 + w;
                gw_partitioned[idx] = g * 100 + w * 10;
                
                /* Multi-dimensional stencil access */
                #pragma acc loop vector
                for (int v = 0; v < 32; v++) {
                    int i = (g * 4 + w) * 8 + (v / 4);
                    int j = (v % 4) * 128;
                    if (i < N-1 && j < M-1) {
                        /* Stencil computation */
                        float val = local_matrix[i][j] + 
                                   local_matrix[i+1][j] + 
                                   local_matrix[i][j+1];
                        if (val > 0.5f) {
                            gw_partitioned[idx] += 1;
                        }
                    }
                }
            }
        }
        
        /* Reduction across workers */
        #pragma acc atomic
        result += gw_partitioned[0];
    }
    
    /* CASE 4: Vector Partitioned */
    /* Vector-only parallelism */
    #pragma acc parallel vector_length(VECTOR_LEN) copy(result)
    {
        int vector_private[VECTOR_LEN];  /* Should be vector partitioned */
        
        #pragma acc loop vector
        for (int v = 0; v < VECTOR_LEN; v++) {
            vector_private[v] = v * 3;
        }
        
        /* Conditional partitioning based on runtime values */
        int cond = (result > 100) ? 1 : 0;
        if (cond) {
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                vector_private[v] *= 2;
            }
        }
        
        #pragma acc atomic
        result += vector_private[VECTOR_LEN-1];
    }
    
    /* CASE 5: Gang+Vector Partitioned */
    /* Two-level partitioning without workers */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
                copy(result) copyin(local_matrix)
    {
        float gv_array[GANGS][VECTOR_LEN];  /* Should be gang+vector partitioned */
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                int i = g * (N/GANGS) + (v % (N/GANGS));
                int j = v * (M/VECTOR_LEN);
                if (i < N && j < M) {
                    gv_array[g][v] = local_matrix[i][j] * 2.0f;
                } else {
                    gv_array[g][v] = 0.0f;
                }
            }
        }
        
        #pragma acc atomic
        result += (int)gv_array[0][0];
    }
    
    /* CASE 6: Worker+Vector Partitioned */
    /* Using parallel construct with workers and vectors but no gang partitioning */
    #pragma acc parallel num_workers(4) vector_length(16) \
                copy(result) copyin(local_matrix)
    {
        double wv_accumulator[4][16];  /* Should be worker+vector partitioned */
        
        #pragma acc loop worker
        for (int w = 0; w < 4; w++) {
            #pragma acc loop vector
            for (int v = 0; v < 16; v++) {
                wv_accumulator[w][v] = 0.0;
                for (int i = w * 64; i < (w+1) * 64 && i < N; i++) {
                    wv_accumulator[w][v] += local_matrix[i][v * 32];
                }
            }
        }
        
        /* Access static variable - adds complexity */
        #pragma acc loop worker
        for (int w = 0; w < 4; w++) {
            static_partial[w % M] += wv_accumulator[w][0];
        }
        
        result += (int)wv_accumulator[0][0];
    }
    
    /* CASE 7: Fully Partitioned (Gang+Worker+Vector) */
    /* Most complex case - three levels of partitioning */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(8) \
                copy(result) copyin(local_matrix)
    {
        /* Fully partitioned 3D array */
        int fully_partitioned[2][2][8];  /* Should be fully partitioned */
        
        #pragma acc loop gang
        for (int g = 0; g < 2; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 2; w++) {
                #pragma acc loop vector
                for (int v = 0; v < 8; v++) {
                    fully_partitioned[g][w][v] = g * 1000 + w * 100 + v * 10;
                    
                    /* Complex multi-dimensional access pattern */
                    int i = (g * 2 + w) * 128 + v * 8;
                    int j = v * 64;
                    if (i < N-2 && j < M-2) {
                        /* 3x3 stencil access */
                        float stencil_sum = 
                            local_matrix[i-1][j-1] + local_matrix[i-1][j] + local_matrix[i-1][j+1] +
                            local_matrix[i][j-1]   + local_matrix[i][j]   + local_matrix[i][j+1] +
                            local_matrix[i+1][j-1] + local_matrix[i+1][j] + local_matrix[i+1][j+1];
                        
                        fully_partitioned[g][w][v] += (int)(stencil_sum * 100.0f);
                    }
                }
            }
        }
        
        /* Mixed sharing attributes within same region */
        int shared_across_vectors[8];  /* Shared across vectors within same worker */
        #pragma acc loop worker
        for (int w = 0; w < 2; w++) {
            #pragma acc loop vector
            for (int v = 0; v < 8; v++) {
                shared_across_vectors[v] = fully_partitioned[0][w][v];
            }
            
            /* Reduction within worker */
            int worker_sum = 0;
            #pragma acc loop vector reduction(+:worker_sum)
            for (int v = 0; v < 8; v++) {
                worker_sum += shared_across_vectors[v];
            }
            
            #pragma acc atomic
            result += worker_sum;
        }
    }
    
    /* Final verification computation */
    float final_sum = 0.0f;
    #pragma acc kernels copyin(local_matrix) copyout(final_sum)
    {
        #pragma acc loop gang reduction(+:final_sum)
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector reduction(+:final_sum)
            for (int j = 0; j < M; j++) {
                final_sum += local_matrix[i][j];
            }
        }
    }
    
    printf("Result: %d\n", result);
    printf("Final sum: %f\n", final_sum);
    
    return 0;
}
