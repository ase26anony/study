/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        a[i] = a[i-1] * b[i] + c[i];  /* distance-1 dependence */
        sum += a[i];                   /* accumulator with carried dependency */
        
        /* Additional operations to create instruction-level parallelism */
        b[i] = b[i-1] + i;             /* another distance-1 dependence */
        c[i] = c[i-1] * 2 - i;         /* yet another distance-1 dependence */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = x[i];
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            acc = acc * y[j] + (j % 7);  /* carried across inner iterations */
            x[i] += acc;
            
            /* Create complex dependency pattern */
            y[j] = y[j-1] + acc / 3;     /* distance-1 in inner loop */
        }
        total += x[i];
        
        /* Prevent loop invariant code motion */
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first elements to create dependencies */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + arr1[i-1] * 3;      /* distance-1 use of arr1 */
        sum2 = sum2 + arr2[i-1] * 5;      /* distance-1 use of arr2 */
        sum3 = sum3 + (sum1 + sum2) / 2;  /* depends on both accumulators */
        
        /* Update arrays with carried dependencies */
        arr1[i] = arr1[i-1] + sum1 - i;   /* distance-1 dependence */
        arr2[i] = arr2[i-1] + sum2 + i;   /* distance-1 dependence */
        
        /* Complex expression to create more ILP opportunities */
        int temp = (arr1[i] * arr2[i]) / (i + 1);
        sum1 += temp;
        sum2 -= temp / 2;
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_loop(int n, int init_val, int *restrict data) {
    volatile int acc = init_val;
    int i;
    
    /* Pattern with multiple distance-1 dependencies */
    data[0] = init_val;
    for (i = 1; i < n; i++) {
        /* Chain of dependencies */
        int t1 = data[i-1] * 3 + 7;      /* distance-1 */
        int t2 = acc * 2 - 5;            /* carried accumulator */
        data[i] = t1 + t2 + i;
        acc = data[i] - acc;             /* cross-iteration dependency */
        
        /* Additional operations for scheduler complexity */
        if (i % 4 == 0) {
            acc += data[i-3] * 2;        /* longer distance dependence */
        }
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(acc) : "memory");
    }
    return acc;
}

/* Function 5: Loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict src, int *restrict dst) {
    volatile int count = 0;
    int i;
    
    dst[0] = src[0];
    for (i = 1; i < n; i++) {
        /* Carried dependency through dst */
        int base = dst[i-1];
        
        /* Conditional updates create complex dependencies */
        if (src[i] > 0) {
            dst[i] = base + src[i] * 2;
            count += dst[i];
        } else {
            dst[i] = base - src[i];
            count -= dst[i];
        }
        
        /* Additional arithmetic with carried dependency */
        src[i] = src[i-1] + (i % 10);
        
        /* Prevent optimization */
        asm volatile("" : : "r"(count) : "memory");
    }
    return count;
}

/* Main driver to ensure all functions are called */
int main(int argc, char **argv) {
    const int SIZE = 1000;
    int *a, *b, *c, *x, *y, *arr1, *arr2, *data, *src, *dst;
    int result = 0;
    
    /* Allocate and initialize arrays */
    a = malloc(SIZE * sizeof(int));
    b = malloc(SIZE * sizeof(int));
    c = malloc(SIZE * sizeof(int));
    x = malloc(SIZE * sizeof(int));
    y = malloc(SIZE * sizeof(int));
    arr1 = malloc(SIZE * sizeof(int));
    arr2 = malloc(SIZE * sizeof(int));
    data = malloc(SIZE * sizeof(int));
    src = malloc(SIZE * sizeof(int));
    dst = malloc(SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
        src[i] = rand() % 200 - 100;  /* Include negative values */
        dst[i] = 0;
    }
    
    /* Call all test functions with different parameters */
    result += loop_carried_deps(SIZE, a, b, c);
    result += nested_loop_deps(10, SIZE/10, x, y);
    result += multi_accumulator(SIZE, arr1, arr2);
    result += variable_loop(SIZE, 42, data);
    result += conditional_loop(SIZE, src, dst);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data);
    free(src); free(dst);
    
    return 0;
}
