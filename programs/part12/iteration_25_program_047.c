#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 64

/* Prevent optimization and create dependencies */
volatile int global_seed = 42;

/* Noinline functions to prevent optimization */
__attribute__((noinline)) int use_value(int x) {
    return x + global_seed;
}

__attribute__((noinline)) void modify_value(int *x) {
    *x += global_seed;
}

/* Test 1: Flow dependency (RAW) with carried dependency across iterations */
__attribute__((noinline)) int test_flow_dependency(int *arr, int n) {
    int sum = 0;
    volatile int temp = 0;
    
    /* Classic accumulation with flow dependency */
    for (int i = 0; i < n; i++) {
        temp = arr[i];      /* Read arr[i] */
        sum += temp;        /* Use temp (creates dependency chain) */
        arr[i] = sum;       /* Write back (creates output dependency) */
    }
    return sum;
}

/* Test 2: Anti-dependency (WAR) within same iteration */
__attribute__((noinline)) int test_anti_dependency(int *a, int *b, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int read_val = a[i];    /* Read a[i] */
        a[i] = i * 2;           /* Overwrite a[i] (anti-dependency) */
        b[i] = read_val + i;    /* Use read value */
        result += b[i];
    }
    return result;
}

/* Test 3: Output dependency (WAW) and mixed operations */
__attribute__((noinline)) double test_output_dependency(double *arr, int n) {
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i * 1.5;      /* First write */
        arr[i] = arr[i] * 2.0;         /* Second write to same location (WAW) */
        
        /* Different data type operations */
        float temp = (float)arr[i];
        arr[i] = (double)temp * 1.1;   /* Third write with type conversion */
        
        sum += arr[i];
    }
    return sum;
}

/* Test 4: Nested loops with outer-loop dependencies */
__attribute__((noinline)) int test_nested_dependency(int **matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 1; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Flow dependency across outer loop iterations */
            matrix[i][j] = matrix[i-1][j] + matrix[i][j];
            total += matrix[i][j];
        }
    }
    return total;
}

/* Test 5: Control flow with dependencies */
__attribute__((noinline)) int test_control_flow_dependency(int *arr, int n) {
    int sum = 0;
    int accumulator = 0;
    
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            /* Flow dependency path */
            accumulator += arr[i];
            arr[i] = accumulator;
        } else if (i % 3 == 1) {
            /* Anti-dependency path */
            int old_val = arr[i];
            arr[i] = i * 3;
            sum += old_val;
        } else {
            /* Output dependency path */
            arr[i] = sum;
            arr[i] = arr[i] + 1;  /* WAW */
            sum = arr[i];
        }
    }
    return sum + accumulator;
}

/* Test 6: Pointer aliasing creates ambiguous dependencies */
__attribute__((noinline)) int test_pointer_aliasing(int *a, int *b, int n) {
    int sum = 0;
    
    /* b might alias with a, creating potential dependencies */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  /* Flow dependency through a[i-1] */
        sum += a[i];
        
        /* Potential anti-dependency if b aliases a */
        modify_value(&b[i]);
    }
    return sum;
}

/* Test 7: Complex dependency chain with function calls */
__attribute__((noinline)) int test_complex_chain(int *arr, int n) {
    int chain = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple interdependent operations */
        int val1 = use_value(arr[i]);
        int val2 = val1 * 2;
        arr[i] = val2;
        
        /* Create distance > 0 dependency */
        if (i >= 2) {
            chain += arr[i-2];  /* Distance 2 flow dependency */
        }
        
        /* Mix with anti-dependency */
        int temp = arr[i];
        arr[i] = chain;
        chain += temp;
    }
    return chain;
}

int main() {
    /* Initialize data */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    double *array3 = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize matrix for nested test */
    int **matrix = (int**)malloc(INNER_SIZE * sizeof(int*));
    for (int i = 0; i < INNER_SIZE; i++) {
        matrix[i] = (int*)malloc(INNER_SIZE * sizeof(int));
        for (int j = 0; j < INNER_SIZE; j++) {
            matrix[i][j] = i * INNER_SIZE + j;
        }
    }
    
    /* Initialize arrays with values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = (double)(rand() % 100) / 3.0;
    }
    
    /* Execute all tests to create various DDG edges */
    int result1 = test_flow_dependency(array1, SIZE);
    int result2 = test_anti_dependency(array1, array2, SIZE);
    double result3 = test_output_dependency(array3, SIZE);
    int result4 = test_nested_dependency(matrix, INNER_SIZE, INNER_SIZE);
    int result5 = test_control_flow_dependency(array1, SIZE);
    int result6 = test_pointer_aliasing(array1, array2, SIZE);
    int result7 = test_complex_chain(array1, SIZE);
    
    /* Aggregate results to prevent dead code elimination */
    int final_result = result1 + result2 + (int)result3 + result4 + result5 + result6 + result7;
    
    printf("Final checksum: %d\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    for (int i = 0; i < INNER_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}
