/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Flow dependency (TRUE_DEP) - accumulator pattern */
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
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency on 'temp' */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' across iterations */
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

/* Mixed dependencies - combines multiple types */
int mixed_dependencies(int *arr, int n) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < n; i++) {
        int current = arr[i];
        acc += current;          /* Flow dependency on 'acc' */
        arr[i] = prev;           /* Anti-dependency on 'arr[i]' */
        prev = current;          /* Output dependency on 'prev' */
    }
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
    }
}

/* Loop with pointer-based dependencies */
void pointer_based_deps(int *src, int *dst, int n) {
    int *p = src;
    int *q = dst;
    
    for (int i = 0; i < n; i++) {
        *q = *p + *(p + 1);  /* Complex address calculation */
        p++;
        q++;
    }
}

/* Function to prevent dead code elimination */
volatile int sink;

int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        array1[i] = (int)(lcg_rand() % 100);
        array2[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    /* Test different dependency patterns */
    int result;
    
    /* 1. Flow dependency test */
    result = flow_dependency(array1, N);
    sink = result;  /* Prevent elimination */
    
    /* 2. Anti-dependency test */
    anti_dependency(array1, array2, N);
    sink = array2[N-1];
    
    /* 3. Output dependency test */
    result = output_dependency(array1, N);
    sink = result;
    
    /* 4. Control dependency test */
    control_dependency(array1, array2, N);
    sink = array2[0];
    
    /* 5. Mixed dependencies test */
    result = mixed_dependencies(array1, N);
    sink = result;
    
    /* 6. Nested loops test */
    int totals[M];
    nested_loop_deps(matrix, totals);
    sink = totals[M-1];
    
    /* 7. Pointer-based test */
    pointer_based_deps(array1, array2, N);
    sink = array2[N-1];
    
    /* Clean up */
    free(array1);
    free(array2);
    
    printf("DDG test completed (sink = %d)\n", sink);
    return 0;
}
