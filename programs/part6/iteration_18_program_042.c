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
float global_matrix[N][M];               /* Base for complex partitioning */
static int file_static_counter = 0;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.001f;
        }
    }
}

/* Main function with complex nested parallelism */
int main() {
    /* Local variables with different scopes */
    int host_scalar = 100;
    float host_array[N];
    double host_matrix[N][M];
    
    /* Initialize data */
    init_matrices();
    for (int i = 0; i < N; i++) {
        host_array[i] = sinf(i * 0.01f);
        for (int j = 0; j < M; j++) {
            host_matrix[i][j] = cosf(i * j * 0.0001f);
        }
    }
    
    /* ==================== CASE 0: Gang Redundant ==================== */
    /* Variable accessible by all gangs without partitioning */
    #pragma acc parallel num_gangs(GANGS) copy(global_gang_redundant)
    {
        /* global_gang_redundant should be marked as gang redundant */
        int gang_id = __pgi_gangidx();
        if (gang_id == 0) {
            global_gang_redundant += 1;
        }
    }
    printf("After gang redundant region: %d\n", global_gang_redundant);
    
    /* ==================== CASE 1: Gang Partitioned ==================== */
    /* Scalar private to each gang */
    #pragma acc parallel num_gangs(GANGS) \
        copyin(host_scalar) copyout(host_array[0:N])
    {
        int gang_private = host_scalar + __pgi_gangidx();  /* Gang partitioned */
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            host_array[i] += gang_private;
        }
    }
    
    /* ==================== CASE 2: Worker Partitioned ==================== */
    /* Variable partitioned across workers within a gang */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) \
        copy(host_array[0:N])
    {
        int worker_local = __pgi_workeridx() * 10;  /* Worker partitioned */
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            if (i % WORKERS == __pgi_workeridx()) {
                host_array[i] += worker_local;
            }
        }
    }
    
    /* ==================== CASE 3: Gang+Worker Partitioned ==================== */
    /* Complex nesting with gang and worker parallelism */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copy(global_matrix[0:N][0:M])
    {
        /* This variable should be partitioned across both gangs and workers */
        float gw_partitioned[16];  /* Small array for gang+worker partitioning */
        int gw_idx = __pgi_gangidx() * WORKERS + __pgi_workeridx();
        
        for (int k = 0; k < 16; k++) {
            gw_partitioned[k] = gw_idx * 0.1f + k;
        }
        
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                int idx = (i + j) % 16;
                global_matrix[i][j] += gw_partitioned[idx];
            }
        }
    }
    
    /* ==================== CASE 4: Vector Partitioned ==================== */
    /* Vector-only parallelism */
    #pragma acc kernels copy(host_matrix[0:N][0:M])
    {
        float vector_private;  /* Vector partitioned variable */
        
        #pragma acc loop independent vector(VECTOR_LEN)
        for (int i = 0; i < N; i++) {
            vector_private = __pgi_vectoridx() * 0.01f;  /* Different per vector lane */
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                host_matrix[i][j] += vector_private + i * 0.001f + j * 0.0001f;
            }
        }
    }
    
    /* ==================== CASE 5: Gang+Vector Partitioned ==================== */
    /* Combined gang and vector parallelism */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
        copy(global_matrix[0:N][0:M])
    {
        /* Variable partitioned across gangs and vector lanes */
        double gv_partitioned;  /* Gang+vector partitioned */
        gv_partitioned = __pgi_gangidx() * 100.0 + __pgi_vectoridx();
        
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if ((i * M + j) % VECTOR_LEN == __pgi_vectoridx()) {
                    global_matrix[i][j] += gv_partitioned * 0.001f;
                }
            }
        }
    }
    
    /* ==================== CASE 6: Worker+Vector Partitioned ==================== */
    /* Worker and vector parallelism without gang partitioning */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(host_matrix[0:N][0:M])
    {
        /* Variable partitioned across workers and vector lanes */
        int wv_partitioned[8];  /* Worker+vector partitioned array */
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        for (int k = 0; k < 8; k++) {
            wv_partitioned[k] = worker_id * 1000 + vector_id * 100 + k;
        }
        
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            int idx = (i / VECTOR_LEN) % 8;
            host_matrix[i][i % M] += wv_partitioned[idx] * 0.0001f;
        }
    }
    
    /* ==================== CASE 7: Fully Partitioned ==================== */
    /* All three levels of parallelism with complex data access */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(global_matrix[0:N][0:M], host_matrix[0:N][0:M])
    {
        /* Fully partitioned variable - different for each gang, worker, and vector */
        float fully_partitioned[4][4];  /* Small 2D array fully partitioned */
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Initialize with unique values per parallel dimension */
        for (int x = 0; x < 4; x++) {
            for (int y = 0; y < 4; y++) {
                fully_partitioned[x][y] = gang_id * 1000.0f + 
                                         worker_id * 100.0f + 
                                         vector_id * 10.0f + 
                                         x * 2.0f + y * 0.5f;
            }
        }
        
        /* Stencil computation accessing neighboring elements */
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 1; i < N-1; i++) {
            for (int j = 1; j < M-1; j++) {
                /* Complex access pattern to force full partitioning analysis */
                int idx_i = (gang_id + i) % 4;
                int idx_j = (worker_id + j) % 4;
                
                /* 5-point stencil using the partitioned array */
                float center = fully_partitioned[idx_i][idx_j];
                float north = fully_partitioned[(idx_i+3)%4][idx_j];
                float south = fully_partitioned[(idx_i+1)%4][idx_j];
                float east = fully_partitioned[idx_i][(idx_j+1)%4];
                float west = fully_partitioned[idx_i][(idx_j+3)%4];
                
                /* Update both matrices with partitioned computation */
                global_matrix[i][j] = 0.2f * (center + north + south + east + west);
                host_matrix[i][j] += sinf(center) * 0.01f;
            }
        }
    }
    
    /* Conditional partitioning based on runtime values */
    int dynamic_condition = host_array[0] > 50.0f ? 1 : 0;
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copy(host_array[0:N]) copyin(dynamic_condition)
    {
        /* Variable whose partitioning might depend on condition */
        float conditional_var;
        
        if (dynamic_condition) {
            conditional_var = __pgi_gangidx() * 10.0f;  /* Gang partitioned */
        } else {
            conditional_var = __pgi_workeridx() * 5.0f;  /* Worker partitioned */
        }
        
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            host_array[i] += conditional_var * 0.1f;
        }
    }
    
    /* Final computation and verification */
    double checksum = 0.0;
    #pragma acc parallel loop reduction(+:checksum) \
        copyin(global_matrix[0:N][0:M], host_matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += global_matrix[i][j] + host_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
