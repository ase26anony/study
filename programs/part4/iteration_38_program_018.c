/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Single accumulator with distance-1 dependence */
int loop_accumulator(int *data, int n, int coeff) {
    volatile int sum = 0;  /* volatile to preserve dependence */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependence: sum from iteration i used in iteration i+1 */
        sum = sum + data[i] * coeff;
        /* Add some independent operations to create ILP */
        int temp = data[i] * 2;
        temp = temp - coeff;
        /* Memory clobber to prevent optimization */
        asm volatile("" : : "r"(sum), "r"(temp) : "memory");
    }
    return sum;
}

/* Function 2: Multiple interleaved carried dependencies */
void loop_multiple_deps(int *a, int *b, int *c, int n) {
    volatile int acc1 = a[0];
    volatile int acc2 = b[0];
    
    for (int i = 1; i < n; i++) {
        /* Two separate distance-1 dependencies */
        acc1 = acc1 * 3 + a[i];
        acc2 = acc2 / 2 + b[i];
        
        /* Array access with distance-1 dependence */
        c[i] = c[i-1] + a[i] * b[i];
        
        /* Additional operations for scheduling complexity */
        int temp1 = a[i] << 2;
        int temp2 = b[i] >> 1;
        asm volatile("" : : "r"(acc1), "r"(acc2), "r"(temp1), "r"(temp2) : "memory");
    }
}

/* Function 3: Nested loop with inner loop carried dependency */
void loop_nested(int *mat, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        volatile int row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* Inner loop has distance-1 dependence */
            row_sum = row_sum + mat[i * cols + j];
            
            /* Complex addressing with multiple operations */
            int idx = i * cols + j;
            int val = mat[idx] * (i + j);
            mat[idx] = val % 256;
            
            asm volatile("" : : "r"(row_sum), "r"(val) : "memory");
        }
    }
}

/* Function 4: Loop with unknown trip count (prevents unrolling) */
int loop_unknown_count(int *x, int *y, int n, int k) {
    volatile int result = 0;
    /* k is unknown at compile time, prevents complete unrolling */
    for (int i = 0; i < n; i += k) {
        /* Distance-1 dependence through array */
        if (i > 0) {
            y[i] = y[i-1] + x[i] * 2;
        } else {
            y[i] = x[i];
        }
        
        /* Multiple operations with carried dependency */
        result = result * 7 + x[i];
        
        /* Complex expression with multiple uses */
        int t1 = x[i] * 3;
        int t2 = x[i] / 4;
        int t3 = t1 + t2;
        asm volatile("" : : "r"(result), "r"(t3) : "memory");
    }
    return result;
}

/* Function 5: Mixed dependencies with conditional */
void loop_mixed(int *a, int *b, int *c, int n) {
    volatile int state = 0;
    
    for (int i = 1; i < n; i++) {
        /* Strong distance-1 dependence */
        state = (state * 1103515245 + 12345) & 0x7fffffff;
        
        /* Array with distance-1 dependence */
        a[i] = a[i-1] + b[i];
        
        /* Another carried dependency */
        c[i] = c[i-1] * 2 - b[i];
        
        /* Conditional that creates different paths */
        if (state % 3 == 0) {
            a[i] = a[i] * 2;
        } else {
            c[i] = c[i] + 1;
        }
        
        asm volatile("" : : "r"(state), "r"(a[i]), "r"(c[i]) : "memory");
    }
}

/* Main driver function */
int main(int argc, char *argv[]) {
    const int SIZE = 1024;
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *data3 = malloc(SIZE * sizeof(int));
    int *result = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        result[i] = 0;
    }
    
    /* Call all test functions to ensure they're compiled */
    int sum1 = loop_accumulator(data1, SIZE, 3);
    loop_multiple_deps(data1, data2, result, SIZE);
    
    int mat_size = 32;
    int *matrix = malloc(mat_size * mat_size * sizeof(int));
    for (int i = 0; i < mat_size * mat_size; i++) {
        matrix[i] = rand() % 100;
    }
    loop_nested(matrix, mat_size, mat_size);
    
    int k = (argc > 1) ? atoi(argv[1]) : 3;
    if (k == 0) k = 3;
    int sum4 = loop_unknown_count(data1, result, SIZE, k);
    
    loop_mixed(data1, data2, data3, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    volatile int checksum = sum1 + sum4;
    for (int i = 0; i < SIZE; i++) {
        checksum += data1[i] + data2[i] + data3[i] + result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(data1);
    free(data2);
    free(data3);
    free(result);
    free(matrix);
    
    return 0;
}
