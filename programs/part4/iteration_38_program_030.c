/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler debug output
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Prevent compiler from optimizing away loops */
static void escape(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependency */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations with carried dependencies */
        int temp = a[i-1] * 3;      /* Distance-1 use of a[i-1] */
        a[i] = temp + b[i] * c[i];  /* Store creates new value for next iteration */
        sum += a[i];                /* Accumulator with carried dependency */
        
        /* Additional dependency chain */
        b[i] = b[i-1] + a[i] / 2;   /* Another distance-1 dependence */
    }
    
    escape(&sum);
    return sum;
}

/* Function 2: Nested loops with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc1 = 0, acc2 = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Initialize for this i */
        int local_acc = x[i];
        
        for (j = 1; j < m; j++) {
            /* Inner loop with carried dependency across j */
            local_acc = local_acc * 7 + y[(i * m) + j];  /* Distance-1 in j dimension */
            
            /* Interleaved dependency */
            y[(i * m) + j] = y[(i * m) + j - 1] + local_acc;  /* Another distance-1 */
            
            acc1 += local_acc;
            acc2 ^= y[(i * m) + j];  /* Separate accumulator */
        }
        
        /* Force dependency between outer loop iterations */
        x[i] = (i > 0) ? x[i-1] + local_acc : local_acc;
    }
    
    escape(&acc1);
    escape(&acc2);
    return acc1 + acc2;
}

/* Function 3: Multiple interleaved carried dependencies */
int multi_accumulator_loop(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first elements with dependency */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Three separate dependency chains */
        int chain1 = arr1[i-1] * 5 + i;      /* Distance-1 use */
        int chain2 = arr2[i-1] + chain1 * 3; /* Distance-1 use, depends on chain1 */
        int chain3 = sum1 * 2 - chain2;      /* Depends on accumulator */
        
        arr1[i] = chain1;
        arr2[i] = chain2;
        
        /* Update accumulators with carried dependencies */
        sum1 = sum1 + chain1;      /* Simple accumulator */
        sum2 = sum2 * 3 + chain2;  /* Multiplicative accumulator */
        sum3 = sum3 ^ chain3;      /* XOR accumulator */
        
        /* Cross-dependency between accumulators */
        if (i % 4 == 0) {
            sum1 = sum1 + sum2;
            sum2 = sum2 ^ sum3;
        }
    }
    
    escape(&sum1);
    escape(&sum2);
    escape(&sum3);
    return sum1 + sum2 + sum3;
}

/* Function 4: Loop with unknown trip count (parameter) */
int variable_trip_loop(int n, int *restrict data) {
    volatile int result = 0;
    int i;
    
    if (n <= 0) return 0;
    
    /* Initialize first element */
    data[0] = n;
    
    for (i = 1; i < n; i++) {
        /* Complex carried dependency pattern */
        int prev = data[i-1];
        int curr = prev * 13 + i * 17;
        
        /* Multiple uses of the carried value */
        data[i] = curr;
        result = result + (prev % 7) * (curr % 11);
        
        /* Additional operation with memory dependency */
        if (i > 2) {
            data[i] = data[i] + data[i-2] / 3;  /* Distance-2 dependence */
        }
        
        /* Inline asm to prevent optimization but preserve dependencies */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    escape(&result);
    return result;
}

/* Function 5: Loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int count = 0;
    int i;
    
    a[0] = 1;
    b[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through a */
        int base = a[i-1] * 2;
        
        /* Conditional update creates complex dependency graph */
        if (base > threshold) {
            a[i] = base - b[i-1];      /* Distance-1 use of b */
            b[i] = a[i] * 3;
        } else {
            a[i] = base + b[i-1];      /* Distance-1 use of b */
            b[i] = a[i] / 2;
        }
        
        /* Accumulator with carried dependency */
        count = count + a[i] - b[i];
        
        /* Additional operation to increase ILP */
        a[i] = a[i] ^ (count & 0xFF);
    }
    
    escape(&count);
    return count;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int i;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *x = (int*)malloc(SIZE * SIZE * sizeof(int));
    int *y = (int*)malloc(SIZE * SIZE * sizeof(int));
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 + 2;
        c[i] = i * 7 + 3;
        arr1[i] = i * 11 + 5;
        arr2[i] = i * 13 + 7;
        data[i] = i * 17 + 11;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        x[i] = i * 19 + 13;
        y[i] = i * 23 + 17;
    }
    
    int result = 0;
    
    /* Call all test functions multiple times to ensure they're compiled */
    for (i = 0; i < 10; i++) {
        result += loop_carried_deps(SIZE, a, b, c);
        result += nested_loop_carried(32, 32, x, y);
        result += multi_accumulator_loop(SIZE, arr1, arr2);
        result += variable_trip_loop(SIZE, data);
        result += conditional_loop(SIZE, a, b, 1000);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(x);
    free(y);
    free(arr1);
    free(arr2);
    free(data);
    
    return 0;
}
