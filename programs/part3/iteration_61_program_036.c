/* test_ddg.c - Test program to trigger ddg_edge creation in GCC's DDG builder */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128
#define SEED 42

/* Simple PRNG to generate data without external dependencies */
static unsigned int lcg = SEED;
static inline unsigned int rand_int(void) {
    lcg = lcg * 1103515245 + 12345;
    return lcg;
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
        if (a[i] > 0) {     /* Control dependency based on loop-variant value */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies with loop-carried flow dependency */
int mixed_dependencies(int *arr, int n) {
    int acc = 0;
    int prev = 0;
    for (int i = 0; i < n; i++) {
        int curr = arr[i];
        acc += curr;        /* Flow dependency on 'acc' */
        if (prev > 0) {     /* Control dependency */
            acc += 1;
        }
        prev = curr;        /* Anti-dependency on 'prev' */
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
        int prev = 0;
        for (int i = 0; i < N; i++) {
            int temp = matrix[j][i];
            matrix[j][i] = prev + temp;
            prev = temp;
        }
    }
}

/* Complex loop with multiple dependency types */
void complex_dependency_pattern(int *a, int *b, int *c, int n) {
    int s1 = 0, s2 = 0;
    for (int i = 0; i < n; i++) {
        /* Flow dependencies */
        s1 = s1 + a[i];
        s2 = s2 + b[i];
        
        /* Anti-dependency through temporary */
        int t = c[i];
        a[i] = t * 2;
        
        /* Control dependency */
        if (s1 > s2) {
            b[i] = 1;
        } else {
            b[i] = -1;
        }
        
        /* Output-like dependency */
        c[i] = s1 - s2;
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)(rand_int() % 100) - 50;
        arr2[i] = (int)(rand_int() % 100) - 50;
        arr3[i] = (int)(rand_int() % 100) - 50;
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    int test_select = 0;
    
    if (argc > 1) {
        test_select = atoi(argv[1]);
    }
    
    /* Allocate and initialize test data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int *results = (int*)malloc(M * sizeof(int));
    int matrix[M][N];
    
    init_arrays(arr1, arr2, arr3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(rand_int() % 100);
        }
    }
    
    /* Execute selected tests to trigger DDG edge creation */
    switch (test_select) {
        case 0:
            /* Run all tests */
            printf("Running all dependency tests...\n");
            printf("Flow result: %d\n", flow_dependency(arr1, N));
            anti_dependency(arr1, arr2, N);
            printf("Output result: %d\n", output_dependency(arr1, N));
            control_dependency(arr1, arr2, N);
            printf("Mixed result: %d\n", mixed_dependencies(arr1, N));
            nested_loop_dependencies(matrix, results);
            complex_dependency_pattern(arr1, arr2, arr3, N);
            break;
            
        case 1:
            /* Focus on flow dependencies */
            printf("Flow dependency test: %d\n", flow_dependency(arr1, N));
            break;
            
        case 2:
            /* Focus on anti-dependencies */
            anti_dependency(arr1, arr2, N);
            printf("Anti-dependency test completed\n");
            break;
            
        case 3:
            /* Focus on control dependencies */
            control_dependency(arr1, arr2, N);
            printf("Control dependency test completed\n");
            break;
            
        case 4:
            /* Nested loops */
            nested_loop_dependencies(matrix, results);
            printf("Nested loop test completed\n");
            break;
            
        case 5:
            /* Complex pattern */
            complex_dependency_pattern(arr1, arr2, arr3, N);
            printf("Complex pattern test completed\n");
            break;
            
        default:
            printf("Invalid test selection\n");
            break;
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(results);
    
    return 0;
}
