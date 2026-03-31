/* test_modulo_sched.c - Test program for GCC modulo scheduler edge printing */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define ITERS 1000

/* Global volatile sink to prevent optimization */
volatile int sink = 0;

/* Function 1: Loop with carried dependency and mixed operations */
int func1_carried_dep(volatile int *arr, int n, int scalar) {
    int sum = 0;
    /* Loop with distance-1 carried dependency */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * scalar + arr[i];
        /* Mix in some floating point operations */
        float ftmp = (float)arr[i] * 1.5f;
        sum += (int)ftmp;
    }
    return sum;
}

/* Function 2: Reduction loop with pointer chasing pattern */
int func2_reduction_chase(int *data, int n) {
    int *p = data;
    int total = 0;
    /* Pointer chasing creates dependencies */
    for (int i = 0; i < n; i++) {
        total += *p;
        p = data + (total & (n-1));  /* Non-linear access pattern */
        /* Integer operations with different latencies */
        total ^= (total << 3);
        total += i * 7;
    }
    return total;
}

/* Function 3: Loop with multiple independent statements and dependent store */
double func3_mixed_ops(double *a, double *b, int n, double coeff) {
    double acc = 0.0;
    /* Mixed integer/floating point with memory dependencies */
    for (int i = 1; i < n; i++) {
        double t1 = a[i-1] * coeff;
        double t2 = b[i] + 1.5;
        a[i] = t1 + t2;  /* Dependent on both previous computations */
        acc += a[i];
        
        /* Bitwise operations to add variety */
        int itmp = (int)acc;
        itmp = (itmp << 2) | (itmp >> 30);  /* Rotate */
        acc = (double)itmp * 0.01;
    }
    return acc;
}

/* Function 4: Nested loops with conditional inner logic */
int func4_nested_conditional(int *mat, int rows, int cols, int threshold) {
    int count = 0;
    for (int r = 0; r < rows; r++) {
        /* Inner loop with carried dependency */
        if (r % 2 == 0) {  /* Conditional prevents trivial optimization */
            int prev = mat[r * cols];
            for (int c = 1; c < cols; c++) {
                int idx = r * cols + c;
                prev = mat[idx] * 3 + prev;
                mat[idx] = prev;
                count += prev;
                
                /* Compiler barrier to preserve operations */
                asm volatile("" : "+r"(prev) : : "memory");
            }
        }
    }
    return count;
}

/* Function 5: Loop with multiple recurrence distances */
void func5_multiple_recurrences(int *x, int *y, int n, int alpha, int beta) {
    /* Two independent carried dependencies */
    int acc1 = x[0];
    int acc2 = y[0];
    
    for (int i = 1; i < n; i++) {
        /* First recurrence chain */
        acc1 = acc1 * alpha + x[i];
        x[i] = acc1;
        
        /* Second recurrence chain with different distance */
        if (i >= 2) {
            acc2 = acc2 + y[i-2] * beta;
        }
        y[i] = acc2;
        
        /* Cross dependency between chains */
        x[i] ^= acc2 & 0xFF;
    }
}

/* Main driver that ensures all loops execute */
int main(int argc, char **argv) {
    /* Initialize data arrays */
    volatile int arr1[SIZE];
    int arr2[SIZE];
    double arr3[SIZE], arr4[SIZE];
    int matrix[10][10];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (i * 3) % 97;
        arr3[i] = (double)i * 0.5;
        arr4[i] = (double)i * 1.5;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Execute all loop patterns multiple times */
    int total = 0;
    for (int iter = 0; iter < ITERS; iter++) {
        total += func1_carried_dep(arr1, SIZE, 3);
        total += func2_reduction_chase(arr2, SIZE);
        
        double dres = func3_mixed_ops(arr3, arr4, SIZE, 2.5);
        total += (int)dres;
        
        total += func4_nested_conditional((int *)matrix, 10, 10, 50);
        
        int x[SIZE], y[SIZE];
        for (int i = 0; i < SIZE; i++) {
            x[i] = i;
            y[i] = SIZE - i;
        }
        func5_multiple_recurrences(x, y, SIZE, 2, 3);
        total += x[SIZE-1] + y[SIZE-1];
        
        /* Use sink to prevent dead code elimination */
        sink = total;
    }
    
    printf("Result: %d (sink: %d)\n", total, sink);
    return 0;
}
