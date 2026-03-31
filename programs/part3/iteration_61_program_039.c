/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
int flow_dependency(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read from src */
        dst[i] = temp + 1;  /* Write to dst - anti-dependency on temp if reused */
    }
    
    /* Force anti-dependency with explicit reuse */
    for (int i = 1; i < size; i++) {
        int x = dst[i-1];   /* Read */
        dst[i] = x * 2;     /* Write - creates anti-dependency */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * arr[i];
        result = t;  /* Output dependency on 'result' across iterations */
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

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on 'acc' */
        }
        total[j] = acc;
        
        /* Another inner loop with anti-dependency */
        for (int i = 1; i < N; i++) {
            int prev = matrix[j][i-1];  /* Read */
            matrix[j][i] = prev + 1;    /* Write - anti-dependency */
        }
    }
}

/* Complex pattern with multiple dependency types */
void mixed_dependencies(int *a, int *b, int *c, int size) {
    int accum = 0;
    int temp;
    
    for (int i = 0; i < size; i++) {
        /* Flow dependency */
        accum += a[i];
        
        /* Anti-dependency pattern */
        temp = b[i];
        c[i] = temp * accum;
        
        /* Control dependency */
        if (accum > 1000) {
            b[i] = 0;
        }
        
        /* Output-like dependency through pointer */
        *(&accum) = accum % 100;
    }
}

/* Loop with carried dependencies and pointer aliasing */
void pointer_based_dependencies(int *arr, int size) {
    int *ptr = arr;
    int sum = 0;
    
    for (int i = 0; i < size - 1; i++) {
        /* Flow dependency through pointer */
        sum += *ptr;
        ptr++;
        
        /* Anti-dependency through array access */
        arr[i+1] = sum + arr[i];
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)simple_rand() % 100;
        arr2[i] = (int)simple_rand() % 100;
        arr3[i] = 0;
    }
}

/* Main driver that exercises all dependency patterns */
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
    
    /* Initialize with reproducible data */
    init_arrays(array1, array2, array3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)simple_rand() % 100;
        }
    }
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependencies */
    int sum1 = flow_dependency(array1, N);
    printf("Flow dependency test: sum = %d\n", sum1);
    
    /* Test 2: Anti-dependencies */
    anti_dependency(array1, array3, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependencies */
    int result = output_dependency(array2, N);
    printf("Output dependency test: result = %d\n", result);
    
    /* Test 4: Control dependencies */
    control_dependency(array1, array3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Nested loop dependencies */
    nested_loop_dependencies(matrix, totals);
    printf("Nested loop test completed\n");
    
    /* Test 6: Mixed dependencies */
    mixed_dependencies(array1, array2, array3, N);
    printf("Mixed dependencies test completed\n");
    
    /* Test 7: Pointer-based dependencies */
    pointer_based_dependencies(array1, N);
    printf("Pointer-based dependencies test completed\n");
    
    /* Clean up */
    free(array1);
    free(array2);
    free(array3);
    
    printf("All DDG tests completed successfully\n");
    return 0;
}
