/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency if temp reused */
        /* Force anti-dependency by reusing temp in next iteration */
        if (i < n - 1) {
            src[i + 1] = temp * 2;  /* Anti-dependency: write after read */
        }
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {  /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies - more complex pattern */
int mixed_dependencies(int *a, int *b, int n) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < n; i++) {
        int temp = a[i];
        
        /* Flow dependency on acc */
        acc += temp;
        
        /* Anti-dependency through prev */
        b[i] = prev + temp;
        prev = temp;  /* Creates flow to next iteration's anti-dependency */
        
        /* Control dependency */
        if (acc > 1000) {
            b[i] *= 2;
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
        
        /* Additional anti-dependency in second inner loop */
        int prev = 0;
        for (int i = 0; i < N; i++) {
            int temp = matrix[j][i];
            matrix[j][i] = prev + temp;  /* Anti-dependency through prev */
            prev = temp;
        }
    }
}

/* Loop with distance > 1 dependency */
void distance_dependency(int *a, int *b, int n) {
    /* Process with distance 2: read i, write i+2 */
    for (int i = 0; i < n - 2; i++) {
        b[i + 2] = a[i] * 2;  /* Distance 2 flow dependency */
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Allocate arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    
    int matrix[M][N];
    int totals[M];
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(arr1, arr2, N);
    
    /* Run all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* 1. Flow dependency */
    int sum = flow_dependency(arr1, N);
    printf("Flow dependency result: %d\n", sum);
    
    /* 2. Anti-dependency */
    anti_dependency(arr1, arr3, N);
    printf("Anti-dependency completed\n");
    
    /* 3. Output dependency */
    int out = output_dependency(arr2, N);
    printf("Output dependency result: %d\n", out);
    
    /* 4. Control dependency */
    control_dependency(arr1, arr3, N);
    printf("Control dependency completed\n");
    
    /* 5. Mixed dependencies */
    int mixed = mixed_dependencies(arr1, arr3, N);
    printf("Mixed dependencies result: %d\n", mixed);
    
    /* 6. Initialize matrix for nested loops */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 50);
        }
    }
    
    /* Nested loop dependencies */
    nested_loop_dependencies(matrix, totals);
    printf("Nested loop dependencies completed\n");
    
    /* 7. Distance dependency */
    distance_dependency(arr1, arr3, N);
    printf("Distance dependency completed\n");
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("All DDG test patterns executed\n");
    return 0;
}
