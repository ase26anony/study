/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Basic loop with multiple carried dependencies */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int prod = 1;
    
    /* Loop with distance-1 dependencies:
       - sum depends on previous iteration (sum = sum + ...)
       - a[i] depends on a[i-1] 
       - prod depends on previous iteration */
    for (int i = 1; i < n; i++) {
        /* Multiple operations to create complex schedule */
        a[i] = a[i-1] * b[i] + c[i];      /* Distance-1 dependence on a[i-1] */
        sum = sum + a[i] * 3;             /* Accumulator with distance-1 */
        prod = prod * (b[i] + 1);         /* Another distance-1 dependence */
        c[i] = c[i-1] + sum / 2;          /* Distance-1 on c[i-1] */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum), "r"(prod) : "memory");
    }
    
    return sum + prod;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loop_deps(int n, int m, int *restrict x, int *restrict y) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        volatile int inner_sum = 0;
        int prev = x[i * m];  /* Start value for inner loop */
        
        /* Inner loop with carried dependency */
        for (int j = 1; j < m; j++) {
            int idx = i * m + j;
            prev = prev * 2 + y[idx];      /* Distance-1 in inner loop */
            inner_sum = inner_sum + prev;  /* Another distance-1 */
            x[idx] = prev + inner_sum;     /* Store result */
            
            /* Force dependency preservation */
            asm volatile("" : : "r"(prev), "r"(inner_sum) : "memory");
        }
        
        total += inner_sum;
    }
    
    return total;
}

/* Function 3: Multiple interleaved accumulators */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int acc1 = 0;
    volatile int acc2 = 0;
    volatile int acc3 = 0;
    
    /* Three separate carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Each accumulator has its own distance-1 dependence */
        acc1 = acc1 + arr1[i] * arr2[i];
        acc2 = acc2 - arr1[i-1] + arr2[i];  /* Explicit distance-1 use */
        acc3 = acc3 * 2 + arr1[i];
        
        /* Cross-iteration array dependency */
        arr1[i] = arr1[i-1] + acc1 - acc2;
        arr2[i] = arr2[i-1] * acc3;
        
        /* Memory clobber to prevent optimization */
        asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3) : "memory");
    }
    
    return acc1 + acc2 + acc3;
}

/* Function 4: Complex loop with if-conversion potential */
int conditional_loop(int n, int *restrict data, int threshold) {
    volatile int hot_sum = 0;
    volatile int cold_sum = 0;
    int prev_hot = data[0];
    int prev_cold = data[0] * 2;
    
    for (int i = 1; i < n; i++) {
        /* Branch creates complex scheduling constraints */
        if (data[i] > threshold) {
            prev_hot = prev_hot * 3 + data[i];      /* Distance-1 */
            hot_sum = hot_sum + prev_hot;           /* Distance-1 */
            data[i] = prev_hot - prev_cold;
        } else {
            prev_cold = prev_cold / 2 + data[i];    /* Distance-1 */
            cold_sum = cold_sum + prev_cold;        /* Distance-1 */
            data[i] = prev_cold + prev_hot;
        }
        
        /* Force both paths to be considered */
        asm volatile("" : : "r"(hot_sum), "r"(cold_sum) : "memory");
    }
    
    return hot_sum - cold_sum;
}

/* Function 5: Loop with pointer chasing (harder to optimize) */
int pointer_chasing_loop(int n, int *restrict base) {
    volatile int result = 0;
    int *current = base;
    
    /* Pointer-based distance-1 dependency */
    for (int i = 0; i < n; i++) {
        int val = *current;
        result = result + val * i;          /* Distance-1 accumulator */
        
        /* Update pointer with stride that's not compile-time known */
        int stride = (val % 4) + 1;         /* Non-constant stride */
        current = current + stride;
        
        /* Prevent optimization of pointer arithmetic */
        asm volatile("" : : "r"(result), "r"(current) : "memory");
        
        /* Boundary check */
        if (current >= base + n) {
            current = base;
        }
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    
    /* Allocate and initialize test arrays */
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    int *c = (int *)malloc(n * sizeof(int));
    int *x = (int *)malloc(n * m * sizeof(int));
    int *y = (int *)malloc(n * m * sizeof(int));
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    int *data = (int *)malloc(n * sizeof(int));
    int *base = (int *)malloc(n * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < n; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 - 2;
        c[i] = i * 7 + 3;
        arr1[i] = i * 11 - 5;
        arr2[i] = i * 13 + 7;
        data[i] = i * 17 % 100;
        base[i] = i * 19 % 50;
    }
    
    for (int i = 0; i < n * m; i++) {
        x[i] = i * 2 + 1;
        y[i] = i * 3 - 1;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_deps(n, a, b, c);
    printf("Result 1: %d\n", result);
    
    result += nested_loop_deps(10, m, x, y);
    printf("Result 2: %d\n", result);
    
    result += multi_accumulators(n, arr1, arr2);
    printf("Result 3: %d\n", result);
    
    result += conditional_loop(n, data, 50);
    printf("Result 4: %d\n", result);
    
    result += pointer_chasing_loop(n, base);
    printf("Result 5: %d\n", result);
    
    /* Free allocated memory */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data); free(base);
    
    return result != 0 ? 0 : 1;
}
