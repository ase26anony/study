/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] * c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        int temp = a[i-1] * b[i];      /* distance-1 use of a[i-1] */
        a[i] = temp + c[i];            /* defines a[i] for next iteration */
        sum += a[i] * 2;               /* accumulator with carried dependency */
        
        /* Additional operation to create more ILP */
        b[i] = (b[i-1] + 1) & 0xFF;    /* another distance-1 dependence */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Initialize for this i */
        int base = x[i];
        
        for (j = 1; j < m; j++) {
            /* Inner loop with carried dependency across j */
            y[j] = y[j-1] + base * j;  /* distance-1 in inner loop */
            acc1 += y[j];
            
            /* Another carried dependency */
            acc2 = acc2 * 2 + y[j];
            
            /* Force dependence preservation */
            asm volatile("" : "+r"(acc1), "+r"(acc2) : : "memory");
        }
        
        /* Cross-iteration dependency */
        x[i+1] = acc1 & 0xFF;
    }
    
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulators(int n, int *restrict data, int coeff1, int coeff2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Loop with parameterized bound (unknown at compile time) */
    for (i = 1; i < n; i++) {
        /* Three separate carried dependencies */
        sum1 = sum1 + data[i] * coeff1;          /* accumulator 1 */
        sum2 = sum2 + data[i-1] * coeff2;        /* uses previous element */
        sum3 = sum3 * 2 + (sum1 & 0xF);          /* depends on sum1 */
        
        /* Complex operation chain */
        data[i] = (data[i-1] + sum1 - sum2) & 0xFF;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    
    return sum1 + sum2 * 2 + sum3 * 3;
}

/* Function 4: Loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict arr, int threshold) {
    volatile int count = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through count */
        int diff = arr[i] - arr[i-1];  /* distance-1 use */
        
        if (diff > threshold) {
            count = count * 3 + diff;  /* carried dependency in true path */
            arr[i] = arr[i-1] + 1;     /* another distance-1 */
        } else {
            count = count / 2 - diff;  /* carried dependency in false path */
            arr[i] = arr[i-1] - 1;     /* distance-1 */
        }
        
        /* Memory clobber */
        asm volatile("" : : "r"(count) : "memory");
    }
    
    return count;
}

/* Function 5: Reduction with multiple dependency chains */
int complex_reduction(int n, int *restrict a, int *restrict b) {
    volatile int r1 = 0, r2 = 1, r3 = 0;
    int i;
    
    /* Initialize first element */
    a[0] = b[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: a[i] depends on a[i-1] and b[i] */
        int t1 = a[i-1] * 3;           /* distance-1 */
        int t2 = b[i] * 7;
        a[i] = (t1 + t2) & 0xFFFF;
        
        /* Chain 2: r1 accumulates with carried dependency */
        r1 = r1 + a[i] - b[i-1];       /* uses b[i-1], distance-1 */
        
        /* Chain 3: r2 has multiplicative accumulation */
        r2 = r2 * (a[i] & 0xF) + 1;
        
        /* Chain 4: r3 depends on r1 from previous iteration indirectly */
        r3 = r3 ^ (r1 & 0xFF);
        
        /* Force all dependencies to be preserved */
        asm volatile("" : : "r"(r1), "r"(r2), "r"(r3) : "memory");
    }
    
    return r1 + r2 + r3;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *x = (int*)malloc((n+1) * sizeof(int));
    int *y = (int*)malloc(m * sizeof(int));
    int *data = (int*)malloc(n * sizeof(int));
    int *arr = (int*)malloc(n * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 7) & 0xFF;
        b[i] = (i * 13) & 0xFF;
        c[i] = (i * 19) & 0xFF;
        x[i] = (i * 23) & 0xFF;
        data[i] = (i * 29) & 0xFF;
        arr[i] = (i * 31) & 0xFF;
    }
    for (int j = 0; j < m; j++) {
        y[j] = (j * 37) & 0xFF;
    }
    
    int result = 0;
    
    /* Call all test functions to ensure they're compiled */
    result += loop_carried_deps(n, a, b, c);
    result += nested_loop_deps(10, m, x, y);
    result += multi_accumulators(n, data, 3, 7);
    result += conditional_loop(n, arr, 50);
    result += complex_reduction(n, a, b);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(x); free(y); free(data); free(arr);
    
    return result != 0 ? 0 : 1;
}
