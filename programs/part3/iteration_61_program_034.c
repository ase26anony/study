/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int force_volatile = 0;

/* Flow dependency (TRUE_DEP) - classic accumulator */
int flow_dependency(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum + force_volatile;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Anti-dependency if 'temp' reused (register pressure) */
        /* Force reuse of temp to create anti-dependency */
        src[i] = temp * 2;  /* Write to src[i] after reading it */
    }
    force_volatile = temp;  /* Prevent dead code elimination */
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * arr[i];  /* Computation */
        result = t;  /* Output dependency on 'result' across iterations */
        arr[i] = result;  /* Use result to prevent elimination */
    }
    return result + force_volatile;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > 0) {  /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    force_volatile = b[0];
}

/* Nested loops with inner loop dependencies */
int nested_loop_dependency(int matrix[M][N]) {
    int total[M];
    int outer_sum = 0;
    
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on 'acc' */
        }
        total[j] = acc;
        outer_sum += acc;
    }
    return outer_sum + force_volatile;
}

/* Complex loop with multiple dependency types */
void mixed_dependencies(int *a, int *b, int *c, int size) {
    int temp = 0;
    for (int i = 1; i < size; i++) {
        /* Flow dependency on b[i-1] -> b[i] */
        b[i] = b[i-1] + a[i];
        
        /* Anti-dependency: read a[i], then modify it */
        temp = a[i];
        a[i] = temp * 2;
        
        /* Control dependency */
        if (b[i] > 100) {
            c[i] = 1;
        } else {
            c[i] = 0;
        }
    }
    force_volatile = temp;
}

/* Initialize arrays with pseudo-random values (no external libs) */
void init_arrays(int *arr1, int *arr2, int *arr3, int size) {
    unsigned int seed = 12345;
    for (int i = 0; i < size; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (seed >> 16) & 0x7FFF;
        arr2[i] = i;
        arr3[i] = 0;
    }
}

/* Initialize matrix with values */
void init_matrix(int matrix[M][N]) {
    unsigned int seed = 54321;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            seed = seed * 1103515245 + 12345;
            matrix[j][i] = (seed >> 16) & 0xFF;
        }
    }
}

int main(int argc, char **argv) {
    int *array1, *array2, *array3;
    int matrix[M][N];
    int result;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, N);
    init_matrix(matrix);
    
    /* Run all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* 1. Flow dependency */
    result = flow_dependency(array1, N);
    printf("Flow dependency result: %d\n", result);
    
    /* 2. Anti-dependency */
    anti_dependency(array1, array2, N);
    printf("Anti-dependency completed\n");
    
    /* 3. Output dependency */
    result = output_dependency(array1, N);
    printf("Output dependency result: %d\n", result);
    
    /* 4. Control dependency */
    control_dependency(array1, array3, N);
    printf("Control dependency completed\n");
    
    /* 5. Nested loop dependency */
    result = nested_loop_dependency(matrix);
    printf("Nested loop dependency result: %d\n", result);
    
    /* 6. Mixed dependencies */
    mixed_dependencies(array1, array2, array3, N);
    printf("Mixed dependencies completed\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
