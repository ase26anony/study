/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Single accumulator with loop-carried dependency */
int test_accumulator(int n, int* data, int coeff) {
    volatile int sum = 0;  /* volatile prevents optimization of carried dependency */
    for (int i = 0; i < n; i++) {
        /* Multiple operations to create instruction-level parallelism */
        int temp = data[i] * coeff;
        sum = sum + temp;           /* Distance-1 dependence on sum */
        data[i] = sum;              /* Store with dependency */
        
        /* Additional operations to create more edges */
        int extra = data[i] ^ 0x55;
        sum = sum ^ extra;          /* Another distance-1 dependence */
        
        /* Memory barrier to preserve dependencies */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Multiple interleaved dependencies */
int test_multiple_deps(int n, int* a, int* b, int* c) {
    volatile int acc1 = 1;
    volatile int acc2 = 2;
    
    for (int i = 1; i < n; i++) {
        /* Pattern: a[i] depends on a[i-1] (distance-1) */
        a[i] = a[i-1] * b[i] + c[i];  /* Distance-1 array dependency */
        
        /* Multiple accumulators with dependencies */
        acc1 = acc1 * 3 + a[i];        /* Distance-1 on acc1 */
        acc2 = acc2 ^ acc1;            /* Cross-dependency between accumulators */
        b[i] = acc2 + i;               /* Store result */
        
        /* Complex expression to create more scheduling opportunities */
        c[i] = (acc1 << 2) | (acc2 >> 1);
        
        /* Force dependency preservation */
        asm volatile("" : : "r"(acc1), "r"(acc2) : "memory");
    }
    return acc1 + acc2;
}

/* Function 3: Nested loops with inner loop carried dependency */
int test_nested_loops(int n, int m, int* matrix) {
    volatile int total = 0;
    
    /* Outer loop - unrolled manually to encourage modulo scheduling */
    for (int i = 0; i < n; i += 2) {
        int row_sum = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 0; j < m; j++) {
            /* Access pattern with stride-1 dependency */
            int idx = i * m + j;
            row_sum = row_sum + matrix[idx] * (j + 1);  /* Distance-1 in inner loop */
            matrix[idx] = row_sum & 0xFF;
            
            /* Additional operation to create more edges */
            if (j > 0) {
                matrix[idx] ^= matrix[idx - 1];  /* Another distance-1 dependency */
            }
        }
        
        total += row_sum;
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 4: Complex dependency chain with unknown trip count */
int test_complex_chain(int n, int* x, int* y, int* z) {
    if (n < 2) return 0;
    
    volatile int chain1 = x[0];
    volatile int chain2 = y[0];
    
    /* Loop with multiple interleaved distance-1 dependencies */
    for (int i = 1; i < n; i++) {
        /* Chain 1: x[i] depends on previous iteration */
        int temp1 = chain1 * 7 + z[i];
        chain1 = temp1 ^ y[i];           /* Distance-1 on chain1 */
        x[i] = chain1;
        
        /* Chain 2: y[i] depends on x[i-1] */
        int temp2 = chain2 + x[i-1];     /* Explicit distance-1 array access */
        chain2 = temp2 * 3 - i;          /* Distance-1 on chain2 */
        y[i] = chain2;
        
        /* Cross-dependency between chains */
        z[i] = (chain1 + chain2) * 5;
        
        /* Prevent optimization */
        asm volatile("" : : "r"(chain1), "r"(chain2) : "memory");
    }
    return chain1 + chain2 + x[n-1];
}

/* Function 5: Mixed operations with pointer chasing */
int test_mixed_ops(int n, int* arr1, int* arr2) {
    volatile int state = 0x1234;
    int* ptr = arr1;
    
    for (int i = 0; i < n; i++) {
        /* Pointer arithmetic with dependency */
        int val1 = *ptr;
        ptr = &arr1[(i + 1) % n];  /* Next iteration depends on current i */
        
        /* Multiple dependent operations */
        state = state * 1103515245 + 12345;  /* Linear congruential generator */
        int val2 = arr2[i] ^ state;
        
        /* Store with dependency on previous iteration */
        arr2[(i + 1) % n] = val1 + val2;  /* Distance-1 store */
        
        /* Complex expression tree */
        arr1[i] = (val1 * val2) + (state >> 16);
        
        asm volatile("" : : "r"(state), "r"(val1), "r"(val2) : "memory");
    }
    return state;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    /* Use command line or random size to prevent compile-time optimization */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate and initialize test arrays */
    int* data1 = (int*)malloc(n * sizeof(int));
    int* data2 = (int*)malloc(n * sizeof(int));
    int* data3 = (int*)malloc(n * sizeof(int));
    int* matrix = (int*)malloc(n * 10 * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
    }
    for (int i = 0; i < n * 10; i++) {
        matrix[i] = rand() % 256;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result = 0;
    result += test_accumulator(n, data1, 3);
    result += test_multiple_deps(n, data1, data2, data3);
    result += test_nested_loops(10, n/10, matrix);
    result += test_complex_chain(n, data1, data2, data3);
    result += test_mixed_ops(n, data1, data2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(matrix);
    
    return result != 0 ? 0 : 1;
}
