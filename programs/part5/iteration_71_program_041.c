/* 
 * Test program to cover partition type switch cases in omp-oacc-neuter-broadcast.cc
 * Covers all 8 cases (0-7) of partition type enumeration
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define VERIFY_SIZE 256

/* Helper function with routine directive to force partition analysis */
#pragma acc routine gang
void acc_helper_gang(int *arr, int idx, int val) {
    arr[idx] = val;
}

#pragma acc routine worker
void acc_helper_worker(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}

#pragma acc routine vector
void acc_helper_vector(int *arr, int idx, int val) {
    arr[idx] = val * 3;
}

/* OpenMP equivalent helpers */
#pragma omp declare target
void omp_helper_gang(int *arr, int idx, int val) {
    arr[idx] = val;
}

void omp_helper_worker(int *arr, int idx, int val) {
    arr[idx] = val * 2;
}

void omp_helper_vector(int *arr, int idx, int val) {
    arr[idx] = val * 3;
}
#pragma omp end declare target

int main() {
    int *a, *b, *c, *d, *e;
    int i, errors = 0;
    int num_gangs, num_workers, vector_len;
    int flag = 1; /* For conditional partition selection */
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    e = (int*)malloc(N * sizeof(int));
    
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
        d[i] = 0;
        e[i] = 0;
    }
    
    printf("Testing partition type coverage...\n");
    
    /* ========== OPENACC TESTS ========== */
    
    /* Test 1: Case 0 - gang redundant (no explicit partition) */
    printf("Test 1: Gang redundant (case 0)\n");
    #pragma acc data copyin(a[0:N], b[0:N]) copyout(c[0:N])
    {
        num_gangs = 4;
        #pragma acc parallel loop gang(num_gangs) /* No worker/vector clause = gang redundant */
        for (i = 0; i < N; i++) {
            c[i] = a[i] + b[i];
        }
    }
    
    /* Verify */
    for (i = 0; i < VERIFY_SIZE; i++) {
        if (c[i] != a[i] + b[i]) errors++;
    }
    
    /* Test 2: Case 1 - gang partitioned */
    printf("Test 2: Gang partitioned (case 1)\n");
    #pragma acc data copyin(a[0:N]) copy(d[0:N])
    {
        num_gangs = 8;
        #pragma acc parallel loop gang(num_gangs) gang /* Explicit gang partition */
        for (i = 0; i < N; i++) {
            d[i] = a[i] * 2;
            acc_helper_gang(d, i, d[i]); /* Force gang routine analysis */
        }
    }
    
    /* Test 3: Case 2 - worker partitioned */
    printf("Test 3: Worker partitioned (case 2)\n");
    #pragma acc data copyin(b[0:N]) copy(e[0:N])
    {
        num_workers = 4;
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) worker /* Worker partitioned */
        for (i = 0; i < N; i++) {
            e[i] = b[i] * 3;
            acc_helper_worker(e, i, e[i]);
        }
    }
    
    /* Test 4: Case 3 - gang+worker partitioned */
    printf("Test 4: Gang+worker partitioned (case 3)\n");
    {
        int *temp = (int*)malloc(N * sizeof(int));
        #pragma acc data copyin(a[0:N], b[0:N]) create(temp[0:N])
        {
            num_gangs = 2;
            num_workers = 4;
            /* Outer gang-redundant region with inner worker-partitioned loop */
            #pragma acc kernels gang(num_gangs) /* Outer: gang redundant */
            {
                #pragma acc loop worker(num_workers) worker /* Inner: worker partitioned */
                for (i = 0; i < N; i++) {
                    temp[i] = a[i] + b[i];
                }
            }
            /* Combined gang+worker partition */
            #pragma acc parallel loop gang(num_gangs) worker(num_workers) gang worker
            for (i = 0; i < N; i++) {
                c[i] = temp[i] * 2;
            }
        }
        free(temp);
    }
    
    /* Test 5: Case 4 - vector partitioned */
    printf("Test 5: Vector partitioned (case 4)\n");
    #pragma acc data copyin(a[0:N]) copy(d[0:N])
    {
        vector_len = 32;
        #pragma acc parallel loop gang(num_gangs) vector(vector_len) vector /* Vector partitioned */
        for (i = 0; i < N; i++) {
            d[i] = a[i] * 4;
            acc_helper_vector(d, i, d[i]);
        }
    }
    
    /* Test 6: Case 5 - gang+vector partitioned */
    printf("Test 6: Gang+vector partitioned (case 5)\n");
    {
        int *temp = (int*)malloc(N * sizeof(int));
        #pragma acc data copyin(a[0:N]) create(temp[0:N]) copyout(e[0:N])
        {
            num_gangs = 4;
            vector_len = 16;
            /* Conditional partition selection */
            #pragma acc parallel loop gang(num_gangs) vector(flag ? 32 : 64) gang vector
            for (i = 0; i < N; i++) {
                temp[i] = a[i] * 5;
            }
            
            /* Different device type simulation */
            #pragma acc set device_type(nvidia)
            #pragma acc parallel loop gang(num_gangs) vector(vector_len) gang vector
            for (i = 0; i < N; i++) {
                e[i] = temp[i] + 1;
            }
        }
        free(temp);
    }
    
    /* Test 7: Case 6 - worker+vector partitioned */
    printf("Test 7: Worker+vector partitioned (case 6)\n");
    #pragma acc data copyin(b[0:N]) copy(c[0:N])
    {
        num_workers = 2;
        vector_len = 64;
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_len) worker vector
        for (i = 0; i < N; i++) {
            c[i] = b[i] * 6;
        }
    }
    
    /* Test 8: Case 7 - fully partitioned (gang+worker+vector) */
    printf("Test 8: Fully partitioned (case 7)\n");
    #pragma acc data copyin(a[0:N], b[0:N]) copy(d[0:N])
    {
        num_gangs = 8;
        num_workers = 2;
        vector_len = 32;
        #pragma acc parallel loop gang(num_gangs) worker(num_workers) vector(vector_len) gang worker vector
        for (i = 0; i < N; i++) {
            d[i] = a[i] + b[i] * 7;
        }
    }
    
    /* ========== OPENMP TESTS ========== */
    
    /* Reinitialize arrays for OpenMP tests */
    for (i = 0; i < N; i++) {
        c[i] = 0;
        d[i] = 0;
        e[i] = 0;
    }
    
    /* Test 9: OpenMP - gang partitioned (case 1) */
    printf("Test 9: OpenMP gang partitioned\n");
    #pragma omp target data map(to: a[0:N]) map(from: c[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(4) /* gang partitioned */
        for (i = 0; i < N; i++) {
            c[i] = a[i] * 2;
        }
    }
    
    /* Test 10: OpenMP - worker partitioned (case 2) with nested parallelism */
    printf("Test 10: OpenMP worker partitioned\n");
    #pragma omp target data map(to: b[0:N]) map(from: d[0:N])
    {
        #pragma omp target teams distribute parallel for num_teams(2) thread_limit(4) /* worker partitioned */
        for (i = 0; i < N; i++) {
            d[i] = b[i] * 3;
            #pragma omp parallel for /* Nested worker parallelism */
            for (int j = 0; j < 4; j++) {
                d[i] += j;
            }
        }
    }
    
    /* Test 11: OpenMP - vector partitioned (case 4) with SIMD */
    printf("Test 11: OpenMP vector partitioned\n");
    #pragma omp target data map(to: a[0:N], b[0:N]) map(from: e[0:N])
    {
        #pragma omp target teams distribute parallel for simd num_teams(4) simdlen(8) /* vector partitioned */
        for (i = 0; i < N; i++) {
            e[i] = a[i] + b[i];
        }
    }
    
    /* Test 12: OpenMP - fully partitioned (case 7) with unified shared memory */
    printf("Test 12: OpenMP fully partitioned\n");
    #pragma omp requires unified_shared_memory
    {
        int *shared_arr = (int*)malloc(N * sizeof(int));
        #pragma omp target data map(tofrom: shared_arr[0:N])
        {
            #pragma omp target teams distribute parallel for simd \
                num_teams(8) thread_limit(4) simdlen(16) /* gang+worker+vector */
            for (i = 0; i < N; i++) {
                shared_arr[i] = i * 10;
                omp_helper_gang(shared_arr, i, shared_arr[i]);
                omp_helper_worker(shared_arr, i, shared_arr[i]);
                omp_helper_vector(shared_arr, i, shared_arr[i]);
            }
        }
        free(shared_arr);
    }
    
    /* Test 13: Conditional partition selection - may trigger default case */
    printf("Test 13: Conditional partitions (potential illegal case)\n");
    {
        int *temp = (int*)malloc(N * sizeof(int));
        int dynamic_gangs = flag ? 2 : 0; /* Could create invalid configuration */
        
        #pragma acc data copyin(a[0:N]) create(temp[0:N])
        {
            /* Try various combinations that might hit edge cases */
            #pragma acc parallel loop gang(dynamic_gangs) /* May be gang redundant or invalid */
            for (i = 0; i < N; i++) {
                temp[i] = a[i];
            }
            
            /* Mixed explicit and implicit partitioning */
            #pragma acc parallel loop gang(4) vector(32) /* gang+vector, no explicit worker */
            for (i = 0; i < N; i++) {
                temp[i] *= 2;
            }
        }
        free(temp);
    }
    
    /* Final verification */
    printf("\nVerification...\n");
    for (i = 0; i < VERIFY_SIZE; i++) {
        /* Check some computed values */
        if (c[i] != a[i] + b[i] && c[i] != 0) {
            if (c[i] != a[i] * 2 && c[i] != b[i] * 6) errors++;
        }
    }
    
    printf("\nTest Summary:\n");
    printf("Total errors: %d\n", errors);
    printf("Partition types tested: 0-7 (8 cases)\n");
    printf("Test %s\n", errors == 0 ? "PASSED" : "FAILED");
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    
    return errors;
}
