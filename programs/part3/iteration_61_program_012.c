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
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp if reused */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;  /* Compute temporary */
        result = t;          /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies - complex pattern */
int mixed_dependencies(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    for (int i = 1; i < n; i++) {
        /* Flow dep on acc1, anti-dep on arr[i-1] through temp */
        int temp = arr[i - 1];
        acc1 = acc1 + temp;
        
        /* Control dep on acc1 value */
        if (acc1 > 1000) {
            acc2 += arr[i];
        }
    }
    return acc1 + acc2;
}

/* Nested loops with inner loop dependencies */
void nested_loop_deps(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on 'acc' */
        }
        totals[j] = acc;
        
        /* Anti-dependency in second inner loop */
        for (int i = 1; i < N; i++) {
            int prev = matrix[j][i - 1];  /* Read */
            matrix[j][i] = prev + 1;      /* Write - anti-dep through 'prev' */
        }
    }
}

/* Loop with pointer aliasing (creates ambiguous dependencies) */
void pointer_aliasing_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        a[i] = b[i] + c[i];      /* Flow dep on b[i], c[i] */
        b[i] = a[i - 1] * 2;     /* Flow dep on a[i-1], anti-dep on b[i] */
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = (int)(lcg_rand() % 100);
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Allocate and initialize test data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int totals[M];
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with reproducible data */
    init_arrays(arr1, arr2, arr3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = flow_dependency(arr1, N);
    printf("Flow dependency test: sum = %d\n", sum1);
    
    /* Test 2: Anti-dependency */
    anti_dependency(arr1, arr2, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependency */
    int result = output_dependency(arr1, N);
    printf("Output dependency test: result = %d\n", result);
    
    /* Test 4: Control dependency */
    control_dependency(arr1, arr2, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Mixed dependencies */
    int mixed = mixed_dependencies(arr1, N);
    printf("Mixed dependencies test: result = %d\n", mixed);
    
    /* Test 6: Nested loops */
    nested_loop_deps(matrix, totals);
    printf("Nested loop dependencies test completed\n");
    
    /* Test 7: Pointer aliasing */
    pointer_aliasing_deps(arr1, arr2, arr3, N);
    printf("Pointer aliasing test completed\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("All DDG tests completed successfully\n");
    return 0;
}
