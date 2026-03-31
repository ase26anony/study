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
int flow_dependency(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency if temp reused */
        /* Force anti-dependency by reusing temp in next iteration */
        if (i < size - 1) {
            src[i + 1] = temp * 2;  /* Write to src[i+1] after reading src[i] */
        }
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * 2;  /* Compute temporary value */
        result = t;          /* Write to 'result' each iteration - output dependency */
    }
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
}

/* Mixed dependencies - combines multiple types */
int mixed_dependencies(int *arr, int size) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < size; i++) {
        int curr = arr[i];
        
        /* Flow dependency on 'acc' */
        acc += curr;
        
        /* Anti-dependency through 'prev' */
        if (i > 0) {
            arr[i - 1] = prev * 2;  /* Write after read in previous iteration */
        }
        
        /* Control dependency */
        if (curr > 100) {
            acc += 10;
        }
        
        prev = curr;  /* Setup for next iteration */
    }
    
    return acc;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on 'acc' */
            
            /* Anti-dependency within inner loop */
            if (i > 0) {
                matrix[j][i - 1] = acc % 256;
            }
        }
        
        totals[j] = acc;
        
        /* Control dependency in outer loop */
        if (acc > 10000) {
            totals[j] = 10000;
        }
    }
}

/* Loop with pointer-based dependencies */
void pointer_based_dependencies(int *data, int size) {
    int *read_ptr = data;
    int *write_ptr = data;
    
    for (int i = 0; i < size - 1; i++) {
        /* Flow dependency through pointers */
        int val = *read_ptr;
        read_ptr++;
        
        /* Anti-dependency: read then write */
        *write_ptr = val + 1;
        write_ptr++;
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
    }
}

/* Initialize matrix with pseudo-random data */
void init_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 1000);
        }
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int arr1[N], arr2[N];
    int matrix[M][N];
    int totals[M];
    
    /* Initialize test data */
    init_arrays(arr1, arr2, N);
    init_matrix(matrix);
    
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
    int mixed_result = mixed_dependencies(arr1, N);
    printf("Mixed dependencies test: result = %d\n", mixed_result);
    
    /* Test 6: Nested loop dependencies */
    nested_loop_dependencies(matrix, totals);
    printf("Nested loop dependencies test completed\n");
    
    /* Test 7: Pointer-based dependencies */
    pointer_based_dependencies(arr1, N);
    printf("Pointer-based dependencies test completed\n");
    
    /* Verify some results to prevent dead code elimination */
    volatile int check = sum1 + result + mixed_result + totals[0];
    (void)check;
    
    printf("All DDG tests completed successfully\n");
    
    return 0;
}
