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
        int temp = a[i-1] * 3;      /* Distance-1 use of a[i-1] */
        a[i] = temp + b[i] * c[i];  /* Def of a[i] */
        sum += a[i];                 /* Accumulator with carried dependency */
        
        /* Additional operation with another carried dependency */
        b[i] = b[i-1] + i;          /* Another distance-1 dependence */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = x[i];
        /* Inner loop with carried dependency on acc */
        for (j = 1; j < m; j++) {
            acc = acc * 2 + y[i * m + j];  /* Strong carried dependency */
            x[i] += acc;
            
            /* Create complex addressing with dependency */
            int idx = i * m + j;
            y[idx] = y[idx - 1] + (idx % 7);  /* Distance-1 array dependence */
        }
        total += acc;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators with unknown trip count */
int multi_accumulator(int n, int *restrict data, int coeff1, int coeff2) {
    volatile int sum1 = 0, sum2 = 0;
    int i;
    
    /* Initialize with first element */
    sum1 = data[0] * coeff1;
    sum2 = data[0] * coeff2;
    
    for (i = 1; i < n; i++) {
        /* Two separate accumulators with carried dependencies */
        sum1 = sum1 + data[i] * coeff1;      /* Distance-1 on sum1 */
        sum2 = sum2 + data[i-1] * coeff2;    /* Distance-1 on data[i-1] */
        
        /* Cross-dependency between accumulators */
        data[i] = (sum1 % 256) + (sum2 % 256);
        
        /* Complex operation to create more ILP opportunities */
        int temp = (sum1 * sum2) >> 3;
        sum1 ^= temp;
        sum2 += temp;
        
        /* Memory clobber to preserve dependencies */
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    return sum1 + sum2;
}

/* Function 4: Loop with if-converted dependencies */
int conditional_carried(int n, int *restrict arr, int threshold) {
    volatile int count = 0;
    int last_val = arr[0];
    int i;
    
    for (i = 1; i < n; i++) {
        /* Conditional with carried dependency */
        if (last_val > threshold) {
            arr[i] = arr[i-1] * 2;      /* Distance-1 dependence */
            count += arr[i];
        } else {
            arr[i] = arr[i-1] / 2;      /* Alternative distance-1 dependence */
            count -= arr[i];
        }
        
        /* Update carried variable */
        last_val = arr[i] + (i % 5);
        
        /* Prevent optimization */
        asm volatile("" : : "r"(count), "r"(last_val) : "memory");
    }
    return count;
}

/* Function 5: Reduction loop with pointer chasing */
int pointer_chasing_reduction(int n, int *restrict base) {
    volatile int result = 0;
    int *ptr = base;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Pointer arithmetic with dependency */
        result += *ptr;
        ptr = base + (result % (n > 1 ? n : 2));  /* Carried dependency through result */
        
        /* Additional computation */
        *ptr = (*ptr * 1103515245 + 12345) & 0x7fffffff;
        
        /* Memory barrier */
        asm volatile("" : : "r"(result) : "memory");
    }
    return result;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int i;
    const int SIZE = 1024;
    const int NESTED_SIZE = 64;
    
    /* Allocate and initialize test arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *x = (int*)malloc(NESTED_SIZE * NESTED_SIZE * sizeof(int));
    int *y = (int*)malloc(NESTED_SIZE * NESTED_SIZE * sizeof(int));
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *arr = (int*)malloc(SIZE * sizeof(int));
    int *base = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        data[i] = rand() % 100;
        arr[i] = rand() % 100;
        base[i] = rand() % 100;
    }
    
    for (i = 0; i < NESTED_SIZE * NESTED_SIZE; i++) {
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call all test functions with different parameters */
    int result1 = loop_carried_deps(SIZE, a, b, c);
    int result2 = nested_loop_carried(NESTED_SIZE, NESTED_SIZE, x, y);
    int result3 = multi_accumulator(SIZE, data, 3, 7);
    int result4 = conditional_carried(SIZE, arr, 50);
    int result5 = pointer_chasing_reduction(SIZE, base);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + result4 + result5;
    
    printf("Final checksum: %d\n", final_result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(data); free(arr); free(base);
    
    return 0;
}
