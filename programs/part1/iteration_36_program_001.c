/* test_partition_cases.c - Cover all partition mapping switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openacc.h>

#define ARRAY_SIZE 1024
#define MAX_GANGS 8
#define MAX_WORKERS 4
#define VECTOR_LENGTH 32

/* Helper to prevent optimization */
static void use_result(int val) {
    asm volatile("" : : "r"(val));
}

/* Test case 0: gang redundant (single gang) */
void test_gang_redundant() {
    volatile int n = ARRAY_SIZE;
    int *data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:1) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    /* Force partition logic */
    int check = 0;
    #pragma acc parallel loop gang(num:1) reduction(+:check)
    for (int i = 0; i < n; i++) {
        check += (data[i] == i * 2);
    }
    
    if (check != n) {
        /* This could trigger error string generation */
        fprintf(stderr, "Gang redundant test failed: %d/%d\n", check, n);
    }
    
    use_result(check);
    free(data);
}

/* Test case 1: gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    volatile int n = ARRAY_SIZE;
    int *data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:MAX_GANGS) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = acc_gang_id() * 1000 + i;
    }
    
    /* Cross-gang verification */
    int errors = 0;
    #pragma acc parallel loop gang(num:MAX_GANGS) reduction(+:errors)
    for (int i = 0; i < n; i++) {
        int expected_gang = (i * MAX_GANGS) / n;
        if (data[i] / 1000 != expected_gang) errors++;
    }
    
    if (errors > 0) {
        fprintf(stderr, "Gang partitioned errors: %d\n", errors);
    }
    
    use_result(errors);
    free(data);
}

/* Test case 2: worker partitioned (single gang, multiple workers) */
void test_worker_partitioned() {
    volatile int n = ARRAY_SIZE;
    int *data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:1) worker(num:MAX_WORKERS) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = acc_worker_id() * 100 + i;
    }
    
    int worker_counts[MAX_WORKERS] = {0};
    #pragma acc parallel loop gang(num:1) worker(num:MAX_WORKERS) copyout(worker_counts[0:MAX_WORKERS])
    for (int i = 0; i < n; i++) {
        int wid = data[i] / 100;
        if (wid < MAX_WORKERS) worker_counts[wid]++;
    }
    
    use_result(worker_counts[0]);
    free(data);
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned() {
    volatile int n = ARRAY_SIZE;
    int *data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel loop gang(num:MAX_GANGS) worker(num:MAX_WORKERS) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = acc_gang_id() * 10000 + acc_worker_id() * 100 + i;
    }
    
    /* Complex dependency requiring both gang and worker partitioning */
    int *prefix_sum = (int*)malloc(n * sizeof(int));
    #pragma acc parallel loop gang(num:MAX_GANGS) worker(num:MAX_WORKERS) copyout(prefix_sum[0:n])
    for (int i = 0; i < n; i++) {
        int sum = 0;
        #pragma acc loop worker reduction(+:sum)
        for (int j = 0; j <= i % 16; j++) {
            sum += data[(i + j) % n] % 100;
        }
        prefix_sum[i] = sum;
    }
    
    use_result(prefix_sum[n-1]);
    free(data);
    free(prefix_sum);
}

