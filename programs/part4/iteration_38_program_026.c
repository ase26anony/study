/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        a[i] = a[i-1] * b[i] + c[i];      /* Distance-1 dependence on a[i-1] */
        sum = sum + a[i];                  /* Accumulator pattern */
        
        /* Additional operations to create instruction-level parallelism */
        b[i] = b[i] * 2 - c[i-1];          /* Another distance-1 dependence */
        c[i] = c[i] + (a[i] >> 2);
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loops(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = 0;
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            /* Pattern: y[j] depends on y[j-1] */
            y[j] = y[j-1] + x[i] * j;      /* Distance-1 dependence */
            acc = acc + y[j];              /* Accumulator */
            
            /* Additional operations */
            x[i] = x[i] + (y[j] & 0xFF);
            
            /* Force dependence preservation */
            asm volatile("" : : "r"(acc) : "memory");
        }
        total += acc;
        
        /* Unroll hint for outer loop */
        if (i % 2 == 0) {
            x[i] = x[i] * 3;
        }
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    /* Initialize first elements */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Two separate carried dependencies */
        arr1[i] = arr1[i-1] * 3 + arr2[i-1];   /* Depends on both from prev iteration */
        arr2[i] = arr2[i-1] * 2 - arr1[i];     /* Depends on current arr1[i] */
        
        /* Two independent accumulators */
        sum1 = sum1 + arr1[i] * i;
        sum2 = sum2 + arr2[i] / (i + 1);
        
        /* Complex operation mixing both */
        arr1[i] = (arr1[i] + sum1) ^ (arr2[i] + sum2);
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    return sum1 + sum2;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_loop(int n, int init_val, int *restrict data) {
    volatile int state = init_val;
    int i;
    
    /* Fibonacci-like pattern with carried dependency */
    int prev = 0, curr = 1;
    if (n > 0) data[0] = state;
    
    for (i = 1; i < n; i++) {
        /* Strong distance-1 dependence */
        int next = prev + curr + data[i-1];
        prev = curr;
        curr = next;
        
        data[i] = state + curr;
        state = state * 2 + data[i];
        
        /* Array access with stride 1 */
        if (i > 1) {
            data[i] = data[i] - data[i-2];  /* Distance-2 dependence */
        }
        
        /* Memory clobber */
        asm volatile("" : : "r"(state) : "memory");
    }
    return state;
}

/* Function 5: Complex loop with mixed operations */
int complex_mixed_ops(int n, int *restrict buf) {
    volatile int hash = 5381;
    int i;
    
    for (i = 0; i < n; i++) {
        /* DJB2 hash-like algorithm with carried dependency */
        hash = ((hash << 5) + hash) + buf[i];  /* hash = hash * 33 + buf[i] */
        
        /* Modify buffer based on previous hash value */
        if (i > 0) {
            buf[i] = (buf[i] + (hash & 0xFF)) ^ buf[i-1];  /* Distance-1 */
        }
        
        /* Additional arithmetic chain */
        int tmp = hash * 13;
        buf[i] = buf[i] + (tmp % 17);
        
        /* Force the compiler to keep all operations */
        asm volatile("" : : "r"(hash) : "memory");
    }
    return hash;
}

/* Main driver function */
int main(int argc, char *argv[]) {
    const int SIZE = 1024;
    const int N = 10000;
    int i, result = 0;
    
    /* Allocate and initialize test arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *x = (int*)malloc(SIZE * sizeof(int));
    int *y = (int*)malloc(SIZE * sizeof(int));
    int *buf = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        buf[i] = rand() % 256;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(SIZE, a, b, c);
    result += nested_loops(100, SIZE/100, x, y);
    result += multi_accumulators(SIZE, a, b);
    result += variable_loop(SIZE, 42, c);
    result += complex_mixed_ops(SIZE, buf);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(buf);
    
    return 0;
}
