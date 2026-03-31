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
        c[i] = b[i+1] + sum;          /* Use of carried accumulator */
    }
    
    return sum;
}

/* Function 2: Nested loop with inner loop having carried dependencies */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    /* Outer loop partially unrolled */
    for (i = 0; i < n; i += 2) {
        /* Inner loop with carried dependencies */
        for (j = 1; j < m; j++) {
            /* Multiple distance-1 dependencies */
            x[j] = x[j-1] * y[j] + i;      /* Distance-1 on x */
            y[j] = y[j-1] + x[j] * 2;      /* Distance-1 on y */
            acc1 += x[j];
            acc2 += y[j];
            
            /* Force dependence preservation */
            asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
        }
        
        /* Cross-iteration dependency in outer loop */
        if (i + 1 < n) {
            x[0] = acc1 - acc2;  /* Use both accumulators */
        }
    }
    
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved carried dependencies */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2, 
                       int *restrict arr3) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < n; i++) {
        /* Three separate carried dependency chains */
        arr1[i] = arr1[i-1] * 2 + arr2[i];    /* Distance-1 on arr1 */
        arr2[i] = arr2[i-1] + arr3[i] * 3;    /* Distance-1 on arr2 */
        arr3[i] = arr3[i-1] - arr1[i];        /* Distance-1 on arr3 */
        
        /* Three separate accumulators with carried dependencies */
        sum1 = sum1 + arr1[i];
        sum2 = sum2 + arr2[i];
        sum3 = sum3 + arr3[i];
        
        /* Create artificial dependencies between accumulators */
        if (i % 4 == 0) {
            sum1 = sum1 ^ sum2;
            sum2 = sum2 + sum3;
        }
        
        /* Memory clobber to preserve all dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with runtime trip count (prevents unrolling) */
int runtime_trip_count(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* Dynamic loop count forces modulo scheduling analysis */
    for (i = 1; i < n; i++) {
        /* Strong distance-1 carried dependence */
        data[i] = data[i-1] * coeff + i;
        
        /* Accumulator with feedback */
        result = result + data[i] * (i % 8);
        
        /* Varying dependency pattern */
        if (i % 3 == 0) {
            coeff = coeff ^ (data[i] & 0xFF);
        }
        
        asm volatile("" : : "r"(result), "r"(coeff) : "memory");
    }
    
    return result;
}

/* Function 5: Complex loop with if-conversion opportunities */
int complex_conditional(int n, int *restrict a, int *restrict b, 
                        int *restrict c, int threshold) {
    volatile int count = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Distance-1 dependence */
        a[i] = a[i-1] + b[i];
        
        /* Conditional that creates different dependency paths */
        if (a[i] > threshold) {
            b[i] = b[i-1] * 2;      /* Distance-1 in taken path */
            count += a[i];
        } else {
            b[i] = b[i-1] / 2;      /* Distance-1 in not-taken path */
            count -= a[i];
        }
        
        /* Another carried dependency */
        c[i] = c[i-1] + count;
        
        asm volatile("" : : "r"(count) : "memory");
    }
    
    return count + c[n-1];
}

/* Initialize arrays with pseudo-random but deterministic values */
void init_arrays(int n, int *a, int *b, int *c) {
    int i;
    for (i = 0; i < n; i++) {
        a[i] = (i * 13) % 97;
        b[i] = (i * 17) % 101;
        c[i] = (i * 19) % 103;
    }
}

int main(int argc, char **argv) {
    const int N = 1024;
    const int M = 512;
    
    /* Allocate and initialize arrays */
    int *a1 = (int*)malloc(N * sizeof(int));
    int *b1 = (int*)malloc(N * sizeof(int));
    int *c1 = (int*)malloc(N * sizeof(int));
    
    int *a2 = (int*)malloc(N * sizeof(int));
    int *b2 = (int*)malloc(N * sizeof(int));
    int *c2 = (int*)malloc(N * sizeof(int));
    
    int *a3 = (int*)malloc(N * sizeof(int));
    int *b3 = (int*)malloc(N * sizeof(int));
    int *c3 = (int*)malloc(N * sizeof(int));
    
    init_arrays(N, a1, b1, c1);
    init_arrays(N, a2, b2, c2);
    init_arrays(N, a3, b3, c3);
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_deps(N, a1, b1, c1);
    result += nested_loop_carried(N/4, M, a2, b2);
    result += multi_accumulators(N, a3, b3, c3);
    
    /* Re-initialize for runtime trip count test */
    init_arrays(N, a1, b1, c1);
    result += runtime_trip_count(N, a1, 7);
    
    /* Re-initialize for conditional test */
    init_arrays(N, a2, b2, c2);
    result += complex_conditional(N, a2, b2, c2, 50);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a1); free(b1); free(c1);
    free(a2); free(b2); free(c2);
    free(a3); free(b3); free(c3);
    
    return 0;
}
