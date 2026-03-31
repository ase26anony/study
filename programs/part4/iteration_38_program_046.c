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
        int temp = a[i-1] * 3;      /* distance-1 use */
        a[i] = temp + b[i] * c[i];  /* creates complex dependence graph */
        sum += a[i];                /* accumulator with carried dependency */
        
        /* Additional operation with another carried dependency */
        b[i] = b[i-1] + c[i];       /* another distance-1 dependence */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loop with inner loop having carried dependencies */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = 0;
        /* Inner loop with multiple carried dependencies */
        for (j = 1; j < m; j++) {
            /* Pattern: y[j] depends on y[j-1] and x[j-1] */
            y[j] = y[j-1] * 2 + x[j-1];  /* Two distance-1 dependencies */
            acc += y[j] * (i + 1);        /* Accumulator with outer loop dependency */
            
            /* Cross-iteration dependency through x */
            x[j] = x[j-1] + acc % 7;
        }
        total += acc;
        
        /* Prevent loop unrolling from eliminating modulo scheduling opportunity */
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    /* Initialize first elements */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* First accumulator chain with distance-1 dependence */
        int val1 = arr1[i-1] * 5 + i;    /* distance-1 use */
        arr1[i] = val1 % 1023;
        sum1 += arr1[i] * arr2[i-1];     /* cross-array distance-1 */
        
        /* Second accumulator chain with different distance-1 pattern */
        int val2 = arr2[i-1] * 3 - i;    /* another distance-1 use */
        arr2[i] = val2 % 511;
        sum2 += arr2[i] + arr1[i-1];     /* interleaved distance-1 */
        
        /* Complex operation mixing both chains */
        arr1[i] = (arr1[i] + arr2[i-1]) * (sum1 % 31);
        
        /* Force dependence preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    return sum1 + sum2;
}

/* Function 4: Loop with unknown trip count (prevents complete unrolling) */
int variable_loop(int n, int init_val) {
    volatile int state = init_val;
    int i;
    
    /* Fibonacci-like recurrence with multiple dependencies */
    int prev1 = 1, prev2 = 1;
    for (i = 0; i < n; i++) {
        int next = prev1 + prev2;      /* distance-1 and distance-2 dependencies */
        state += next * (i + 1);       /* accumulator with loop index */
        
        /* Update for next iteration */
        prev2 = prev1;
        prev1 = next % 1000;
        
        /* Create additional memory dependencies */
        asm volatile("" : "+r"(state) : : "memory");
    }
    return state;
}

/* Function 5: Complex loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict data, int threshold) {
    volatile int count = 0;
    int i;
    
    int prev = data[0];
    for (i = 1; i < n; i++) {
        /* Distance-1 dependence through prev */
        int curr = data[i] + prev;      /* distance-1 use */
        
        /* Conditional that creates control dependencies */
        if (curr > threshold) {
            prev = curr / 2;            /* carried dependency inside branch */
            count += prev;
        } else {
            prev = curr * 3;            /* alternative carried dependency */
            count -= prev;
        }
        
        /* Additional operation with another carried dependency */
        data[i] = prev + i;             /* modifies array element */
        
        /* Memory barrier */
        asm volatile("" : : "r"(count) : "memory");
    }
    return count;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    const int N = 1000;
    const int M = 500;
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    int *x = (int *)malloc(N * sizeof(int));
    int *y = (int *)malloc(N * sizeof(int));
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *data = (int *)malloc(N * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        x[i] = rand() % 100;
        y[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 200;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_carried(10, M, x, y);
    result += multi_accumulator(N, arr1, arr2);
    result += variable_loop(N, 42);
    result += conditional_loop(N, data, 150);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data);
    
    return 0;
}
