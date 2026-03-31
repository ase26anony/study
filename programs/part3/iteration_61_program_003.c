/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int sink;

/* Flow dependency (TRUE_DEP) - classic accumulator */
int flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum; /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp if reused */
        /* Force anti-dependency by reusing temp in next iteration */
        if (i < n-1) {
            src[i+1] = temp * 2; /* Creates anti-dep between iterations */
        }
    }
    sink = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * i;
        result = t;  /* Output dependency on 'result' across iterations */
        /* Use result to prevent elimination */
        arr[i] = result % 256;
    }
    sink = result;
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control-dep on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[n-1];
}

/* Nested loops with inner loop dependencies */
int nested_flow_dep(int matrix[M][N]) {
    int total[M];
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dep in inner loop */
        }
        total[j] = acc;
    }
    
    int final_sum = 0;
    for (int j = 0; j < M; j++) {
        final_sum += total[j];
    }
    sink = final_sum;
    return final_sum;
}

/* Complex pattern with multiple dependency types */
void mixed_deps(int *a, int *b, int *c, int n) {
    int accum = a[0];
    
    for (int i = 1; i < n; i++) {
        /* Flow dependency on accum */
        accum = accum + a[i];
        
        /* Anti-dependency: read b[i], then write to a[i] */
        int tmp = b[i];
        a[i] = tmp * 2;
        
        /* Control dependency */
        if (accum > 1000) {
            c[i] = 1;
        } else {
            c[i] = 0;
        }
        
        /* Output-like dependency through array */
        b[i-1] = accum % 256;
    }
    sink = accum;
}

/* Initialize arrays with pseudo-random values */
void init_arrays(int *arr1, int *arr2, int *arr3, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (seed >> 16) & 0x7FFF;
        arr2[i] = (seed >> 8) & 0xFF;
        arr3[i] = i;
    }
}

int main(int argc, char **argv) {
    /* Allocate arrays */
    int *arr1 = malloc(N * sizeof(int));
    int *arr2 = malloc(N * sizeof(int));
    int *arr3 = malloc(N * sizeof(int));
    int matrix[M][N];
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(arr1, arr2, arr3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (i * j) % 100;
        }
    }
    
    /* Run all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = flow_dep(arr1, N);
    printf("Flow dep result: %d\n", sum1);
    
    /* Test 2: Anti-dependency */
    anti_dep(arr1, arr2, N);
    printf("Anti dep completed\n");
    
    /* Test 3: Output dependency */
    int sum2 = output_dep(arr3, N);
    printf("Output dep result: %d\n", sum2);
    
    /* Test 4: Control dependency */
    control_dep(arr1, arr2, N);
    printf("Control dep completed\n");
    
    /* Test 5: Nested loops */
    int sum3 = nested_flow_dep(matrix);
    printf("Nested flow dep result: %d\n", sum3);
    
    /* Test 6: Mixed dependencies */
    mixed_deps(arr1, arr2, arr3, N);
    printf("Mixed deps completed\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
