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
        dst[i] = temp + 1;  /* Anti-dependency if 'temp' reused (but compiler may optimize) */
        /* Force anti-dependency by reusing variable */
        src[i] = dst[i] * 2; /* Write to src[i] after reading it earlier in iteration */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 3 + 7;
        result = computed;  /* Output dependency on 'result' across iterations */
        /* The write to 'result' in iteration i+1 depends on write in iteration i */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control dependency - branch depends on loop-variant value */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies in single loop */
int mixed_dependencies(int *a, int *b, int n) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < n; i++) {
        /* Flow dependency on 'acc' */
        acc += a[i];
        
        /* Anti-dependency through 'prev' */
        int current = b[i];
        b[i] = prev + current;
        prev = current;
        
        /* Control dependency */
        if (acc > 1000) {
            b[i] = 0;
        }
    }
    return acc;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int result[M]) {
    for (int j = 0; j < M; j++) {
        int sum = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            sum += matrix[j][i];
        }
        result[j] = sum;
        
        /* Additional anti-dependency in second inner loop */
        int prev = result[j];
        for (int i = 1; i < N; i++) {
            int temp = matrix[j][i];
            matrix[j][i-1] = temp + prev;  /* Anti: read temp, write to different location */
            prev = temp;
        }
    }
}

/* Loop with distance > 1 dependency */
void distance_dependency(int *a, int *b, int n) {
    /* b[i] depends on a[i+2] - creates edge with distance > 0 */
    for (int i = 0; i < n - 2; i++) {
        b[i] = a[i + 2] * 2;
    }
}

/* Initialize arrays with pseudo-random data */
void init_data(int *arr1, int *arr2, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
    }
}

/* Main function that exercises all dependency patterns */
int main(int argc, char *argv[]) {
    /* Allocate and initialize data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    
    int matrix[M][N];
    int result[M];
    
    /* Initialize with deterministic but varying data */
    init_data(arr1, arr2, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    /* Run all dependency tests */
    int total = 0;
    
    /* Test 1: Flow dependency */
    total += flow_dependency(arr1, N);
    
    /* Test 2: Anti-dependency */
    anti_dependency(arr1, arr2, N);
    
    /* Test 3: Output dependency */
    total += output_dependency(arr1, N);
    
    /* Test 4: Control dependency */
    control_dependency(arr1, arr3, N);
    
    /* Test 5: Mixed dependencies */
    total += mixed_dependencies(arr1, arr2, N);
    
    /* Test 6: Nested loops */
    nested_loop_dependencies(matrix, result);
    
    /* Test 7: Distance dependency */
    distance_dependency(arr1, arr2, N);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", total + result[0] + arr3[N-1]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
