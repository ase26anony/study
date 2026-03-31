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
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Anti-dependency on 'temp' if it persists */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' across iterations */
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

/* Mixed dependencies - complex pattern */
int mixed_dependencies(int *arr, int size) {
    int acc1 = 0, acc2 = 0;
    int prev = arr[0];
    
    for (int i = 1; i < size; i++) {
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
void nested_loop_deps(int matrix[M][N], int result[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on 'acc' */
        }
        result[j] = acc;
        
        /* Second inner loop with anti-dependency */
        int temp[N];
        for (int i = 0; i < N; i++) {
            temp[i] = matrix[j][i];  /* Read */
            matrix[j][i] = temp[i] * 2;  /* Write - anti-dependency */
        }
    }
}

/* Loop with distance > 1 dependency */
void distance_dependency(int *src, int *dst, int size, int distance) {
    for (int i = distance; i < size; i++) {
        /* Flow dependency with distance */
        dst[i] = src[i - distance] + dst[i - 1];
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *array3 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    int result[M];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        array1[i] = (int)(lcg_rand() % 100);
        array2[i] = (int)(lcg_rand() % 100);
        array3[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    /* Run all dependency tests */
    int test_mode = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_mode) {
        case 0: /* Run all tests */
            printf("Testing all dependency patterns...\n");
            
            /* Flow dependency */
            int sum1 = flow_dependency(array1, N);
            printf("Flow dependency result: %d\n", sum1);
            
            /* Anti-dependency */
            anti_dependency(array1, array2, N);
            printf("Anti-dependency completed\n");
            
            /* Output dependency */
            int sum2 = output_dependency(array1, N);
            printf("Output dependency result: %d\n", sum2);
            
            /* Control dependency */
            control_dependency(array1, array3, N);
            printf("Control dependency completed\n");
            
            /* Mixed dependencies */
            int sum3 = mixed_dependencies(array1, N);
            printf("Mixed dependencies result: %d\n", sum3);
            
            /* Nested loops */
            nested_loop_deps(matrix, result);
            printf("Nested loops completed\n");
            
            /* Distance dependency */
            distance_dependency(array1, array2, N, 3);
            printf("Distance dependency completed\n");
            break;
            
        case 1:
            flow_dependency(array1, N);
            break;
        case 2:
            anti_dependency(array1, array2, N);
            break;
        case 3:
            output_dependency(array1, N);
            break;
        case 4:
            control_dependency(array1, array3, N);
            break;
        case 5:
            mixed_dependencies(array1, N);
            break;
        case 6:
            nested_loop_deps(matrix, result);
            break;
        case 7:
            distance_dependency(array1, array2, N, 2);
            break;
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
