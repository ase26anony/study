/* test_omp_acc_partitioning.c
 * Designed to exercise GCC's OpenACC variable partitioning analysis
 * Compile with: gcc -O1 -fopenacc -fdump-tree-omplower -c test_omp_acc_partitioning.c
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables for different scoping */
int global_gang_redundant = 42;          /* Should become "gang redundant" (case 0) */
float global_matrix[N][M];               /* Multi-dimensional array */
static int module_scalar = 100;

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    int host_scalar = 10;
    float host_array[N];
    int result = 0;
    
    /* Initialize global data */
    init_data();
    
    /* 1. GANG-ONLY PARALLEL REGION (targeting case 1: "gang partitioned") */
    #pragma acc parallel num_gangs(GANGS) copyout(host_array[0:N])
    {
        int gang_private = 0;  /* Should be gang partitioned */
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            gang_private = i % 256;
            host_array[i] = global_matrix[i][0] + gang_private;
        }
    }
    
    /* 2. VECTOR-ONLY LOOP in KERNELS region (targeting case 4: "vector partitioned") */
    #pragma acc kernels copyin(global_matrix) copyout(host_array[0:N])
    {
        float vector_local = 3.14f;  /* May become vector partitioned */
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            vector_local = (float)i * 0.5f;
            #pragma acc loop seq
            for (int j = 0; j < 16; j++) {
                host_array[i] = global_matrix[i][j % M] + vector_local;
            }
        }
    }
    
    /* 3. COMPLEX NESTED PARALLELISM with all clause combinations */
    {
        int outer_scalar = 777;  /* Mixed attributes */
        float temp_matrix[64][64];
        
        /* Initialize temp matrix */
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                temp_matrix[i][j] = (float)(i + j);
            }
        }
        
        /* Target combined partitioning states (cases 2,3,5,6,7) */
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
                    copyin(temp_matrix) copy(result)
        {
            int gang_worker_partitioned = 0;      /* Should become gang+worker partitioned (case 3) */
            float worker_vector_partitioned = 0.0f; /* Should become worker+vector partitioned (case 6) */
            int fully_partitioned = 0;            /* Should become fully partitioned (case 7) */
            
            /* Conditional partitioning based on runtime values */
            int conditional_var = (global_gang_redundant > 0) ? 1 : 0;
            
            #pragma acc loop gang
            for (int g = 0; g < GANGS; g++) {
                gang_worker_partitioned = g * 100;
                
                #pragma acc loop worker
                for (int w = 0; w < WORKERS; w++) {
                    worker_vector_partitioned = (float)w * 2.5f;
                    
                    #pragma acc loop vector
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        fully_partitioned = g * 1000 + w * 100 + v;
                        
                        /* Complex stencil computation on multi-dimensional array */
                        int idx = (g * WORKERS * VECTOR_LEN + w * VECTOR_LEN + v) % 64;
                        if (idx > 0 && idx < 63) {
                            float stencil_sum = 
                                temp_matrix[idx-1][v % 64] +
                                temp_matrix[idx][v % 64] * 2.0f +
                                temp_matrix[idx+1][v % 64];
                            
                            /* Mix in conditional partitioning */
                            if (conditional_var) {
                                result += (int)(stencil_sum * 0.01f);
                            } else {
                                result -= (int)(stencil_sum * 0.01f);
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* 4. GANG+WORKER PARTITIONING (targeting case 3 explicitly) */
    {
        int gw_scalar = 0;
        #pragma acc parallel num_gangs(4) num_workers(8) copy(gw_scalar)
        {
            int gang_worker_var = 0;  /* Explicit gang+worker partitioned */
            #pragma acc loop gang worker
            for (int i = 0; i < 32; i++) {
                gang_worker_var = i;
                gw_scalar += gang_worker_var;
            }
        }
    }
    
    /* 5. GANG+VECTOR PARTITIONING (targeting case 5) */
    {
        float gv_array[128];
        #pragma acc parallel num_gangs(2) vector_length(64) copy(gv_array[0:128])
        {
            float gang_vector_var = 0.0f;  /* Should become gang+vector partitioned */
            #pragma acc loop gang vector
            for (int i = 0; i < 128; i++) {
                gang_vector_var = (float)i * 0.1f;
                gv_array[i] = gang_vector_var;
            }
        }
    }
    
    /* 6. WORKER+VECTOR PARTITIONING (targeting case 6) */
    {
        int wv_result = 0;
        #pragma acc parallel num_workers(4) vector_length(16) copy(wv_result)
        {
            int worker_vector_var = 0;  /* Should become worker+vector partitioned */
            #pragma acc loop worker vector
            for (int i = 0; i < 64; i++) {
                worker_vector_var = i * 2;
                wv_result += worker_vector_var;
            }
        }
    }
    
    /* 7. WORKER-ONLY PARTITIONING (targeting case 2) */
    {
        float worker_only[256];
        #pragma acc parallel num_workers(8) copy(worker_only[0:256])
        {
            float worker_partitioned = 0.0f;  /* Should become worker partitioned */
            #pragma acc loop worker
            for (int i = 0; i < 256; i++) {
                worker_partitioned = (float)i * 3.14f;
                worker_only[i] = worker_partitioned;
            }
        }
    }
    
    /* Final computation using global_gang_redundant (targeting case 0) */
    int final_sum = 0;
    #pragma acc parallel num_gangs(2) copyin(global_gang_redundant) copy(final_sum)
    {
        /* global_gang_redundant should be marked as gang redundant */
        #pragma acc loop gang
        for (int i = 0; i < 100; i++) {
            final_sum += global_gang_redundant + i;
        }
    }
    
    printf("Result checksum: %d\n", result + final_sum);
    return 0;
}
