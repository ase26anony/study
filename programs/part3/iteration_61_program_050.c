/* ddg_coverage.c - Test program to exercise GCC's Data Dependency Graph builder */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on sum across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src */
        dst[i] = temp + 1;  /* Write to dst - creates anti-dependency on temp */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on result across iterations */
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

/* Mixed dependencies - complex pattern */
int mixed_dependencies(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    int prev = arr[0];
    
    for (int i = 1; i < n; i++) {
        /* Flow dependency on acc1 */
        acc1 += arr[i];
        
        /* Anti-dependency through prev */
        int current = arr[i];
        acc2 += prev;      /* Read prev */
        prev = current;    /* Write prev - anti-dependency */
        
        /* Control dependency */
        if (acc1 > acc2) {
            arr[i] = acc1;  /* Output dependency on arr[i] */
        }
    }
    return acc1 + acc2;
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
        
        /* Second inner loop with anti-dependency */
        int prev = matrix[j][0];
        for (int i = 1; i < N; i++) {
            int curr = matrix[j][i];
            matrix[j][i-1] = prev * 2;  /* Write */
            prev = curr;                /* Read then write - anti-dependency */
        }
    }
}

/* Loop with pointer aliasing (creates conservative dependencies) */
void pointer_aliasing(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n - 1; i++) {
        a[i] = b[i-1] + c[i+1];  /* Flow dependencies through arrays */
        b[i] = a[i] * 2;         /* Anti-dependency through a[i] */
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)lcg_rand() % 100;
        arr2[i] = (int)lcg_rand() % 100;
        arr3[i] = (int)lcg_rand() % 100;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
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
            matrix[j][i] = (int)lcg_rand() % 100;
        }
    }
    
    /* Run all dependency tests */
    int result = 0;
    
    /* Test 1: Flow dependency */
    result += flow_dependency(arr1, N);
    
    /* Test 2: Anti-dependency */
    anti_dependency(arr1, arr2, N);
    
    /* Test 3: Output dependency */
    result += output_dependency(arr1, N);
    
    /* Test 4: Control dependency */
    control_dependency(arr1, arr2, N);
    
    /* Test 5: Mixed dependencies */
    result += mixed_dependencies(arr3, N);
    
    /* Test 6: Nested loops */
    nested_loop_dependencies(matrix, totals);
    
    /* Test 7: Pointer aliasing */
    pointer_aliasing(arr1, arr2, arr3, N);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    for (int j = 0; j < M; j++) {
        sink += totals[j];
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("DDG test completed (result: %d)\n", sink);
    return 0;
}
