/* Test file: omp-oacc-partition-coverage.c
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition omp-oacc-partition-coverage.c
 * Or with diagnostics: gcc -O1 -fopenacc -fopenacc-diag=par -foffload=nvptx-none -o test_partition omp-oacc-partition-coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <openacc.h>

#define NUM_CONFIGS 8
#define MAX_GANGS 32
#define MAX_WORKERS 16
#define VECTOR_LEN 128

/* Structure to store runtime query results */
typedef struct {
    int gangs;
    int workers;
    int vector_len;
    long checksum;
} partition_config_t;

/* Host array to store results from device */
partition_config_t host_results[NUM_CONFIGS];

/* Device array for runtime queries */
#pragma acc declare create(host_results)

/* Function to force runtime to map partition codes */
void test_partition_mapping() {
    int i, j, k;
    int dynamic_gangs = 4;
    int dynamic_workers = 2;
    int dynamic_vector = 64;
    
    /* Initialize host results */
    for (i = 0; i < NUM_CONFIGS; i++) {
        host_results[i].gangs = 0;
        host_results[i].workers = 0;
        host_results[i].vector_len = 0;
        host_results[i].checksum = 0;
    }
    
    /* Copy initial values to device */
    #pragma acc update device(host_results)
    
    printf("Testing OpenACC partition mapping...\n");
    
    /* Configuration 0: gang redundant (likely maps to case 0) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(1) \
        copyout(host_results[0:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[0].gangs = g;
        host_results[0].workers = w;
        host_results[0].vector_len = v;
        host_results[0].checksum = g + w + v;
    }
    
    /* Configuration 1: gang partitioned (case 1) */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyout(host_results[1:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[1].gangs = g;
        host_results[1].workers = w;
        host_results[1].vector_len = v;
        host_results[1].checksum = g * 1000 + w * 100 + v;
    }
    
    /* Configuration 2: worker partitioned (case 2) */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyout(host_results[2:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[2].gangs = g;
        host_results[2].workers = w;
        host_results[2].vector_len = v;
        
        /* Add some computation to prevent optimization */
        long sum = 0;
        #pragma acc loop worker reduction(+:sum)
        for (i = 0; i < 100; i++) {
            sum += i;
        }
        host_results[2].checksum = sum + g + w + v;
    }
    
    /* Configuration 3: gang+worker partitioned (case 3) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(1) \
        copyout(host_results[3:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[3].gangs = g;
        host_results[3].workers = w;
        host_results[3].vector_len = v;
        
        /* Nested parallelism */
        long gang_sum = 0;
        #pragma acc loop gang reduction(+:gang_sum)
        for (i = 0; i < g; i++) {
            long worker_sum = 0;
            #pragma acc loop worker reduction(+:worker_sum)
            for (j = 0; j < w; j++) {
                worker_sum += i * 10 + j;
            }
            gang_sum += worker_sum;
        }
        host_results[3].checksum = gang_sum + g + w + v;
    }
    
    /* Configuration 4: vector partitioned (case 4) */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(VECTOR_LEN) \
        copyout(host_results[4:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[4].gangs = g;
        host_results[4].workers = w;
        host_results[4].vector_len = v;
        
        /* Vector operation */
        int array[VECTOR_LEN];
        #pragma acc loop vector
        for (i = 0; i < v; i++) {
            array[i] = i * 2;
        }
        
        long vec_sum = 0;
        #pragma acc loop vector reduction(+:vec_sum)
        for (i = 0; i < v; i++) {
            vec_sum += array[i];
        }
        host_results[4].checksum = vec_sum + g + w + v;
    }
    
    /* Configuration 5: gang+vector partitioned (case 5) */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        copyout(host_results[5:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[5].gangs = g;
        host_results[5].workers = w;
        host_results[5].vector_len = v;
        
        /* Combined gang and vector parallelism */
        long total = 0;
        #pragma acc loop gang reduction(+:total)
        for (i = 0; i < g; i++) {
            #pragma acc loop vector reduction(+:total)
            for (j = 0; j < v; j++) {
                total += i * v + j;
            }
        }
        host_results[5].checksum = total + g + w + v;
    }
    
    /* Configuration 6: worker+vector partitioned (case 6) */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(16) \
        copyout(host_results[6:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[6].gangs = g;
        host_results[6].workers = w;
        host_results[6].vector_len = v;
        
        /* Worker and vector nested */
        long worker_vec_sum = 0;
        #pragma acc loop worker reduction(+:worker_vec_sum)
        for (i = 0; i < w; i++) {
            #pragma acc loop vector reduction(+:worker_vec_sum)
            for (j = 0; j < v; j++) {
                worker_vec_sum += i * 100 + j;
            }
        }
        host_results[6].checksum = worker_vec_sum + g + w + v;
    }
    
    /* Configuration 7: fully partitioned (case 7) */
    #pragma acc parallel num_gangs(2) num_workers(2) vector_length(8) \
        copyout(host_results[7:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[7].gangs = g;
        host_results[7].workers = w;
        host_results[7].vector_len = v;
        
        /* Fully nested: gang -> worker -> vector */
        long full_sum = 0;
        #pragma acc loop gang reduction(+:full_sum)
        for (i = 0; i < g; i++) {
            #pragma acc loop worker reduction(+:full_sum)
            for (j = 0; j < w; j++) {
                #pragma acc loop vector reduction(+:full_sum)
                for (k = 0; k < v; k++) {
                    full_sum += i * 10000 + j * 100 + k;
                }
            }
        }
        host_results[7].checksum = full_sum + g + w + v;
    }
    
    /* Dynamic configuration using host variables */
    printf("\nTesting dynamic partition configuration...\n");
    #pragma acc parallel num_gangs(dynamic_gangs) num_workers(dynamic_workers) \
        vector_length(dynamic_vector) copyout(host_results[0:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        /* Store in first slot to avoid array bounds */
        host_results[0].gangs += g;
        host_results[0].workers += w;
        host_results[0].vector_len += v;
    }
    
    /* Test with kernels construct (different code path) */
    printf("\nTesting kernels construct...\n");
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(4) \
        copyout(host_results[1:1])
    {
        int g = acc_get_num_gangs(acc_async_noval);
        int w = acc_get_num_workers(acc_async_noval);
        int v = acc_get_vector_length(acc_async_noval);
        
        host_results[1].gangs += g;
        host_results[1].workers += w;
        host_results[1].vector_len += v;
    }
    
    /* Test with collapse clause */
    printf("\nTesting with collapse clause...\n");
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(4) \
        copyout(host_results[2:1])
    {
        long collapse_sum = 0;
        #pragma acc loop gang worker vector collapse(2) reduction(+:collapse_sum)
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 4; j++) {
                collapse_sum += i * 10 + j;
            }
        }
        host_results[2].checksum += collapse_sum;
    }
    
    /* Force diagnostic path with acc_malloc */
    printf("\nTesting diagnostic path with acc_malloc...\n");
    void *dev_ptr = acc_malloc(1024);
    if (dev_ptr == NULL) {
        printf("acc_malloc failed - this may trigger diagnostic output\n");
    } else {
        acc_free(dev_ptr);
    }
    
    /* Update all results from device */
    #pragma acc update host(host_results[0:NUM_CONFIGS])
    
    /* Print results and calculate final checksum */
    long final_checksum = 0;
    printf("\nPartition Configuration Results:\n");
    printf("Config | Gangs | Workers | Vector | Checksum\n");
    printf("-------|-------|---------|--------|----------\n");
    
    for (i = 0; i < NUM_CONFIGS; i++) {
        printf("%6d | %5d | %7d | %6d | %10ld\n",
               i, host_results[i].gangs, host_results[i].workers,
               host_results[i].vector_len, host_results[i].checksum);
        final_checksum += host_results[i].gangs + host_results[i].workers +
                         host_results[i].vector_len + host_results[i].checksum;
    }
    
    printf("\nFinal checksum: %ld\n", final_checksum);
    
    /* Verify we got some non-zero results */
    if (final_checksum == 0) {
        printf("WARNING: All results are zero - regions may have been optimized away\n");
        return;
    }
    
    printf("\nPartition mapping test completed successfully.\n");
}

/* Test with data regions and unstructured constructs */
void test_data_regions() {
    int N = 1000;
    float *a, *b, *c;
    
    a = (float*)malloc(N * sizeof(float));
    b = (float*)malloc(N * sizeof(float));
    c = (float*)malloc(N * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 0.5f;
        c[i] = 0.0f;
    }
    
    /* Structured data region */
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(32)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] + b[i];
            }
            
            /* Force partition mapping by using runtime values */
            if (g > 0 && w > 0 && v > 0) {
                /* This computation uses the partition dimensions */
                float dummy = g * 0.1f + w * 0.01f + v * 0.001f;
                c[0] += dummy;
            }
        }
    }
    
    /* Unstructured data region */
    float *dev_a, *dev_b, *dev_c;
    dev_a = (float*)acc_malloc(N * sizeof(float));
    dev_b = (float*)acc_malloc(N * sizeof(float));
    dev_c = (float*)acc_malloc(N * sizeof(float));
    
    if (dev_a && dev_b && dev_c) {
        #pragma acc enter data copyin(a[0:N], b[0:N]) create(c[0:N])
        #pragma acc parallel present(a, b, c) num_gangs(4) num_workers(2) vector_length(16)
        {
            int g = acc_get_num_gangs(acc_async_noval);
            int w = acc_get_num_workers(acc_async_noval);
            int v = acc_get_vector_length(acc_async_noval);
            
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                c[i] = a[i] * b[i];
            }
        }
        #pragma acc exit data copyout(c[0:N]) delete(a, b)
        
        acc_free(dev_a);
        acc_free(dev_b);
        acc_free(dev_c);
    }
    
    free(a);
    free(b);
    free(c);
}

int main() {
    printf("=== OpenACC Partition Mapping Coverage Test ===\n\n");
    
    /* Test 1: Direct partition mapping */
    test_partition_mapping();
    
    /* Test 2: Data regions with different styles */
    printf("\n\n=== Testing Data Regions ===\n");
    test_data_regions();
    
    /* Final summary */
    printf("\n=== All tests completed ===\n");
    printf("The runtime should have internally mapped partition codes to strings:\n");
    printf("  0: gang redundant\n");
    printf("  1: gang partitioned\n");
    printf("  2: worker partitioned\n");
    printf("  3: gang+worker partitioned\n");
    printf("  4: vector partitioned\n");
    printf("  5: gang+vector partitioned\n");
    printf("  6: worker+vector partitioned\n");
    printf("  7: fully partitioned\n");
    
    return 0;
}
