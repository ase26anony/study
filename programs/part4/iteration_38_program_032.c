/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    for (i = 1; i < n; i++) {
        /* Multiple operations to create complex schedule */
        int temp = a[i-1] * b[i];      /* Use of value from previous iteration */
        a[i] = temp + c[i];            /* Store result */
        sum += a[i];                   /* Accumulator with carried dependency */
        
        /* Additional operations for instruction-level parallelism */
        b[i] = b[i-1] + i;             /* Another distance-1 dependence */
        c[i] = c[i] * 2 - sum;         /* Mix with accumulator */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Unrolled outer loop */
        int acc1 = 0, acc2 = 0;
        
        for (j = 1; j < m; j++) {
            /* Inner loop with multiple carried dependencies */
            acc1 = acc1 + x[j-1] * y[j];    /* Distance-1 dependence */
            acc2 = y[j-1] + acc2 * 3;       /* Another distance-1 dependence */
            x[j] = acc1 - acc2;
            
            /* Complex expression to create scheduling opportunities */
            y[j] = (x[j] << 2) | (y[j-1] & 0xFF);
            
            /* Force dependency preservation */
            asm volatile("" : "+r"(acc1), "+r"(acc2) : : "memory");
        }
        
        /* Combine accumulators */
        total += acc1 * acc2;
        
        /* Prevent loop invariant code motion */
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulator(int n, int *restrict data, int coeff1, int coeff2) {
    /* Unknown coefficients force runtime computation */
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + data[i-1] * coeff1;      /* Distance-1 use of data */
        sum2 = sum2 * coeff2 + data[i];        /* Different pattern */
        sum3 = sum3 + (sum1 ^ sum2);           /* Mix the two accumulators */
        
        /* Update data array with carried dependency */
        data[i] = data[i-1] + sum3;
        
        /* Create anti-dependencies for more complex graph */
        int temp = sum1;
        sum1 = sum2;
        sum2 = sum3;
        sum3 = temp;
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    /* Combine results in non-trivial way */
    return (sum1 * 2) + (sum2 * 3) + (sum3 * 5);
}

/* Function 4: Loop with if-conversion opportunities and carried deps */
int conditional_carried(int n, int *restrict arr, int threshold) {
    volatile int count = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Condition creates control dependencies */
        if (arr[i-1] > threshold) {    /* Distance-1 dependence in condition */
            count += arr[i] * 2;
            arr[i] = arr[i-1] + 1;     /* Another distance-1 dependence */
        } else {
            count -= arr[i];
            arr[i] = arr[i-1] - 1;     /* Distance-1 dependence in else path */
        }
        
        /* Additional arithmetic with carried value */
        arr[i] = arr[i] * (count % 16);
        
        /* Force dependency chain */
        asm volatile("" : : "r"(count) : "memory");
    }
    return count;
}

/* Function 5: Reduction with multiple dependency patterns */
int complex_reduction(int n, int *restrict src1, int *restrict src2) {
    volatile int red1 = src1[0], red2 = src2[0];
    int i;
    
    for (i = 1; i < n; i++) {
        /* Multiple reduction operations with different dependencies */
        red1 = red1 * 3 + src1[i];          /* Simple carried dependency */
        red2 = (red2 << 1) | (src2[i-1] & 1); /* Distance-1 use of src2 */
        
        /* Cross-dependency between reductions */
        int mix = red1 ^ red2;
        red1 = red1 + mix;
        red2 = red2 - mix;
        
        /* Update source arrays with carried dependencies */
        src1[i] = src1[i-1] + red1;
        src2[i] = src2[i-1] + red2;
        
        /* Compiler barrier */
        asm volatile("" : : "r"(red1), "r"(red2) : "memory");
    }
    
    return red1 + red2;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    
    /* Allocate and initialize test arrays */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *x = (int *)malloc(n * sizeof(int));
    int *y = (int *)malloc(n * sizeof(int));
    int *data = (int *)malloc(n * sizeof(int));
    int *arr = (int *)malloc(n * sizeof(int));
    int *src1 = (int *)malloc(n * sizeof(int));
    int *src2 = (int *)malloc(n * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 - 2;
        c[i] = i * 7 + 3;
        x[i] = i * 11 - 5;
        y[i] = i * 13 + 7;
        data[i] = i * 17 - 11;
        arr[i] = i * 19 + 13;
        src1[i] = i * 23 - 17;
        src2[i] = i * 29 + 19;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_deps(n, a, b, c);
    result += nested_loop_carried(10, m, x, y);
    result += multi_accumulator(n, data, 3, 7);
    result += conditional_carried(n, arr, 500);
    result += complex_reduction(n, src1, src2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(data); free(arr);
    free(src1); free(src2);
    
    return 0;
}