/* Test case 4: vector partitioned */
void test_vector_partitioned() {
    volatile int n = ARRAY_SIZE;
    float *data = (float*)malloc(n * sizeof(float));
    
    #pragma acc parallel loop vector_length(VECTOR_LENGTH) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = (float)i * 1.5f;
    }
    
    /* Vector operations */
    float sum = 0.0f;
    #pragma acc parallel loop vector_length(VECTOR_LENGTH) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    
    use_result((int)sum);
    free(data);
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned() {
    volatile int n = ARRAY_SIZE;
    float *data = (float*)malloc(n * sizeof(float));
    
    #pragma acc parallel loop gang(num:MAX_GANGS) vector_length(VECTOR_LENGTH) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = acc_gang_id() * 10.0f + (float)(i % VECTOR_LENGTH) * 0.1f;
    }
    
    /* Vector reduction within gangs */
    float gang_sums[MAX_GANGS] = {0};
    #pragma acc parallel loop gang(num:MAX_GANGS) vector_length(VECTOR_LENGTH) copyout(gang_sums[0:MAX_GANGS])
    for (int g = 0; g < MAX_GANGS; g++) {
        float local_sum = 0.0f;
        #pragma acc loop vector reduction(+:local_sum)
        for (int i = g * (n/MAX_GANGS); i < (g+1) * (n/MAX_GANGS); i++) {
            local_sum += data[i];
        }
        gang_sums[g] = local_sum;
    }
    
    use_result((int)gang_sums[0]);
    free(data);
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned() {
    volatile int n = ARRAY_SIZE;
    float *data = (float*)malloc(n * sizeof(float));
    
    #pragma acc parallel loop gang(num:1) worker(num:MAX_WORKERS) vector_length(VECTOR_LENGTH) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = acc_worker_id() * 5.0f + (float)(i % VECTOR_LENGTH) * 0.2f;
    }
    
    /* Worker-vector nested computation */
    float worker_results[MAX_WORKERS] = {0};
    #pragma acc parallel loop gang(num:1) worker(num:MAX_WORKERS) vector_length(VECTOR_LENGTH) copyout(worker_results[0:MAX_WORKERS])
    for (int w = 0; w < MAX_WORKERS; w++) {
        float vec_max = 0.0f;
        #pragma acc loop vector reduction(max:vec_max)
        for (int i = w * (n/MAX_WORKERS); i < (w+1) * (n/MAX_WORKERS); i++) {
            if (data[i] > vec_max) vec_max = data[i];
        }
        worker_results[w] = vec_max;
    }
    
    use_result((int)worker_results[0]);
    free(data);
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    volatile int n = ARRAY_SIZE;
    float *data = (float*)malloc(n * sizeof(float));
    
    #pragma acc parallel loop gang(num:MAX_GANGS) worker(num:MAX_WORKERS) vector_length(VECTOR_LENGTH) copyout(data[0:n])
    for (int i = 0; i < n; i++) {
        data[i] = acc_gang_id() * 100.0f + acc_worker_id() * 10.0f + (float)(i % VECTOR_LENGTH) * 0.5f;
    }
    
    /* Three-level nested reduction */
    float final_sum = 0.0f;
    #pragma acc parallel loop gang(num:MAX_GANGS) worker(num:MAX_WORKERS) vector_length(VECTOR_LENGTH) reduction(+:final_sum)
    for (int i = 0; i < n; i++) {
        /* Complex computation requiring all three levels */
        float val = data[i];
        #pragma acc loop worker reduction(+:val)
        for (int w = 0; w < 2; w++) {
            #pragma acc loop vector reduction(+:val)
            for (int v = 0; v < 2; v++) {
                val += 0.1f * (w + v);
            }
        }
        final_sum += val;
    }
    
    use_result((int)final_sum);
    free(data);
}

/* Test default case through boundary conditions */
void test_invalid_partition() {
    /* Force compiler to consider invalid partition codes */
    volatile int invalid_code = 8;  /* Outside 0-7 range */
    
    /* Use runtime value to select partition type */
    int n = 64;
    int *data = (int*)malloc(n * sizeof(int));
    
    /* This may generate default case handling */
    switch (invalid_code) {
        case 0: case 1: case 2: case 3:
        case 4: case 5: case 6: case 7:
            /* Valid cases handled elsewhere */
            break;
        default:
            /* Could trigger default string generation */
            #pragma acc parallel loop copyout(data[0:n])
            for (int i = 0; i < n; i++) {
                data[i] = i + invalid_code;
            }
            break;
    }
    
    /* Force materialization of error path */
    if (invalid_code > 7) {
        fprintf(stderr, "Invalid partition code: %d\n", invalid_code);
    }
    
    use_result(data[0]);
    free(data);
}

/* OpenMP equivalents to trigger similar logic */
#ifdef _OPENMP
void test_omp_partitioning() {
    int n = ARRAY_SIZE;
    int *data = (int*)malloc(n * sizeof(int));
    
    /* OpenMP target offload with various partitioning */
    #pragma omp target teams distribute parallel for map(from:data[0:n]) num_teams(MAX_GANGS) thread_limit(MAX_WORKERS*VECTOR_LENGTH)
    for (int i = 0; i < n; i++) {
        data[i] = omp_get_team_num() * 1000 + omp_get_thread_num() * 10 + i;
    }
    
    /* Nested SIMD for vector partitioning */
    #pragma omp target teams distribute simd map(from:data[0:n]) num_teams(MAX_GANGS)
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2;
    }
    
    use_result(data[n-1]);
    free(data);
}
#endif

int main() {
    printf("Testing all partition mapping cases...\n");
    
    /* Execute all test cases systematically */
    test_gang_redundant();          /* Case 0 */
    test_gang_partitioned();        /* Case 1 */
    test_worker_partitioned();      /* Case 2 */
    test_gang_worker_partitioned(); /* Case 3 */
    test_vector_partitioned();      /* Case 4 */
    test_gang_vector_partitioned(); /* Case 5 */
    test_worker_vector_partitioned(); /* Case 6 */
    test_fully_partitioned();       /* Case 7 */
    test_invalid_partition();       /* Default case */
    
    #ifdef _OPENMP
    test_omp_partitioning();        /* Additional OpenMP coverage */
    #endif
    
    printf("All partition tests completed.\n");
    
    /* Final check that forces compiler to consider all paths */
    volatile int check_code = 0;
    #pragma acc parallel loop gang(num:1) reduction(+:check_code)
    for (int i = 0; i < 8; i++) {
        check_code += i;
    }
    
    return check_code == 28 ? 0 : 1;
}
