/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int force_keep = 0;

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int flow_dependency(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on sum across iterations */
    }
    force_keep = sum;  /* Prevent dead code elimination */
    return sum;
}

/* Anti dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti dependency on temp */
        /* Anti dependency: read src[i] in iteration i, 
           potential write to src[i+1] in next iteration if arrays overlap */
    }
    force_keep = dst[size-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * 2;  /* Computation */
        result = t;          /* Output dependency on result across iterations */
        /* Each iteration overwrites result */
    }
    force_keep = result;
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
    force_keep = b[size-1];
}

/* Complex flow dependency with multiple accumulators */
int multi_flow_dependency(int *arr, int size) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    for (int i = 0; i < size; i++) {
        sum1 += arr[i];      /* Flow dep 1 */
        sum2 += sum1;        /* Flow dep 2 (depends on sum1) */
        sum3 += sum2;        /* Flow dep 3 (depends on sum2) */
    }
    force_keep = sum3;
    return sum3;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependency(int matrix[M][N], int result[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        result[j] = acc;
        
        /* Add some anti-dependency between outer loop iterations */
        if (j > 0) {
            result[j] += result[j-1];  /* Flow between outer iterations */
        }
    }
    force_keep = result[M-1];
}

/* Loop with mixed dependencies */
void mixed_dependencies(int *a, int *b, int *c, int size) {
    int temp = 0;
    for (int i = 0; i < size; i++) {
        /* Flow: temp depends on previous iteration */
        temp = a[i] + temp;
        
        /* Anti: read b[i] before potentially modifying it */
        int read_val = b[i];
        
        /* Output: c[i] written each iteration (if i doesn't change) */
        c[i] = temp + read_val;
        
        /* Control: depends on computed value */
        if (c[i] > 100) {
            b[i] = 0;  /* Anti: modifies b read in next iteration */
        }
    }
    force_keep = c[size-1];
}

/* Initialize arrays with pseudo-random values */
void init_array(int *arr, int size, int seed) {
    /* Simple LCG to avoid library dependencies */
    int val = seed;
    for (int i = 0; i < size; i++) {
        val = (1103515245 * val + 12345) & 0x7fffffff;
        arr[i] = (val % 100) - 50;  /* Values between -50 and 49 */
    }
}

/* Initialize matrix */
void init_matrix(int matrix[M][N], int seed) {
    int val = seed;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            val = (1103515245 * val + 12345) & 0x7fffffff;
            matrix[j][i] = (val % 100) - 50;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int result[M];
    
    /* Initialize data */
    init_array(arr1, N, 42);
    init_array(arr2, N, 123);
    init_array(arr3, N, 789);
    init_matrix(matrix, 456);
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    printf("1. Flow dependency test: ");
    int sum = flow_dependency(arr1, N);
    printf("sum = %d\n", sum);
    
    /* Test 2: Anti dependency */
    printf("2. Anti dependency test: ");
    anti_dependency(arr1, arr2, N);
    printf("done\n");
    
    /* Test 3: Output dependency */
    printf("3. Output dependency test: ");
    int out = output_dependency(arr1, N);
    printf("result = %d\n", out);
    
    /* Test 4: Control dependency */
    printf("4. Control dependency test: ");
    control_dependency(arr1, arr3, N);
    printf("done\n");
    
    /* Test 5: Multiple flow dependencies */
    printf("5. Multiple flow dependencies test: ");
    int multi = multi_flow_dependency(arr1, N);
    printf("result = %d\n", multi);
    
    /* Test 6: Nested loop dependencies */
    printf("6. Nested loop dependencies test: ");
    nested_loop_dependency(matrix, result);
    printf("done\n");
    
    /* Test 7: Mixed dependencies */
    printf("7. Mixed dependencies test: ");
    mixed_dependencies(arr1, arr2, arr3, N);
    printf("done\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("All tests completed.\n");
    
    return 0;
}
