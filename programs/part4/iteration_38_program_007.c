/* test_modulo_sched.c - Program to trigger GCC modulo scheduler debug output */
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
        /* Multiple carried dependencies to create complex schedule */
        a[i] = a[i-1] * b[i] + c[i];      /* distance-1 dependence */
        sum = sum + a[i];                  /* accumulator pattern */
        b[i] = b[i-1] + sum;               /* another distance-1 dependence */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Outer loop with some computation */
        acc1 = acc1 + x[i];
        
        /* Inner loop with carried dependency */
        y[0] = x[i] * 2;
        for (j = 1; j < m; j++) {
            /* Distance-1 dependence in inner loop */
            y[j] = y[j-1] + x[i] * j + acc1;
            acc2 = acc2 + y[j];
            
            /* Force dependency preservation */
            asm volatile("" : : "r"(acc2) : "memory");
        }
        
        /* Cross-iteration dependency */
        x[i] = acc2 % 100;
    }
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first element with dependency */
    arr1[0] = arr2[0] * 3;
    arr2[0] = arr1[0] + 1;
    
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + arr1[i-1] * arr2[i];    /* distance-1 use of arr1 */
        sum2 = sum2 + arr2[i-1] + arr1[i];    /* distance-1 use of arr2 */
        sum3 = sum3 * 2 + sum1 + sum2;        /* complex accumulator */
        
        /* Update arrays with carried dependencies */
        arr1[i] = arr1[i-1] + sum1;           /* another distance-1 */
        arr2[i] = arr2[i-1] * sum2 + i;       /* distance-1 on arr2 */
        
        /* Multiple memory barriers to ensure dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with if-converted dependencies */
int conditional_deps(int n, int *restrict data, int threshold) {
    volatile int count = 0;
    volatile int last_val = data[0];
    int i;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through last_val */
        int temp = last_val * data[i];
        
        /* Conditional update creates complex dependency graph */
        if (temp > threshold) {
            last_val = temp - threshold;
            count = count + last_val;
        } else {
            last_val = temp + data[i-1];  /* distance-1 use of data */
            count = count - last_val;
        }
        
        /* Store with potential dependency */
        data[i] = last_val + i;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(count) : "memory");
    }
    
    return count;
}

/* Function 5: Reduction with multiple dependency chains */
int complex_reduction(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int r1 = a[0], r2 = b[0], r3 = c[0];
    int i;
    
    for (i = 1; i < n; i++) {
        /* Three parallel reduction chains with inter-dependencies */
        int t1 = r1 * a[i] + r3;
        int t2 = r2 + b[i] * r1;
        int t3 = r3 - c[i] + r2;
        
        /* Update with cross-iteration dependencies */
        r1 = t1 + a[i-1];      /* distance-1 use of a */
        r2 = t2 * b[i-1];      /* distance-1 use of b */
        r3 = t3 + r1 + r2;     /* depends on both r1 and r2 */
        
        /* Force all dependencies to be preserved */
        asm volatile("" : : "r"(r1), "r"(r2), "r"(r3) : "memory");
    }
    
    return r1 + r2 + r3;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    const int N = 1000;
    const int M = 100;
    int *a, *b, *c, *x, *y, *arr1, *arr2, *data;
    int result = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    x = (int*)malloc(N * sizeof(int));
    y = (int*)malloc(M * sizeof(int));
    arr1 = (int*)malloc(N * sizeof(int));
    arr2 = (int*)malloc(N * sizeof(int));
    data = (int*)malloc(N * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
    }
    for (int i = 0; i < M; i++) {
        y[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_deps(N/10, M, x, y);
    result += multi_accumulators(N, arr1, arr2);
    result += conditional_deps(N, data, 5000);
    result += complex_reduction(N, a, b, c);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(x); free(y);
    free(arr1); free(arr2); free(data);
    
    return 0;
}
