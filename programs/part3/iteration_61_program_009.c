/* test_ddg.c - Program to trigger DDG edge creation in GCC */
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
            src[i + 1] = temp * 2;  /* Anti-dependency on src[i+1] */
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

/* Mixed dependencies with loop-carried flow */
int mixed_dependencies(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    for (int i = 0; i < n; i++) {
        acc1 += arr[i];          /* Flow dependency on acc1 */
        if (acc1 > 1000) {       /* Control dependency */
            acc2 = arr[i];       /* Anti-dependency through arr[i] */
        }
        arr[i] = acc2 * 2;       /* Output dependency on arr[i] */
    }
    return acc1 + acc2;
}

/* Nested loops - DDG often built for inner loops */
void nested_loop_dependencies(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on 'acc' */
        }
        total[j] = acc;
        
        /* Second inner loop with anti-dependency */
        for (int i = 1; i < N; i++) {
            int temp = matrix[j][i - 1];
            matrix[j][i] = temp + 1;  /* Anti-dependency */
        }
    }
}

/* Complex loop with multiple dependency distances */
void distance_dependencies(int *a, int *b, int n) {
    for (int i = 2; i < n; i++) {
        /* Distance 2 flow dependency */
        a[i] = a[i - 2] + b[i];
        
        /* Distance 1 anti-dependency */
        b[i - 1] = a[i] * 2;
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    int *array1 = malloc(N * sizeof(int));
    int *array2 = malloc(N * sizeof(int));
    int matrix[M][N];
    int total[M];
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(array1, array2, N);
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 1000);
        }
    }
    
    /* Run all dependency patterns */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_num) {
        case 0:
            printf("Running all tests...\n");
            /* Fall through */
        case 1:
            printf("Flow dependency test: %d\n", 
                   flow_dependency(array1, N));
            if (test_num == 1) break;
        case 2:
            anti_dependency(array1, array2, N);
            printf("Anti-dependency test completed\n");
            if (test_num == 2) break;
        case 3:
            printf("Output dependency test: %d\n",
                   output_dependency(array1, N));
            if (test_num == 3) break;
        case 4:
            control_dependency(array1, array2, N);
            printf("Control dependency test completed\n");
            if (test_num == 4) break;
        case 5:
            printf("Mixed dependencies test: %d\n",
                   mixed_dependencies(array1, N));
            if (test_num == 5) break;
        case 6:
            nested_loop_dependencies(matrix, total);
            printf("Nested loop test completed\n");
            if (test_num == 6) break;
        case 7:
            distance_dependencies(array1, array2, N);
            printf("Distance dependencies test completed\n");
            if (test_num == 7) break;
        default:
            printf("Invalid test number. Use 0-7\n");
    }
    
    /* Use results to prevent dead code elimination */
    volatile int sink = array1[0] + array2[0] + total[0];
    (void)sink;
    
    free(array1);
    free(array2);
    
    return 0;
}
