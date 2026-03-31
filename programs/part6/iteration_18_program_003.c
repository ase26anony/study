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
static int file_scope_counter = 0;       /* Static variable for testing */

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) / (N * M);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    /* Local variables with different scopes */
    int gang_partitioned_scalar = 0;      /* Target: case 1 */
    float worker_partitioned_array[WORKERS]; /* Target: case 2 */
    double gang_worker_partitioned[GANGS][WORKERS]; /* Target: case 3 */
    int vector_partitioned_local;         /* Target: case 4 */
    float gang_vector_partitioned[GANGS][VECTOR_LEN]; /* Target: case 5 */
    double worker_vector_partitioned[WORKERS][VECTOR_LEN]; /* Target: case 6 */
    int fully_partitioned[GANGS][WORKERS][VECTOR_LEN]; /* Target: case 7 */
    
    /* Initialize arrays */
    init_matrices();
    
    for (int i = 0; i < WORKERS; i++) {
        worker_partitioned_array[i] = 0.0f;
    }
    
    /* ========== SCENARIO 1: Gang Redundant (case 0) ========== */
    printf("Starting Scenario 1: Gang Redundant\n");
    #pragma acc parallel num_gangs(GANGS) copy(global_gang_redundant)
    {
        /* global_gang_redundant should be marked as gang redundant */
        int local_gang_id = __pgi_gangidx();
        if (local_gang_id == 0) {
            global_gang_redundant += 1;
        }
        #pragma acc loop gang
        for (int i = 0; i < GANGS; i++) {
            /* Access the gang-redundant variable */
            int temp = global_gang_redundant + i;
        }
    }
    
    /* ========== SCENARIO 2: Gang Partitioned (case 1) ========== */
    printf("Starting Scenario 2: Gang Partitioned\n");
    #pragma acc parallel num_gangs(GANGS) private(gang_partitioned_scalar) \
        copyin(global_matrix) copyout(gang_partitioned_scalar)
    {
        int gang_id = __pgi_gangidx();
        
        /* Each gang gets its own private copy */
        gang_partitioned_scalar = gang_id * 100;
        
        #pragma acc loop gang independent
        for (int i = 0; i < N; i += N/GANGS) {
            float local_sum = 0.0f;
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                local_sum += global_matrix[i][j];
            }
            gang_partitioned_scalar += (int)local_sum;
        }
    }
    
    /* ========== SCENARIO 3: Worker Partitioned (case 2) ========== */
    printf("Starting Scenario 3: Worker Partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) \
        private(worker_partitioned_array) copy(worker_partitioned_array)
    {
        int worker_id = __pgi_workeridx();
        
        /* Each worker gets its own array element */
        worker_partitioned_array[worker_id] = worker_id * 10.0f;
        
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            if (w == worker_id) {
                worker_partitioned_array[w] += 1.0f;
            }
        }
    }
    
    /* ========== SCENARIO 4: Gang+Worker Partitioned (case 3) ========== */
    printf("Starting Scenario 4: Gang+Worker Partitioned\n");
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        private(gang_worker_partitioned) copy(gang_worker_partitioned)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        /* 2D partitioning across gangs and workers */
        gang_worker_partitioned[gang_id][worker_id] = 
            (double)gang_id * 1000.0 + (double)worker_id;
        
        /* Conditional partitioning based on runtime values */
        if (gang_id % 2 == 0) {
            gang_worker_partitioned[gang_id][worker_id] *= 2.0;
        }
    }
    
    /* ========== SCENARIO 5: Vector Partitioned (case 4) ========== */
    printf("Starting Scenario 5: Vector Partitioned\n");
    #pragma acc kernels copyin(global_matrix) copy(vector_partitioned_local)
    {
        vector_partitioned_local = 0;
        
        #pragma acc loop independent vector(VECTOR_LEN)
        for (int i = 0; i < N; i++) {
            float row_sum = 0.0f;
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                row_sum += global_matrix[i][j];
            }
            #pragma acc atomic update
            vector_partitioned_local += (int)row_sum;
        }
    }
    
    /* ========== SCENARIO 6: Gang+Vector Partitioned (case 5) ========== */
    printf("Starting Scenario 6: Gang+Vector Partitioned\n");
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
        private(gang_vector_partitioned) copy(gang_vector_partitioned)
    {
        int gang_id = __pgi_gangidx();
        int vector_id = __pgi_vectoridx();
        
        /* Partition across gangs and vector lanes */
        if (vector_id < VECTOR_LEN) {
            gang_vector_partitioned[gang_id][vector_id] = 
                (float)(gang_id * VECTOR_LEN + vector_id) * 0.5f;
        }
        
        /* Stencil-like computation to complicate analysis */
        #pragma acc loop gang vector
        for (int i = gang_id * (N/GANGS); i < (gang_id + 1) * (N/GANGS); i++) {
            if (i > 0 && i < N-1 && vector_id < M) {
                /* Access neighboring elements */
                float neighbor_sum = global_matrix[i-1][vector_id] + 
                                   global_matrix[i+1][vector_id];
                gang_vector_partitioned[gang_id][vector_id] += neighbor_sum;
            }
        }
    }
    
    /* ========== SCENARIO 7: Worker+Vector Partitioned (case 6) ========== */
    printf("Starting Scenario 7: Worker+Vector Partitioned\n");
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        private(worker_vector_partitioned) copy(worker_vector_partitioned)
    {
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Partition across workers and vector lanes */
        if (vector_id < VECTOR_LEN) {
            worker_vector_partitioned[worker_id][vector_id] = 
                (double)(worker_id * VECTOR_LEN + vector_id) * 0.25;
        }
        
        /* Nested loops with mixed parallelism */
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            if (w == worker_id) {
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    worker_vector_partitioned[w][v] += 
                        sin(worker_vector_partitioned[w][v]);
                }
            }
        }
    }
    
    /* ========== SCENARIO 8: Fully Partitioned (case 7) ========== */
    printf("Starting Scenario 8: Fully Partitioned\n");
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        private(fully_partitioned) copy(fully_partitioned)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Full 3D partitioning */
        if (gang_id < GANGS && worker_id < WORKERS && vector_id < VECTOR_LEN) {
            fully_partitioned[gang_id][worker_id][vector_id] = 
                gang_id * 10000 + worker_id * 100 + vector_id;
        }
        
        /* Complex multi-dimensional access pattern */
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            if (g == gang_id) {
                #pragma acc loop worker
                for (int w = 0; w < WORKERS; w++) {
                    if (w == worker_id) {
                        #pragma acc loop vector
                        for (int v = 0; v < VECTOR_LEN; v++) {
                            /* Access with stride to force partitioning analysis */
                            int idx = (g * WORKERS * VECTOR_LEN + 
                                      w * VECTOR_LEN + v) % (N * M);
                            int i = idx / M;
                            int j = idx % M;
                            fully_partitioned[g][w][v] += 
                                (int)(global_matrix[i][j] * 1000.0f);
                        }
                    }
                }
            }
        }
    }
    
    /* ========== MIXED SCENARIO: Combined partitioning ========== */
    printf("Starting Mixed Scenario\n");
    {
        int mixed_partitioning[GANGS][VECTOR_LEN];
        float shared_across_workers[WORKERS];
        
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
            private(mixed_partitioning) private(shared_across_workers) \
            copy(mixed_partitioning) copy(shared_across_workers)
        {
            int gang_id = __pgi_gangidx();
            int worker_id = __pgi_workeridx();
            int vector_id = __pgi_vectoridx();
            
            /* Different variables with different partitioning */
            if (vector_id < VECTOR_LEN) {
                mixed_partitioning[gang_id][vector_id] = gang_id * VECTOR_LEN + vector_id;
            }
            
            shared_across_workers[worker_id] = worker_id * 1.5f;
            
            /* Conditional execution paths */
            if (gang_id % 2 == 0) {
                #pragma acc loop worker
                for (int w = 0; w < WORKERS; w++) {
                    shared_across_workers[w] += 0.1f;
                }
            } else {
                #pragma acc loop vector
                for (int v = 0; v < VECTOR_LEN; v++) {
                    if (vector_id == v) {
                        mixed_partitioning[gang_id][v] *= 2;
                    }
                }
            }
        }
    }
    
    /* Verify results and prevent dead code elimination */
    int checksum = global_gang_redundant + gang_partitioned_scalar + vector_partitioned_local;
    
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            checksum += (int)gang_worker_partitioned[g][w];
        }
        for (int v = 0; v < VECTOR_LEN; v++) {
            checksum += (int)gang_vector_partitioned[g][v];
        }
    }
    
    for (int w = 0; w < WORKERS; w++) {
        checksum += (int)worker_partitioned_array[w];
        for (int v = 0; v < VECTOR_LEN; v++) {
            checksum += (int)worker_vector_partitioned[w][v];
        }
    }
    
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTOR_LEN; v++) {
                checksum += fully_partitioned[g][w][v];
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum % 1000000);
    printf("All partitioning scenarios executed.\n");
    
    return 0;
}
