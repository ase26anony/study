/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex partitioning scenarios */
int global_gang_redundant = 42;          /* Should become case 0 */
float global_matrix[N][M];               /* Base for multi-dimensional access */
static int file_scope_var = 100;         /* Static storage duration */

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j);
        }
    }
}

/* Main function with complex nested parallelism */
int main() {
    int result = 0;
    float local_matrix[N][M];
    float temp_matrix[N][M];
    
    /* Initialize data */
    init_matrices();
    
    /* Copy to local matrix */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            local_matrix[i][j] = global_matrix[i][j];
        }
    }
    
    /* ============================================
       CASE 0: "gang redundant"
       Global variable accessed by all gangs
       ============================================ */
    #pragma acc parallel copyin(global_gang_redundant) \
                         copy(local_matrix[0:N][0:M]) \
                         num_gangs(GANGS) num_workers(1) vector_length(1)
    {
        int gang_id = __pgi_gangidx();
        /* All gangs read the same global variable */
        local_matrix[gang_id][0] += global_gang_redundant;
    }
    
    /* ============================================
       CASE 1: "gang partitioned"
       Variable partitioned across gangs only
       ============================================ */
    #pragma acc parallel copy(local_matrix[0:N][0:M]) \
                         copyout(temp_matrix[0:N][0:M]) \
                         num_gangs(GANGS) num_workers(1) vector_length(1)
    {
        int gang_private = __pgi_gangidx() * 100;  /* Different per gang */
        int i = __pgi_gangidx();
        
        /* Each gang works on different rows */
        for (int j = 0; j < M; j++) {
            temp_matrix[i][j] = local_matrix[i][j] + gang_private;
        }
    }
    
    /* ============================================
       CASE 2: "worker partitioned" 
       Variable partitioned across workers only
       ============================================ */
    #pragma acc parallel copy(local_matrix[0:N][0:M]) \
                         num_gangs(1) num_workers(WORKERS) vector_length(1)
    {
        int worker_id = __pgi_workeridx();
        int worker_private = worker_id * 50;  /* Different per worker */
        
        /* Each worker processes different columns */
        for (int i = 0; i < N; i++) {
            if (i % WORKERS == worker_id) {
                local_matrix[i][0] += worker_private;
            }
        }
    }
    
    /* ============================================
       CASE 3: "gang+worker partitioned"
       Variable partitioned across both gangs and workers
       ============================================ */
    #pragma acc parallel copy(local_matrix[0:N][0:M]) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(1)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        /* Combined gang-worker private variable */
        int gang_worker_private = gang_id * 1000 + worker_id * 100;
        
        /* Each gang-worker pair gets different work */
        int start_i = gang_id * (N / GANGS);
        int end_i = (gang_id + 1) * (N / GANGS);
        
        for (int i = start_i; i < end_i; i++) {
            if (i % WORKERS == worker_id) {
                local_matrix[i][0] += gang_worker_private;
            }
        }
    }
    
    /* ============================================
       CASE 4: "vector partitioned"
       Variable partitioned across vector lanes
       ============================================ */
    #pragma acc kernels copy(local_matrix[0:N][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            float vector_private[M];  /* Will be vector partitioned */
            
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                /* Vector parallelism inside worker loop */
                #pragma acc loop vector
                for (int k = 0; k < VECTOR_LEN; k++) {
                    if (j * VECTOR_LEN + k < M) {
                        vector_private[j] = local_matrix[i][j] * k;
                        local_matrix[i][j] += vector_private[j];
                    }
                }
            }
        }
    }
    
    /* ============================================
       CASE 5: "gang+vector partitioned"
       Variable partitioned across gangs and vectors
       ============================================ */
    #pragma acc parallel copy(local_matrix[0:N][0:M]) \
                         num_gangs(GANGS) num_workers(1) vector_length(VECTOR_LEN)
    {
        int gang_id = __pgi_gangidx();
        int vector_id = __pgi_vectoridx();
        
        /* Gang+vector private variable */
        float gang_vector_private = gang_id * 10.0f + vector_id * 1.0f;
        
        /* Stencil computation with multi-dimensional access */
        int i = gang_id * (N / GANGS) + vector_id;
        if (i < N - 1) {
            /* Access neighboring elements */
            local_matrix[i][0] = local_matrix[i+1][0] * gang_vector_private;
        }
    }
    
    /* ============================================
       CASE 6: "worker+vector partitioned"
       Variable partitioned across workers and vectors
       ============================================ */
    #pragma acc parallel copy(local_matrix[0:N][0:M]) \
                         num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Worker+vector private variable */
        int worker_vector_private = worker_id * VECTOR_LEN + vector_id;
        
        /* Conditional partitioning based on runtime value */
        if (worker_vector_private % 3 == 0) {
            for (int i = 0; i < N; i++) {
                int idx = (i * M + worker_id * VECTOR_LEN + vector_id) % M;
                if (idx < M) {
                    local_matrix[i][idx] += worker_vector_private;
                }
            }
        }
    }
    
    /* ============================================
       CASE 7: "fully partitioned"
       Variable partitioned across gangs, workers, and vectors
       ============================================ */
    #pragma acc parallel copy(local_matrix[0:N][0:M]) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Fully partitioned variable - unique to each processing element */
        int fully_partitioned = gang_id * WORKERS * VECTOR_LEN + 
                               worker_id * VECTOR_LEN + 
                               vector_id;
        
        /* Complex multi-dimensional stencil computation */
        int i = gang_id * (N / GANGS) + worker_id;
        int j = vector_id;
        
        if (i > 0 && i < N - 1 && j > 0 && j < M - 1) {
            /* 5-point stencil accessing neighbors */
            local_matrix[i][j] = (local_matrix[i-1][j] + 
                                 local_matrix[i+1][j] + 
                                 local_matrix[i][j-1] + 
                                 local_matrix[i][j+1]) / 4.0f;
            local_matrix[i][j] += fully_partitioned;
        }
    }
    
    /* ============================================
       Mixed constructs with explicit clauses
       ============================================ */
    {
        int outer_private = 0;
        int firstprivate_var = 777;
        
        #pragma acc data copy(local_matrix[0:N][0:M]) create(temp_matrix[0:N][0:M])
        {
            #pragma acc kernels
            {
                #pragma acc loop gang(static:1)
                for (int i = 0; i < N; i++) {
                    int gang_private = i;
                    
                    #pragma acc loop worker
                    for (int j = 0; j < M; j++) {
                        int worker_private = j;
                        
                        #pragma acc loop vector
                        for (int k = 0; k < 16; k++) {
                            /* Mix of private and shared accesses */
                            temp_matrix[i][j] = local_matrix[i][j] + 
                                               gang_private + 
                                               worker_private + 
                                               k + 
                                               firstprivate_var;
                        }
                    }
                }
            }
            
            /* Reduction to prevent dead code elimination */
            float sum = 0.0f;
            #pragma acc parallel copyin(temp_matrix) copy(sum) \
                                 reduction(+:sum)
            {
                #pragma acc loop gang reduction(+:sum)
                for (int i = 0; i < N; i++) {
                    #pragma acc loop worker reduction(+:sum)
                    for (int j = 0; j < M; j++) {
                        #pragma acc loop vector reduction(+:sum)
                        for (int k = 0; k < 1; k++) {
                            sum += temp_matrix[i][j];
                        }
                    }
                }
            }
            
            result = (int)sum;
        }
    }
    
    /* Final computation and output */
    printf("Result checksum: %d\n", result % 1000);
    printf("File scope var: %d\n", file_scope_var);
    
    return 0;
}
