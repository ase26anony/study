/* ddg_coverage.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
int flow_dependency(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency: sum read, modified, read next iteration */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp if it's reused */
    }
    /* Force temp to be used to prevent optimization */
    dst[0] += temp;
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int size) {
    int result = 0;
    int intermediate;
    for (int i = 0; i < size; i++) {
        intermediate = arr[i] * 2;  /* Write to intermediate */
        result = intermediate;       /* Output dep: result written each iteration */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies - more complex pattern */
int mixed_dependencies(int *a, int *b, int size) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < size; i++) {
        int temp = a[i];          /* Anti-dep if temp reused (but it's not) */
        acc += temp;              /* Flow dep on acc */
        b[i] = prev;              /* Flow dep on prev (carried across iterations) */
        prev = temp;              /* Output dep on prev? Actually flow to next iter */
        
        /* Control dep inside loop */
        if (acc > 1000) {
            acc = acc / 2;
        }
    }
    return acc;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on acc */
        }
        totals[j] = acc;
        
        /* Another inner loop with anti-dependency */
        for (int i = 1; i < N; i++) {
            int temp = matrix[j][i-1];  /* Read */
            matrix[j][i] = temp + 1;    /* Write - anti-dep through temp */
        }
    }
}

/* Loop with pointer aliasing (creates ambiguous dependencies) */
void pointer_aliasing(int *a, int *b, int *c, int size) {
    for (int i = 1; i < size; i++) {
        a[i] = b[i] + c[i];      /* Flow deps on b[i], c[i] */
        b[i] = a[i-1] * 2;       /* Flow dep on a[i-1] across iterations */
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = (int)(lcg_rand() % 100);
    }
}

/* Main function that exercises all dependency patterns */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *array3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int totals[M];
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    int result = 0;
    
    /* Test different dependency patterns based on command line */
    if (argc > 1) {
        int test_type = atoi(argv[1]);
        switch (test_type) {
            case 1:
                result = flow_dependency(array1, N);
                printf("Flow dependency test: %d\n", result);
                break;
            case 2:
                anti_dependency(array1, array2, N);
                printf("Anti-dependency test complete\n");
                break;
            case 3:
                result = output_dependency(array1, N);
                printf("Output dependency test: %d\n", result);
                break;
            case 4:
                control_dependency(array1, array2, N);
                printf("Control dependency test complete\n");
                break;
            case 5:
                result = mixed_dependencies(array1, array2, N);
                printf("Mixed dependencies test: %d\n", result);
                break;
            case 6:
                nested_loop_dependencies(matrix, totals);
                printf("Nested loop test complete\n");
                break;
            case 7:
                pointer_aliasing(array1, array2, array3, N);
                printf("Pointer aliasing test complete\n");
                break;
            default:
                /* Run all tests */
                result = flow_dependency(array1, N);
                anti_dependency(array1, array2, N);
                result += output_dependency(array1, N);
                control_dependency(array1, array2, N);
                result += mixed_dependencies(array1, array2, N);
                nested_loop_dependencies(matrix, totals);
                pointer_aliasing(array1, array2, array3, N);
                printf("All tests completed, final result: %d\n", result);
                break;
        }
    } else {
        /* Default: run all tests */
        result = flow_dependency(array1, N);
        anti_dependency(array1, array2, N);
        result += output_dependency(array1, N);
        control_dependency(array1, array2, N);
        result += mixed_dependencies(array1, array2, N);
        nested_loop_dependencies(matrix, totals);
        pointer_aliasing(array1, array2, array3, N);
        printf("All tests completed, final result: %d\n", result);
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
