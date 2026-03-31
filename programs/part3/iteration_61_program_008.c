/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 64

/* Prevent aggressive optimization */
static volatile int force_keep = 0;

/* Flow dependency: TRUE_DEP */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    force_keep = sum;  /* Prevent dead code elimination */
    return sum;
}

/* Anti dependency: ANTI_DEP */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp if it persists */
    }
    force_keep = dst[n-1];
}

/* Output dependency: OUTPUT_DEP */
int output_dependency(int *arr, int n) {
    int result = 0;
    int temp;
    for (int i = 0; i < n; i++) {
        temp = arr[i] * 2;  /* Multiple writes to 'temp' create output dep */
        result = temp;      /* Flow dep on temp, output dep on result */
    }
    force_keep = result;
    return result;
}

/* Control dependency: CONTROL_DEP */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {     /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    force_keep = b[n-1];
}

/* Mixed dependencies with loop-carried flow */
int mixed_dependencies(int *arr, int n) {
    int acc = 0;
    int prev = 0;
    for (int i = 0; i < n; i++) {
        int curr = arr[i];
        acc += curr;        /* Flow dep on acc */
        if (prev > 0) {     /* Control dep on prev */
            acc += 1;       /* Additional flow dep */
        }
        prev = curr;        /* Anti dep on curr? Actually flow to prev */
    }
    force_keep = acc;
    return acc;
}

/* Nested loops with inner loop dependencies */
void nested_loop_deps(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        totals[j] = acc;
        
        /* Add some control flow in outer loop */
        if (j % 2 == 0) {
            totals[j] *= 2;  /* Control dep in outer loop */
        }
    }
    force_keep = totals[M-1];
}

/* Complex loop with multiple interleaved dependencies */
void complex_dependency_pattern(int *a, int *b, int *c, int n) {
    int x = 0, y = 0;
    for (int i = 1; i < n; i++) {
        /* Flow: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Anti: Read b[i] before potentially modifying it */
        int tmp = b[i];
        
        /* Output: Multiple writes to y */
        y = tmp * 2;
        
        /* Control: Depends on a[i] */
        if (a[i] > 100) {
            c[i] = y + 1;
        } else {
            c[i] = y - 1;
        }
        
        /* Modify b for next iteration (creates output/flow to next iter) */
        b[i] = c[i] / 2;
    }
    force_keep = a[n-1] + b[n-1] + c[n-1];
}

/* Initialize arrays with pseudo-random values */
void init_array(int *arr, int n, int seed) {
    unsigned int state = seed;
    for (int i = 0; i < n; i++) {
        /* Simple LCG to avoid library dependencies */
        state = state * 1103515245 + 12345;
        arr[i] = (state >> 16) & 0x7FFF;
    }
}

/* Initialize 2D array */
void init_matrix(int matrix[M][N], int seed) {
    unsigned int state = seed;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            state = state * 1103515245 + 12345;
            matrix[j][i] = (state >> 16) & 0xFF;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int *arr4 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    int totals[M];
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_array(arr1, N, 42);
    init_array(arr2, N, 123);
    init_array(arr3, N, 456);
    init_array(arr4, N, 789);
    init_matrix(matrix, 999);
    
    /* Run all dependency patterns */
    int result;
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    result = flow_dependency(arr1, N);
    printf("Flow dependency test: %d\n", result);
    
    /* Test 2: Anti dependency */
    anti_dependency(arr1, arr2, N);
    printf("Anti dependency test completed\n");
    
    /* Test 3: Output dependency */
    result = output_dependency(arr1, N);
    printf("Output dependency test: %d\n", result);
    
    /* Test 4: Control dependency */
    control_dependency(arr1, arr3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Mixed dependencies */
    result = mixed_dependencies(arr1, N);
    printf("Mixed dependencies test: %d\n", result);
    
    /* Test 6: Nested loops */
    nested_loop_deps(matrix, totals);
    printf("Nested loop test completed (total[0]=%d)\n", totals[0]);
    
    /* Test 7: Complex pattern */
    complex_dependency_pattern(arr1, arr2, arr4, N);
    printf("Complex pattern test completed\n");
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    printf("All tests completed successfully.\n");
    
    return 0;
}
