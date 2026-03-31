/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables that will be partitioned differently */
int global_gang_redundant = 42;          /* Should become case 0 */
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
    
    /* Initialize test data */
    init_data();
    
    /* 1. GANG-ONLY REGION - targeting case 1 (gang partitioned) */
    printf("Starting gang-only region...\n");
    #pragma acc parallel num_gangs(GANGS) copyout(host_array[0:N])
    {
        int gang_private = 0;  /* Should be gang partitioned (case 1) */
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            gang_private = i;
            host_array[i] = global_matrix[i][0] + gang_private;
        }
    }
    
    /* 2. VECTOR-ONLY REGION - targeting case 4 (vector partitioned) */
    printf("Starting vector-only region...\n");
    {
        int vector_private = 5;  /* Will be vector partitioned */
        #pragma acc parallel vector_length(VECTOR_LEN) copy(result)
        {
            #pragma acc loop vector
            for (int i = 0; i < 100; i++) {
                vector_private += i;
            }
            #pragma acc atomic update
            result += vector_private;  /* Force computation */
        }
    }
    
    /* 3. COMPLEX NESTED REGION - targeting multiple combined states */
    printf("Starting complex nested region...\n");
    {
        int gang_worker_var = 0;      /* Should become case 3 */
        int gang_vector_var = 0;      /* Should become case 5 */
        int worker_vector_var = 0;    /* Should become case 6 */
        int fully_partitioned = 0;    /* Should become case 7 */
        float temp_matrix[64][64];    /* Local multi-dimensional array */
        
        /* Initialize local matrix */
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                temp_matrix[i][j] = (float)(i + j);
            }
        }
        
        #pragma acc parallel num_gangs(2) num_workers(4) vector_length(32) \
                copyin(temp_matrix) copy(gang_worker_var, gang_vector_var, \
                worker_vector_var, fully_partitioned)
        {
            /* Variables at different scopes within parallel region */
            int gang_level = 0;           /* Gang partitioned (case 1) */
            int worker_level = 0;         /* Worker partitioned (case 2) */
            int vector_level = 0;         /* Vector partitioned (case 4) */
            
            /* GANG REDUNDANT variable - targeting case 0 */
            const int gang_redundant_val = global_gang_redundant;
            
            #pragma acc loop gang
            for (int g = 0; g < 2; g++) {
                gang_level = g;
                
                #pragma acc loop worker
                for (int w = 0; w < 4; w++) {
                    worker_level = w;
                    gang_worker_var = gang_level * 10 + worker_level;
                    
                    #pragma acc loop vector
                    for (int v = 0; v < 32; v++) {
                        vector_level = v;
                        
                        /* Combined partitioning states */
                        gang_vector_var = gang_level * 100 + vector_level;
                        worker_vector_var = worker_level * 100 + vector_level;
                        fully_partitioned = gang_level * 1000 + worker_level * 100 + vector_level;
                        
                        /* Stencil-like computation on multi-dimensional array */
                        int idx = (g * 4 * 32 + w * 32 + v) % 64;
                        if (idx > 0 && idx < 63) {
                            temp_matrix[idx][0] = 
                                temp_matrix[idx-1][0] * 0.25f +
                                temp_matrix[idx][0] * 0.5f +
                                temp_matrix[idx+1][0] * 0.25f;
                        }
                    }
                }
            }
        }
        
        printf("Complex region results: %d, %d, %d, %d\n", 
               gang_worker_var, gang_vector_var, worker_vector_var, fully_partitioned);
    }
    
    /* 4. KERNELS REGION with explicit loop directives */
    printf("Starting kernels region...\n");
    {
        float kernels_matrix[128][128];
        int kernels_scalar = 0;
        
        /* Initialize */
        for (int i = 0; i < 128; i++) {
            for (int j = 0; j < 128; j++) {
                kernels_matrix[i][j] = (float)(i * j);
            }
        }
        
        #pragma acc kernels copy(kernels_matrix, kernels_scalar)
        {
            /* Mix of private and shared variables */
            int private_gang = 0;
            int private_worker = 0;
            int private_vector = 0;
            
            /* Explicit loop directives with different partitioning */
            #pragma acc loop gang independent private(private_gang)
            for (int i = 0; i < 128; i++) {
                private_gang = i;
                
                #pragma acc loop worker independent private(private_worker)
                for (int j = 0; j < 128; j++) {
                    private_worker = j;
                    
                    #pragma acc loop vector independent private(private_vector)
                    for (int k = 0; k < 8; k++) {
                        private_vector = k;
                        
                        /* Conditional partitioning influence */
                        if ((i + j + k) % 3 == 0) {
                            kernels_matrix[i][j] += private_gang + private_worker + private_vector;
                        } else {
                            kernels_matrix[i][j] -= private_gang * private_worker * private_vector;
                        }
                    }
                }
            }
            
            /* Reduction with mixed partitioning */
            #pragma acc loop gang reduction(+:kernels_scalar)
            for (int i = 0; i < 128; i++) {
                #pragma acc loop worker reduction(+:kernels_scalar)
                for (int j = 0; j < 128; j++) {
                    kernels_scalar += (int)kernels_matrix[i][j];
                }
            }
        }
        
        printf("Kernels scalar result: %d\n", kernels_scalar);
    }
    
    /* 5. RUNTIME-DEPENDENT CONDITIONAL REGION */
    printf("Starting conditional region...\n");
    {
        int conditional_var = 0;
        float dynamic_matrix[50][50];
        
        /* Initialize with pattern */
        for (int i = 0; i < 50; i++) {
            for (int j = 0; j < 50; j++) {
                dynamic_matrix[i][j] = (float)((i + j) % 10);
            }
        }
        
        #pragma acc parallel num_gangs(2) copy(conditional_var, dynamic_matrix) \
                if(host_scalar > 5)  /* Runtime condition */
        {
            int local_partitioned = 0;
            
            /* The partitioning may change based on runtime condition */
            #pragma acc loop gang private(local_partitioned)
            for (int i = 0; i < 50; i++) {
                local_partitioned = i % 2;
                
                #pragma acc loop vector
                for (int j = 0; j < 50; j++) {
                    /* Access with stride to complicate analysis */
                    int idx = (i * 13 + j * 7) % 50;
                    dynamic_matrix[i][j] += local_partitioned * dynamic_matrix[idx][idx];
                    
                    #pragma acc atomic update
                    conditional_var += (int)dynamic_matrix[i][j];
                }
            }
        }
        
        printf("Conditional region result: %d\n", conditional_var);
    }
    
    /* Final computation and output */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += host_array[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
