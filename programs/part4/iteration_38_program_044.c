/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int prod = 1;
    
    /* Loop with distance-1 dependencies:
       - a[i] depends on a[i-1] (carried dependency)
       - sum accumulates across iterations
       - prod has inter-iteration dependency */
    for (int i = 1; i < n; i++) {
        /* Multiple operations to create complex schedule */
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 dependence on a[i-1] */
        sum = sum + a[i];              /* Accumulator with carried dependency */
        prod = prod * (b[i] + 1);      /* Another carried dependency */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum), "r"(prod) : "memory");
    }
    
    return sum + prod;
}

/* Function 2: Nested loop with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    int total = 0;
    
    /* Outer loop partially unrolled */
    for (int i = 0; i < n; i += 2) {
        int inner_acc = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 1; j < m; j++) {
            /* Distance-1 dependence on y[j-1] */
            y[j] = y[j-1] + x[i % m] * j;
            inner_acc += y[j];
            
            /* Create additional dependencies */
            x[(i + j) % m] = x[(i + j - 1) % m] + inner_acc;
        }
        
        total += inner_acc;
        asm volatile("" : : "r"(total) : "memory");
    }
    
    return total;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int acc1 = 0;
    volatile int acc2 = 0;
    volatile int acc3 = 0;
    
    /* Loop with three separate carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Each accumulator has its own carried dependency */
        acc1 = acc1 + arr1[i] * arr2[i];
        acc2 = acc2 + arr1[i-1] + arr2[i];  /* Uses arr1[i-1] - distance-1 */
        acc3 = acc3 * 2 + arr1[i];
        
        /* Cross-iteration dependency chain */
        arr1[i] = arr1[i-1] + acc1 - acc2;
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(acc1), "r"(acc2), "r"(acc3) : "memory");
    }
    
    return acc1 + acc2 + acc3;
}

/* Function 4: Complex loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict data, int threshold) {
    int count = 0;
    volatile int sum = 0;
    
    /* Loop with conditional carried dependency */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence on data[i-1] */
        int temp = data[i-1] * 3 + i;
        
        if (temp > threshold) {
            sum = sum + temp;  /* Carried dependency in one path */
            data[i] = data[i-1] + 1;  /* Another distance-1 dependence */
        } else {
            sum = sum - temp;  /* Same accumulator, different operation */
            data[i] = data[i-1] - 1;  /* Distance-1 dependence */
        }
        
        count += (temp % 2);
        asm volatile("" : : "r"(sum), "r"(count) : "memory");
    }
    
    return sum * count;
}

/* Function 5: Loop with pointer chasing (harder to optimize away) */
int pointer_chasing_loop(int n, int *restrict base) {
    volatile int *ptr = base;
    int result = 0;
    
    /* Create an artificial pointer-chasing pattern */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependency through pointer */
        int value = *ptr;
        result = result + value * i;  /* Accumulator with carried dependency */
        
        /* Move pointer with stride that creates dependency */
        ptr = base + (value % 16);
        
        /* Force memory dependency */
        asm volatile("" : : "r"(result), "r"(ptr) : "memory");
    }
    
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    const int SIZE = 1024;
    const int N = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Allocate and initialize test arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    int *x = (int *)malloc(SIZE * sizeof(int));
    int *y = (int *)malloc(SIZE * sizeof(int));
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *arr2 = (int *)malloc(SIZE * sizeof(int));
    int *data = (int *)malloc(SIZE * sizeof(int));
    int *base = (int *)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
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
        base[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_deps(N, a, b, c);
    result += nested_loop_carried(N/10, 32, x, y);
    result += multi_accumulators(N, arr1, arr2);
    result += conditional_loop(N, data, 50);
    result += pointer_chasing_loop(N/2, base);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data); free(base);
    
    return 0;
}
