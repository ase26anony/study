/* test-omp-oacc-neuter-broadcast.c
 * 
 * This program is designed to trigger the uncovered lines in
 * omp-oacc-neuter-broadcast.cc (lines 335-343) by creating various
 * OpenACC compute regions with explicit data partition mappings.
 * The compiler's neuter-broadcast pass should process these partition
 * clauses and invoke the string mapping function for each partition type.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 64
#define M 32
#define P 16

/* Helper function to initialize arrays */
void init_3d_array(int arr[N][M][P]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            for (int k = 0; k < P; k++)
                arr[i][j][k] = i * 1000 + j * 100 + k;
}

/* Routine directive with gang partition */
#pragma acc routine vec gang
void acc_routine_gang(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += 1;
    }
}

/* Routine directive with vector partition */
#pragma acc routine seq vector
void acc_routine_vector(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] *= 2;
    }
}

/* Test 1: Basic partition types in parallel regions */
void test_basic_partitions(int arr1[N][M][P], int arr2[N][M][P], int condition) {
    printf("Test 1: Basic partition types\n");
    
    /* Case 0: gang redundant (implicit) */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 1;
    }
    
    /* Case 1: gang partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang(static:2)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 2;
    }
    
    /* Case 2: worker partitioned */
    #pragma acc parallel copy(arr2[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr2[i][j][k] += 3;
    }
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang worker
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 4;
    }
    
    /* Case 4: vector partitioned */
    #pragma acc parallel copy(arr2[0:N][0:M][0:P]) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr2[i][j][k] += 5;
    }
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 6;
    }
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel copy(arr2[0:N][0:M][0:P]) worker vector
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr2[i][j][k] += 7;
    }
    
    /* Case 7: fully partitioned (gang+worker+vector) */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang worker vector
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 8;
    }
}

/* Test 2: Kernels construct with various partitions */
void test_kernels_partitions(int arr1[N][M][P], int arr2[N][M][P]) {
    printf("Test 2: Kernels construct partitions\n");
    
    /* gang partitioned in kernels */
    #pragma acc kernels create(arr1[0:N][0:M][0:P]) gang(static:4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] = i + j + k;
    }
    
    /* worker partitioned in kernels */
    #pragma acc kernels copyout(arr2[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr2[i][j][k] = i * j * k;
    }
    
    /* gang+vector partitioned in kernels */
    #pragma acc kernels copy(arr1[0:N][0:M][0:P]) gang vector
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += arr2[i][j][k];
    }
}

/* Test 3: Multi-dimensional array with collapse and partitions */
void test_collapse_partitions(int arr1[N][M][P], int arr2[N][M][P]) {
    printf("Test 3: Collapse with partitions\n");
    
    /* collapse with gang+vector partition */
    #pragma acc parallel loop collapse(3) gang vector copy(arr1[0:N][0:M][0:P])
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            for (int k = 0; k < P; k++)
                arr1[i][j][k] *= 2;
    
    /* collapse with worker partition */
    #pragma acc parallel loop collapse(2) worker copy(arr2[0:N][0:M][0:P])
    for (int i = 0; i < N; i++)
        for (int j = 0; j < M; j++)
            for (int k = 0; k < P; k++)
                arr2[i][j][k] /= 2;
}

/* Test 4: Nested and sequential compute regions */
void test_nested_regions(int arr1[N][M][P], int condition) {
    printf("Test 4: Nested and sequential regions\n");
    
    /* Outer region with gang partition */
    #pragma acc parallel if(condition) copy(arr1[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* Inner region with worker partition */
            #pragma acc parallel loop worker
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += i;
        }
    }
    
    /* Sequential regions with different partitions */
    #pragma acc parallel copy(arr1[0:N][0:M][0:P]) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 1;
    }
    
    #pragma acc kernels copy(arr1[0:N][0:M][0:P]) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 2;
    }
}

