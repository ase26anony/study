/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        a[i] = a[i-1] * b[i] + c[i];  /* distance-1 use of a[i-1] */
        
        /* Additional accumulator with distance-1 dependence */
        sum = sum + a[i] * 3;  /* sum used in next iteration */
        
        /* Another distance-1 pattern */
        b[i] = b[i-1] + c[i] * 2;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    return sum + a[n-1];
}

/* Function 2: Nested loops with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Inner loop with carried dependency */
        int temp = x[i];
        for (j = 1; j < m; j++) {
            /* Distance-1 dependence in inner loop */
            temp = temp * y[j] + (j % 7);
            acc += temp;
        }
        x[i] = temp;
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(acc) : "memory");
    }
    
    return acc;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three separate accumulators with different operations */
        sum1 = sum1 + arr1[i] * arr2[i];      /* distance-1 use of sum1 */
        sum2 = sum2 - arr1[i-1] + arr2[i];    /* distance-1 use of arr1[i-1] */
        sum3 = sum3 * 2 + arr1[i];            /* distance-1 use of sum3 */
        
        /* Cross-iteration array dependency */
        arr1[i] = arr1[i-1] + sum1 - sum2;    /* distance-1 use of arr1[i-1] */
        
        /* Complex expression with multiple dependencies */
        arr2[i] = (arr2[i-1] * 3 + sum3) / 2; /* distance-1 use of arr2[i-1] */
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (parameter) and mixed operations */
int variable_loop(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* n is unknown at compile time - prevents unrolling */
    for (i = 1; i < n; i++) {
        /* Multiple distance-1 dependencies */
        int prev = data[i-1];
        data[i] = prev * coeff + i;
        
        /* Accumulator with distance-1 */
        result = result + data[i] * prev;
        
        /* Another distance-1 pattern */
        coeff = (coeff + prev) % 256;
        
        /* Memory clobber to preserve dependencies */
        asm volatile("" : : "r"(result), "r"(coeff) : "memory");
    }
    
    return result;
}

/* Function 5: Loop with if-conversion opportunities and dependencies */
int conditional_loop(int n, int *restrict a, int *restrict b) {
    volatile int total = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Conditional with carried dependency */
        if (a[i-1] > 0) {  /* distance-1 use */
            a[i] = a[i-1] + b[i] * 2;
        } else {
            a[i] = b[i] - a[i-1];
        }
        
        /* Unconditional carried dependency */
        total = total * 3 + a[i];
        
        /* Prevent dead code elimination */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int SIZE = 1000;
    int *a, *b, *c, *x, *y, *arr1, *arr2, *data;
    int i, result = 0;
    
    /* Seed for reproducible array values */
    srand(42);
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(SIZE * sizeof(int));
    b = (int*)malloc(SIZE * sizeof(int));
    c = (int*)malloc(SIZE * sizeof(int));
    x = (int*)malloc(SIZE * sizeof(int));
    y = (int*)malloc(SIZE * sizeof(int));
    arr1 = (int*)malloc(SIZE * sizeof(int));
    arr2 = (int*)malloc(SIZE * sizeof(int));
    data = (int*)malloc(SIZE * sizeof(int));
    
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, a, b, c);
    result += nested_loop_carried(SIZE/10, 10, x, y);
    result += multi_accumulator(SIZE, arr1, arr2);
    result += variable_loop(SIZE, data, 17);
    result += conditional_loop(SIZE, a, b);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(x); free(y);
    free(arr1); free(arr2); free(data);
    
    return 0;
}
