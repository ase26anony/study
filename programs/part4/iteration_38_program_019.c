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
        
        /* Add some computation to create more ILP opportunities */
        b[i] = (b[i] * 3) / 2;
        c[i] = c[i] + i;
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loops with inner loop carried dependency */
int nested_loops(int n, int m, int *restrict x, int *restrict y) {
    volatile int acc = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Inner loop with multiple carried dependencies */
        int temp = x[i];
        for (j = 1; j < m; j++) {
            /* Multiple distance-1 dependencies */
            temp = temp * y[j] + (j * 2);
            y[j] = y[j-1] + temp;  /* Another distance-1 dependence */
            acc += temp;
            
            /* Additional operations for scheduler complexity */
            y[j] = y[j] ^ (j * 3);
            temp = temp + (j & 0xFF);
        }
        x[i] = temp;
        asm volatile("" : : "r"(acc) : "memory");
    }
    return acc;
}

/* Function 3: Multiple interleaved accumulators with complex dependencies */
int multi_accumulators(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0;
    int prod = 1;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Two separate carried dependencies */
        sum1 = sum1 + arr1[i] * 2;      /* Accumulator 1 */
        sum2 = sum2 + arr2[i] * 3;      /* Accumulator 2 */
        
        /* Cross-iteration product with distance-1 */
        prod = prod * (arr1[i] + 1);
        
        /* Array with distance-1 dependence */
        arr1[i] = arr1[i-1] + sum1;
        arr2[i] = arr2[i-1] + sum2;
        
        /* Complex computation to increase ILP */
        int tmp = (sum1 * sum2) / (prod + 1);
        arr1[i] = arr1[i] ^ tmp;
        arr2[i] = arr2[i] | tmp;
        
        asm volatile("" : : "r"(sum1), "r"(sum2) : "memory");
    }
    return sum1 + sum2 + prod;
}

/* Function 4: Loop with unknown trip count (prevents unrolling) */
int variable_trip_count(int n, int *restrict data, int coeff) {
    volatile int result = 0;
    int i;
    
    /* Unknown n forces modulo scheduling analysis */
    for (i = 1; i < n; i++) {
        /* Strong carried dependency chain */
        data[i] = data[i-1] * coeff + data[i];
        result += data[i] * i;
        
        /* Additional operations to create scheduling opportunities */
        coeff = (coeff * 13) % 17;  /* Varying coefficient */
        data[i] = data[i] ^ (i * 7);
        
        asm volatile("" : : "r"(result) : "memory");
    }
    return result;
}

/* Function 5: Complex loop with if-conversion opportunities */
int conditional_loop(int n, int *restrict a, int *restrict b, int threshold) {
    volatile int count = 0;
    int sum = 0;
    int i;
    
    for (i = 1; i < n; i++) {
        /* Carried dependency through sum */
        sum = sum + a[i];
        
        /* Conditional that creates control dependencies */
        if (sum > threshold) {
            b[i] = b[i-1] * 2;  /* Distance-1 dependence in conditional path */
            count++;
        } else {
            b[i] = b[i-1] / 2;  /* Distance-1 dependence in else path */
        }
        
        /* More computation */
        a[i] = a[i] + (i % 16);
        sum = sum ^ (b[i] & 0xFF);
        
        asm volatile("" : : "r"(count), "r"(sum) : "memory");
    }
    return count + sum;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int n, int seed) {
    int i;
    srand(seed);
    for (i = 0; i < n; i++) {
        a[i] = rand() % 100;
    }
}

int main(void) {
    const int SIZE = 1024;
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *x = malloc(SIZE * sizeof(int));
    int *y = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_arrays(a, SIZE, 42);
    init_arrays(b, SIZE, 43);
    init_arrays(c, SIZE, 44);
    init_arrays(x, SIZE, 45);
    init_arrays(y, SIZE, 46);
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    
    result += loop_carried_deps(SIZE, a, b, c);
    result += nested_loops(SIZE/16, 16, x, y);
    result += multi_accumulators(SIZE, a, b);
    result += variable_trip_count(SIZE, c, 5);
    result += conditional_loop(SIZE, x, y, 1000);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(x); free(y);
    
    return 0;
}
