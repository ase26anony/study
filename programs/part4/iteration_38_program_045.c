/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    /* This should set distance1_uses = true */
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 carried dependence */
        sum += a[i];                   /* Accumulator with carried dependence */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    
    /* Additional operations to create more scheduling opportunities */
    for (i = 0; i < n-1; i++) {
        b[i+1] = b[i] * 3 - a[i];     /* Another distance-1 dependence */
        c[i] = c[i+1] + sum;          /* Reverse access pattern */
    }
    
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    /* Outer loop partially unrolled */
    for (i = 0; i < n; i += 2) {
        /* Inner loop with carried dependency */
        for (j = 1; j < m; j++) {
            x[j] = x[j-1] * y[j] + i;  /* Distance-1 in inner loop */
            acc1 += x[j];
            
            /* Create multiple dependence edges */
            y[j] = y[j-1] + x[j] * 7;
            acc2 ^= y[j];  /* Different operation to avoid simplification */
        }
        
        /* Cross-iteration dependency in outer loop */
        if (i > 0) {
            x[0] = x[m-1] + acc1;
        }
        
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved accumulators with complex addressing */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2, 
                      int *restrict arr3, int *restrict arr4) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < n; i++) {
        /* Pattern 1: Simple distance-1 */
        arr1[i] = arr1[i-1] * 2 + arr2[i];
        sum1 += arr1[i];
        
        /* Pattern 2: Distance-1 with offset */
        arr2[i] = arr3[i-1] + arr4[i] * 3;
        sum2 ^= arr2[i];  /* Different operation */
        
        /* Pattern 3: Two-step dependency chain */
        arr3[i] = arr1[i] + arr2[i-1];
        sum3 = sum3 * 5 + arr3[i];
        
        /* Pattern 4: Modulo addressing creates complex dependencies */
        arr4[i] = arr4[(i-1+n)%n] + sum1;
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 * 7 + sum3;
}

/* Function 4: Loop with runtime trip count and mixed operations */
int runtime_trip_count(int n, int seed) {
    volatile int result = seed;
    int *tmp = (int*)malloc(n * sizeof(int));
    int i;
    
    if (!tmp) return -1;
    
    /* Initialize with pattern */
    for (i = 0; i < n; i++) {
        tmp[i] = i * 3 + seed;
    }
    
    /* Main loop with carried dependencies - n is runtime value */
    for (i = 1; i < n; i++) {
        /* Multiple distance-1 dependencies */
        tmp[i] = tmp[i-1] * 7 + tmp[i];
        result += tmp[i];
        
        /* Additional operations to create more edges */
        if (i % 3 == 0) {
            result = result ^ (tmp[i-1] * 11);
        } else {
            result = result * 13 - tmp[i];
        }
        
        /* Complex addressing */
        tmp[(i*17) % n] = result + i;
        
        asm volatile("" : : "r"(result) : "memory");
    }
    
    free(tmp);
    return result;
}

/* Function 5: Software pipelining candidate with high ILP */
int high_ilp_loop(int n, int *restrict a, int *restrict b, 
                  int *restrict c, int *restrict d) {
    volatile int acc[4] = {0, 0, 0, 0};
    int i;
    
    /* Loop designed to maximize instruction-level parallelism */
    for (i = 4; i < n; i++) {
        /* Multiple independent chains with carried dependencies */
        a[i] = a[i-1] * b[i-2] + c[i-3] - d[i-4];
        acc[0] += a[i];
        
        b[i] = b[i-1] + a[i-2] * 3 - c[i-3];
        acc[1] ^= b[i];
        
        c[i] = c[i-1] * 5 + b[i-2] - a[i-3];
        acc[2] = acc[2] * 7 + c[i];
        
        d[i] = d[i-1] + acc[0] - acc[1] + acc[2];
        acc[3] += d[i] * 11;
        
        /* Force all accumulators to be live */
        asm volatile("" : : "r"(acc[0]), "r"(acc[1]), "r"(acc[2]), "r"(acc[3]) : "memory");
    }
    
    return acc[0] + acc[1] + acc[2] + acc[3];
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    const int N = 1024;
    int *a, *b, *c, *d, *x, *y;
    int result = 0;
    
    /* Allocate and initialize arrays */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    x = (int*)malloc(N * sizeof(int));
    y = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < N; i++) {
        a[i] = i + 1;
        b[i] = i * 2 + 3;
        c[i] = i * 3 + 5;
        d[i] = i * 5 + 7;
        x[i] = i * 7 + 11;
        y[i] = i * 11 + 13;
    }
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_deps(N/4, 16, x, y);
    result += multi_accumulator(N, a, b, c, d);
    result += runtime_trip_count(N, 42);
    result += high_ilp_loop(N, a, b, c, d);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(x); free(y);
    
    return 0;
}
