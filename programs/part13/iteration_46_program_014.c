/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare directive analysis */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP constructs */
void process_data(int N, int M, int argc) {
    /* Use argc for dynamic bounds to prevent constant folding */
    volatile int dyn_N = (argc > 1) ? N : 512;
    volatile int dyn_M = (argc > 2) ? M : 512;
    
    int src[dyn_N][dyn_M];
    int dst[dyn_N][dyn_M];
    int tmp_sum = 0;
    int i, j, k;
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < dyn_N; i++) {
        for (j = 0; j < dyn_M; j++) {
            src[i][j] = i * dyn_M + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with complex partitioning scenarios */
    #pragma acc data copy(src, dst) copyin(dyn_N, dyn_M) copyout(tmp_sum)
    {
        /* FIRST PARALLEL REGION: Complex partitioning with all levels */
        /* This should trigger analysis for multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:tmp_sum) \
                firstprivate(dyn_N, dyn_M)
        for (i = 0; i < dyn_N; i++) {
            for (j = 0; j < dyn_M; j++) {
                int temp = 0;
                
                /* Conditional data access to force broadcast analysis */
                if ((i + j) % 3 == 0) {
                    /* Access pattern requiring gang-level communication */
                    for (k = 0; k < 4; k++) {
                        temp += src[(i + k) % dyn_N][j];
                    }
                } else if ((i + j) % 3 == 1) {
                    /* Different pattern for worker-level analysis */
                    temp = src[i][(j + 1) % dyn_M] - src[i][(j - 1 + dyn_M) % dyn_M];
                } else {
                    /* Vector-level pattern */
                    temp = src[i][j] * 2;
                }
                
                /* Conditional store with reduction */
                if (temp > 0) {
                    dst[i][j] = temp;
                    tmp_sum += temp;
                }
            }
        }
        
        /* SECOND KERNELS REGION: Different partitioning strategy */
        /* Uses gang+vector without worker partitioning */
        int row_sums[dyn_N];
        #pragma acc kernels copyin(src) copyout(row_sums) \
                present(dyn_N, dyn_M)
        {
            #pragma acc loop gang vector
            for (i = 0; i < dyn_N; i++) {
                int row_sum = 0;
                #pragma acc loop vector reduction(+:row_sum)
                for (j = 0; j < dyn_M; j++) {
                    /* Data-dependent access with condition */
                    if (src[i][j] % 7 == 0) {
                        row_sum += src[i][j];
                    } else if (src[i][j] % 7 == 1) {
                        row_sum -= src[i][j] / 2;
                    }
                }
                row_sums[i] = row_sum;
            }
        }
        
        /* THIRD: Nested parallelism with explicit clauses */
        /* This creates gang+worker partitioned scenario */
        int block_sums[16][16];
        #pragma acc parallel num_gangs(16) num_workers(4) vector_length(32) \
                copyout(block_sums)
        {
            #pragma acc loop gang
            for (int bx = 0; bx < 16; bx++) {
                #pragma acc loop worker
                for (int by = 0; by < 16; by++) {
                    int block_sum = 0;
                    #pragma acc loop vector reduction(+:block_sum)
                    for (int idx = 0; idx < 256; idx++) {
                        int x = bx * 16 + (idx / 16);
                        int y = by * 16 + (idx % 16);
                        if (x < dyn_N && y < dyn_M) {
                            block_sum += dst[x][y];
                        }
                    }
                    block_sums[bx][by] = block_sum;
                }
            }
        }
        
        /* Update global device data */
        #pragma acc parallel present(global_matrix, global_sum)
        {
            #pragma acc loop gang worker vector collapse(2)
            for (i = 0; i < 256; i++) {
                for (j = 0; j < 256; j++) {
                    if (i < dyn_N && j < dyn_M) {
                        global_matrix[i][j] += dst[i][j];
                    }
                }
            }
            
            #pragma acc atomic update
            global_sum += tmp_sum;
        }
    }
    
    /* Mixed OpenMP outside ACC region */
    int verify_sum = 0;
    #pragma omp parallel for reduction(+:verify_sum) private(i, j)
    for (i = 0; i < dyn_N && i < 256; i++) {
        for (j = 0; j < dyn_M && j < 256; j++) {
            verify_sum += global_matrix[i][j];
        }
    }
    
    printf("Partial checksum: %d\n", tmp_sum);
    printf("Global verify sum: %d\n", verify_sum);
}

/* Another function with different partitioning pattern */
void vector_heavy_computation(int size, int argc) {
    int vec_a[size], vec_b[size], vec_c[size];
    volatile int dyn_size = (argc > 3) ? size : 1024;
    
    /* Initialize */
    #pragma omp parallel for
    for (int i = 0; i < dyn_size; i++) {
        vec_a[i] = i;
        vec_b[i] = dyn_size - i;
    }
    
    /* Vector-partitioned computation */
    #pragma acc parallel loop vector_length(256) \
            copyin(vec_a, vec_b, dyn_size) copyout(vec_c)
    for (int i = 0; i < dyn_size; i++) {
        /* Complex conditional vector operation */
        int val;
        if (i % 4 == 0) {
            val = vec_a[i] + vec_b[i];
        } else if (i % 4 == 1) {
            val = vec_a[i] * vec_b[i];
        } else if (i % 4 == 2) {
            val = vec_a[i] - vec_b[i];
        } else {
            val = vec_b[i] - vec_a[i];
        }
        
        /* Nested conditional */
        vec_c[i] = (val > 0) ? val : -val;
    }
    
    /* Fully partitioned scenario with private/firstprivate mixing */
    int local_sums[4] = {0, 0, 0, 0};
    #pragma acc parallel num_gangs(4) num_workers(8) vector_length(32) \
            copyin(vec_c, dyn_size) copyout(local_sums)
    {
        int gang_id, worker_id;
        #pragma acc loop gang private(gang_id)
        for (int g = 0; g < 4; g++) {
            gang_id = g;
            #pragma acc loop worker private(worker_id) reduction(+:local_sums[0:4])
            for (int w = 0; w < 8; w++) {
                worker_id = w;
                int segment_sum = 0;
                #pragma acc loop vector reduction(+:segment_sum)
                for (int v = 0; v < 32; v++) {
                    int idx = (gang_id * 8 * 32) + (worker_id * 32) + v;
                    if (idx < dyn_size) {
                        segment_sum += vec_c[idx];
                    }
                }
                local_sums[gang_id] += segment_sum;
            }
        }
    }
    
    printf("Vector computation completed, local sums calculated\n");
}

int main(int argc, char **argv) {
    printf("Starting complex OpenACC partitioning test...\n");
    
    /* First test with 2D data */
    process_data(512, 512, argc);
    
    /* Second test with 1D vector operations */
    vector_heavy_computation(2048, argc);
    
    /* Final mixed region */
    int final_check = 0;
    #pragma acc parallel loop reduction(+:final_check) \
            copyin(global_sum) copyout(final_check)
    for (int i = 0; i < 1000; i++) {
        final_check += global_sum % (i + 1);
    }
    
    printf("Final check value: %d\n", final_check);
    printf("Test completed successfully.\n");
    
    return 0;
}
