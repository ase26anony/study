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
        dst[i] = temp + 1;  /* Anti-dependency if src and dst alias */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
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
        if (a[i] > 0) {     /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies in a single loop */
int mixed_dependencies(int *a, int *b, int *c, int n) {
    int acc = 0;
    int temp;
    
    for (int i = 1; i < n; i++) {
        /* Flow dependency on acc */
        acc += a[i];
        
        /* Anti-dependency through temp variable */
        temp = b[i - 1];
        c[i] = temp * 2;
        
        /* Control dependency */
        if (acc > 1000) {
            b[i] = 0;
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
            acc += matrix[j][i];
        }
        totals[j] = acc;
        
        /* Second inner loop with anti-dependency */
        for (int i = 1; i < N; i++) {
            int temp = matrix[j][i - 1];
            matrix[j][i] = temp + 1;
        }
    }
}

/* Loop with pointer aliasing to create complex dependencies */
void pointer_aliasing_deps(int *ptr1, int *ptr2, int n) {
    /* Assume ptr1 and ptr2 may alias */
    for (int i = 0; i < n - 1; i++) {
        ptr1[i + 1] = ptr2[i] + ptr1[i];  /* Flow and potential anti-deps */
    }
}

/* Volatile variables to prevent optimization */
void volatile_dependencies(volatile int *in, volatile int *out, int n) {
    volatile int accum = 0;
    for (int i = 0; i < n; i++) {
        accum += in[i];      /* Flow dep with volatile */
        out[i] = accum;      /* Flow dep to output */
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = simple_rand() % 100;
        arr2[i] = simple_rand() % 100;
        arr3[i] = simple_rand() % 100;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int *array1, *array2, *array3;
    int matrix[M][N];
    int totals[M];
    
    /* Allocate and initialize data */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = simple_rand() % 100;
        }
    }
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = flow_dependency(array1, N);
    printf("Flow dependency test: sum = %d\n", sum1);
    
    /* Test 2: Anti-dependency */
    anti_dependency(array1, array2, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependency */
    int result = output_dependency(array1, N);
    printf("Output dependency test: result = %d\n", result);
    
    /* Test 4: Control dependency */
    control_dependency(array1, array3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Mixed dependencies */
    int mixed_result = mixed_dependencies(array1, array2, array3, N);
    printf("Mixed dependencies test: result = %d\n", mixed_result);
    
    /* Test 6: Nested loops */
    nested_loop_dependencies(matrix, totals);
    printf("Nested loop test completed\n");
    
    /* Test 7: Pointer aliasing */
    pointer_aliasing_deps(array1, array2, N);
    printf("Pointer aliasing test completed\n");
    
    /* Test 8: Volatile dependencies */
    volatile_dependencies(array1, array2, 100);
    printf("Volatile dependencies test completed\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    printf("All DDG tests completed successfully\n");
    return 0;
}
