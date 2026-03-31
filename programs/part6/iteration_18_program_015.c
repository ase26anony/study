/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables that can be partitioned in various ways */
int global_scalar = 42;
float global_array[N][M];
int global_reduction = 0;

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_array[i][j] = (float)(i * M + j);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    int host_scalar = 100;
    float host_array[N][M];
    int result = 0;
    
    /* Initialize test data */
    init_data();
    memcpy(host_array, global_array, sizeof(global_array));
    
    printf("Starting OpenACC/OpenMP partitioning tests...\n");
    
    /* ============================================
       CASE 0: Gang Redundant
       A variable that is the same for all gangs
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(global_scalar) \
        copyin(global_array) copyout(host_array[0:N][0:M])
    {
        /* global_scalar should be gang redundant (case 0) */
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Each gang processes its own chunk */
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                /* global_scalar is read-only and same for all gangs */
                host_array[i][j] = global_array[i][j] + global_scalar;
            }
        }
    }
    
    /* ============================================
       CASE 1: Gang Partitioned
       Variables private to each gang
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) private(host_scalar) \
        copy(host_array[0:N][0:M])
    {
        /* host_scalar is private to each gang (case 1) */
        int gang_private = __pgi_gangidx() * 1000;
        
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            int gang_local = gang_private + i;  /* Gang partitioned */
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                host_array[i][j] += gang_local;
            }
        }
    }
    
    /* ============================================
       CASE 2: Worker Partitioned  
       Variables private to each worker within a gang
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copy(host_array[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            int worker_partitioned;  /* Will be worker partitioned (case 2) */
            
            #pragma acc loop worker independent
            for (int j = 0; j < M; j += WORKERS) {
                worker_partitioned = __pgi_workeridx() * 100;
                
                #pragma acc loop vector
                for (int k = 0; k < WORKERS && (j + k) < M; k++) {
                    host_array[i][j + k] += worker_partitioned + k;
                }
            }
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       Variables partitioned across both gangs and workers
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copy(host_array[0:N][0:M])
    {
        #pragma acc loop gang worker independent
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                /* Complex indexing creates gang+worker partitioning (case 3) */
                int gw_partitioned = (__pgi_gangidx() * WORKERS + __pgi_workeridx()) * 10;
                host_array[i][j] += gw_partitioned;
            }
        }
    }
    
    /* ============================================
       CASE 4: Vector Partitioned
       Variables private to each vector lane
       ============================================ */
    #pragma acc parallel vector_length(VECTOR_LEN) \
        copy(host_array[0:N][0:M])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector independent
            for (int j = 0; j < M; j++) {
                /* vector_id creates vector partitioning (case 4) */
                int vector_private = __pgi_vectoridx();
                host_array[i][j] += vector_private;
            }
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
        copy(host_array[0:N][0:M])
    {
        #pragma acc loop gang vector independent
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                /* Combined gang and vector partitioning (case 5) */
                int gv_partitioned = __pgi_gangidx() * VECTOR_LEN + __pgi_vectoridx();
                host_array[i][j] += gv_partitioned;
            }
        }
    }
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       ============================================ */
    #pragma acc parallel num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(host_array[0:N][0:M])
    {
        #pragma acc loop worker vector independent
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                /* Worker+vector partitioning (case 6) */
                int wv_partitioned = __pgi_workeridx() * VECTOR_LEN + __pgi_vectoridx();
                host_array[i][j] += wv_partitioned;
            }
        }
    }
    
    /* ============================================
       CASE 7: Fully Partitioned (Gang+Worker+Vector)
       Most complex partitioning scenario
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        vector_length(VECTOR_LEN) copy(host_array[0:N][0:M])
    {
        /* Create a fully partitioned variable (case 7) */
        int fully_partitioned = __pgi_gangidx() * (WORKERS * VECTOR_LEN) +
                               __pgi_workeridx() * VECTOR_LEN +
                               __pgi_vectoridx();
        
        #pragma acc loop gang worker vector independent collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                /* Stencil computation with neighbor access */
                float left = (j > 0) ? host_array[i][j-1] : 0;
                float right = (j < M-1) ? host_array[i][j+1] : 0;
                float up = (i > 0) ? host_array[i-1][j] : 0;
                float down = (i < N-1) ? host_array[i+1][j] : 0;
                
                host_array[i][j] = (host_array[i][j] + left + right + up + down) / 5.0f;
                host_array[i][j] += fully_partitioned;
            }
        }
    }
    
    /* ============================================
       Mixed constructs to force complex analysis
       ============================================ */
    #pragma acc kernels copy(host_array[0:N][0:M])
    {
        int kernel_scalar = 777;  /* Will have different partitioning in kernels */
        
        #pragma acc loop gang(static:1)
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker(4) vector(16)
            for (int j = 0; j < M; j++) {
                /* Conditional partitioning */
                if (host_array[i][j] > 1000.0f) {
                    kernel_scalar = 1;  /* Different flow paths */
                } else {
                    kernel_scalar = 2;
                }
                host_array[i][j] += kernel_scalar;
            }
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    #pragma acc parallel num_gangs(GANGS) copy(host_array[0:N][0:M]) \
        copyout(result)
    {
        int local_sum = 0;
        
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector reduction(+:local_sum)
            for (int j = 0; j < M; j++) {
                local_sum += (int)host_array[i][j];
            }
        }
        
        #pragma acc atomic update
        result += local_sum;
    }
    
    printf("Final checksum: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