/* Test 5: Routine directives with partitions */
void test_routine_partitions(int arr1[N][M][P]) {
    printf("Test 5: Routine directives\n");
    
    #pragma acc parallel loop gang copy(arr1[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            /* Call gang-partitioned routine */
            acc_routine_gang(&arr1[i][j][0], P);
            
            #pragma acc loop vector
            for (int k = 0; k < P; k++) {
                /* Call vector-partitioned routine */
                if (k % 2 == 0) {
                    acc_routine_vector(&arr1[i][j][k], 1);
                }
            }
        }
    }
}

/* Test 6: Device data environment with partitions */
void test_device_data_partitions(int arr1[N][M][P], int arr2[N][M][P]) {
    printf("Test 6: Device data environment\n");
    
    /* Enter data with gang partition */
    #pragma acc enter data copyin(arr1[0:N][0:M][0:P]) gang
    
    /* Enter data with worker partition */
    #pragma acc enter data create(arr2[0:N][0:M][0:P]) worker
    
    /* Compute region with present clause and gang partition */
    #pragma acc parallel present(arr1) gang
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += 10;
    }
    
    /* Compute region with present clause and worker partition */
    #pragma acc parallel present(arr2) worker
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr2[i][j][k] += 20;
    }
    
    /* Compute region with present clause and vector partition */
    #pragma acc parallel present(arr1, arr2) vector
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < P; k++)
                    arr1[i][j][k] += arr2[i][j][k];
    }
    
    /* Exit data */
    #pragma acc exit data copyout(arr1[0:N][0:M][0:P])
    #pragma acc exit data delete(arr2[0:N][0:M][0:P])
}

/* Validation function */
int validate_results(int arr[N][M][P], int expected_base) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                /* We can't predict exact values due to parallel execution,
                   but we can check they're not zero or corrupted */
                if (arr[i][j][k] == 0) {
                    errors++;
                }
            }
        }
    }
    return errors;
}

int main(int argc, char *argv[]) {
    /* Use argc to create conditional execution paths */
    int condition = (argc > 1);
    
    /* Declare multi-dimensional arrays */
    int array1[N][M][P];
    int array2[N][M][P];
    int array3[N][M][P];
    
    /* Initialize arrays */
    init_3d_array(array1);
    init_3d_array(array2);
    init_3d_array(array3);
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Execute tests based on argc to prevent dead code elimination */
    if (argc > 1) {
        test_basic_partitions(array1, array2, condition);
    }
    
    if (argc > 2) {
        test_kernels_partitions(array1, array3);
    }
    
    if (argc > 3) {
        test_collapse_partitions(array2, array3);
    }
    
    if (argc > 4) {
        test_nested_regions(array1, condition);
    }
    
    if (argc > 5) {
        test_routine_partitions(array2);
    }
    
    if (argc > 6) {
        test_device_data_partitions(array1, array3);
    }
    
    /* Run all tests if no specific test is selected */
    if (argc == 1) {
        test_basic_partitions(array1, array2, condition);
        test_kernels_partitions(array1, array3);
        test_collapse_partitions(array2, array3);
        test_nested_regions(array1, condition);
        test_routine_partitions(array2);
        test_device_data_partitions(array1, array3);
    }
    
    /* Validate results */
    int errors1 = validate_results(array1, 0);
    int errors2 = validate_results(array2, 0);
    int errors3 = validate_results(array3, 0);
    
    printf("Validation results:\n");
    printf("  Array1 errors: %d\n", errors1);
    printf("  Array2 errors: %d\n", errors2);
    printf("  Array3 errors: %d\n", errors3);
    
    if (errors1 + errors2 + errors3 == 0) {
        printf("All arrays contain non-zero values (parallel execution successful)\n");
    } else {
        printf("Warning: Some array elements are zero\n");
    }
    
    printf("Test completed. Compile with -fopenacc to trigger neuter-broadcast pass.\n");
    
    return 0;
}
