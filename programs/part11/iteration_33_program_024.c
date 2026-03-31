/* Test program to exercise OpenACC partitioning classification logic
 * Specifically targets the switch cases in omp-oacc-neuter-broadcast.cc
 * lines 335-343 for different partitioning types.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Case 0: gang redundant - parallel region without loop or gang(1) */
void test_gang_redundant(float *data) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(data[0:N]) copyin(local_sum)
    {
        /* No associated loop - should be gang redundant */
        #pragma acc loop gang(1)
        for (int i = 0; i < N; i++) {
            data[i] = i * 0.5f;
        }
        
        /* Simple assignment in parallel region */
        local_sum = 42.0f;
    }
    
    printf("Gang redundant test: data[0]=%.2f, data[%d]=%.2f\n", 
           data[0], N-1, data[N-1]);
}

/* Case 1: gang partitioned - loop with explicit gang clause */
void test_gang_partitioned(float *data) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(data[0:N]) reduction(+:sum)
    for (int i = 0; i < N; i++) {
        data[i] = data[i] * 2.0f + i;
        sum += data[i];
    }
    
    printf("Gang partitioned test: sum=%.2f\n", sum);
}

/* Case 2: worker partitioned - inner loop with worker clause */
void test_worker_partitioned(float data[M][M]) {
    #pragma acc parallel copy(data[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                data[i][j] = (float)(i + j);
            }
        }
    }
    
    printf("Worker partitioned test: data[0][0]=%.2f, data[%d][%d]=%.2f\n",
           data[0][0], M-1, M-1, data[M-1][M-1]);
}

/* Case 3: gang+worker partitioned - nested loops with both clauses */
void test_gang_worker_partitioned(float data[M][M]) {
    #pragma acc parallel copy(data[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                data[i][j] = data[i][j] * 0.5f + (i * j);
            }
        }
    }
    
    printf("Gang+worker partitioned test: data[10][10]=%.2f\n", data[10][10]);
}

/* Case 4: vector partitioned - loop with vector clause */
void test_vector_partitioned(float *data) {
    #pragma acc parallel loop vector copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = data[i] * data[i] - 1.0f;
    }
    
    printf("Vector partitioned test: data[100]=%.2f\n", data[100]);
}

/* Case 5: gang+vector partitioned - loop with both gang and vector */
void test_gang_vector_partitioned(float *data) {
    #pragma acc parallel loop gang vector copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = sinf(data[i]) * 100.0f;
    }
    
    printf("Gang+vector partitioned test: data[500]=%.2f\n", data[500]);
}

/* Case 6: worker+vector partitioned - nested loops with worker and vector */
void test_worker_vector_partitioned(float data[M][M]) {
    #pragma acc parallel copy(data[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                data[i][j] = cosf(data[i][j]) * 50.0f;
            }
        }
    }
    
    printf("Worker+vector partitioned test: data[5][5]=%.2f\n", data[5][5]);
}

/* Case 7: fully partitioned - triple nested loop with all three levels */
void test_fully_partitioned(float data[M][M]) {
    float temp[M][M];
    
    /* Initialize temp array */
    #pragma acc parallel loop gang copy(temp[0:M][0:M])
    for (int i = 0; i < M; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            temp[i][j] = (float)(i * j);
        }
    }
    
    /* Complex stencil computation requiring all three levels */
    #pragma acc parallel copy(data[0:M][0:M]) copyin(temp[0:M][0:M])
    {
        #pragma acc loop gang
        for (int i = 1; i < M-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < M-1; j++) {
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {  /* Small vector loop */
                    /* Stencil-like computation with data dependencies */
                    data[i][j] = (temp[i-1][j] + temp[i][j-1] + 
                                  temp[i+1][j] + temp[i][j+1]) * 0.25f;
                }
            }
        }
    }
    
    printf("Fully partitioned test: data[15][15]=%.2f\n", data[15][15]);
}

/* Helper function with conditional execution to ensure compiler analysis */
void conditional_test(int test_id, float *arr1, float arr2[M][M]) {
    /* Use argc/argv simulation to force compiler to analyze both paths */
    if (test_id > 0) {
        switch (test_id % 8) {
            case 0: test_gang_redundant(arr1); break;
            case 1: test_gang_partitioned(arr1); break;
            case 2: test_worker_partitioned(arr2); break;
            case 3: test_gang_worker_partitioned(arr2); break;
            case 4: test_vector_partitioned(arr1); break;
            case 5: test_gang_vector_partitioned(arr1); break;
            case 6: test_worker_vector_partitioned(arr2); break;
            case 7: test_fully_partitioned(arr2); break;
        }
    } else {
        /* Host fallback path - compiler must analyze this too */
        for (int i = 0; i < N; i++) arr1[i] = 0.0f;
        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++) arr2[i][j] = 0.0f;
    }
}

int main(int argc, char *argv[]) {
    float array1[N];
    float array2[M][M];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = (float)i / N;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            array2[i][j] = (float)(i + j) / (2 * M);
        }
    }
    
    /* Execute tests based on command-line arguments */
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run all tests conditionally to ensure compiler sees all patterns */
    for (int i = 0; i < 8; i++) {
        conditional_test((test_to_run + i) % 9, array1, array2);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Final check: array1[0]=%.4f, array1[%d]=%.4f\n", 
           array1[0], N-1, array1[N-1]);
    printf("Final check: array2[0][0]=%.4f, array2[%d][%d]=%.4f\n",
           array2[0][0], M-1, M-1, array2[M-1][M-1]);
    
    return 0;
}
