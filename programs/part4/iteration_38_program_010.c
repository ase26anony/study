/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int func1(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Loop with distance-1 dependencies: a[i] depends on a[i-1] */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 carried dependence */
        sum += a[i];                   /* Accumulator with carried dependence */
        
        /* Inline asm to prevent optimization and create scheduling barriers */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    /* Additional operations to create more scheduling opportunities */
    for (i = 0; i < n-1; i++) {
        b[i] = a[i] + a[i+1] * 2;      /* Forward and backward dependencies */
    }
    
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int func2(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    /* Outer loop partially unrolled */
    for (i = 0; i < n; i += 2) {
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            x[j] = x[j-1] * y[j] + i;  /* Distance-1 dependence in inner loop */
            acc1 += x[j];
            
            /* Create multiple dependence edges */
            y[j] = y[j-1] + x[j] * 3;  /* Another distance-1 dependence */
            acc2 += y[j];
        }
        
        /* Memory barrier to preserve loop-carried dependencies */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int func3(int n, int *restrict p, int *restrict q, int *restrict r) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Loop with multiple interleaved carried dependencies */
    for (i = 2; i < n; i++) {
        /* Chain of dependencies creating complex scheduling graph */
        int t1 = p[i-1] * q[i];        /* Distance-1 use of p */
        int t2 = q[i-2] + r[i];        /* Distance-2 use of q */
        p[i] = t1 + t2;                /* Def of p[i] */
        
        sum1 += p[i];                  /* Accumulator 1 */
        sum2 += t1 * t2;               /* Accumulator 2 */
        
        /* Create anti-dependencies */
        r[i-1] = p[i] + q[i-1];        /* Def of r[i-1], use of p[i] and q[i-1] */
        sum3 += r[i-1];                /* Accumulator 3 */
        
        /* Force scheduler to consider all dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 * 2 + sum2 * 3 + sum3 * 5;
}

/* Function 4: Loop with variable trip count and mixed operations */
int func4(int n, int *restrict arr1, int *restrict arr2) {
    volatile int result = 0;
    int i;
    
    /* n is not known at compile time - prevents loop unrolling */
    for (i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        arr1[i] = (arr1[i-1] * 7) >> 2;      /* Distance-1, multiply + shift */
        arr2[i] = arr2[i-1] + arr1[i] * 3;   /* Distance-1, add + multiply */
        
        /* Complex expression creating multiple dependence edges */
        result += arr1[i] * arr2[i] - arr1[i-1];
        
        /* Conditional to create control flow, but predictable */
        if (i % 16 == 0) {
            result ^= arr2[i];               /* Occasional different operation */
        }
    }
    
    return result;
}

/* Function 5: Software pipelining candidate with high ILP */
int func5(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int acc = 0;
    int i;
    
    /* Loop designed to maximize instruction-level parallelism */
    for (i = 3; i < n; i++) {
        /* Independent chains that can be overlapped */
        int chain1 = a[i-1] * b[i] + 5;
        int chain2 = b[i-2] * c[i] - 3;
        int chain3 = c[i-3] + a[i] * 7;
        
        /* Combine chains with carried dependencies */
        a[i] = chain1 + chain2;
        b[i] = chain2 * chain3;
        c[i] = chain1 - chain3;
        
        /* Accumulator with multiple uses */
        acc += a[i] + b[i] + c[i];
        
        /* Create output dependencies */
        a[i-1] = b[i] >> 1;      /* Overwrites a[i-1] used in next iteration */
    }
    
    return acc;
}

/* Main driver that calls all functions */
int main(int argc, char *argv[]) {
    const int N = 1024;
    const int M = 512;
    int total = 0;
    
    /* Allocate and initialize arrays */
    int *a1 = (int*)malloc(N * sizeof(int));
    int *b1 = (int*)malloc(N * sizeof(int));
    int *c1 = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(M * sizeof(int));
    int *y = (int*)malloc(M * sizeof(int));
    int *p = (int*)malloc(N * sizeof(int));
    int *q = (int*)malloc(N * sizeof(int));
    int *r = (int*)malloc(N * sizeof(int));
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a1[i] = rand() % 100;
        b1[i] = rand() % 100;
        c1[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        r[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
    }
    for (int i = 0; i < M; i++) {
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call all functions to ensure they're compiled */
    total += func1(N, a1, b1, c1);
    total += func2(N/4, M, x, y);
    total += func3(N, p, q, r);
    total += func4(N, arr1, arr2);
    total += func5(N, a1, b1, c1);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(a1); free(b1); free(c1);
    free(x); free(y);
    free(p); free(q); free(r);
    free(arr1); free(arr2);
    
    return total != 0 ? 0 : 1;
}
