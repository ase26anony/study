/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* Prevent optimization */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency */
        int temp = arr[i];
        
        /* Conditional store with side effect */
        if (temp > 100) {
            arr[i] = temp / 2;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
        } else {
            arr[i] = temp * 2;
        }
        
        /* Complex loop body with multiple operations */
        for (j = 0; j < 5; j++) {
            sum += arr[i] + j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    if (sum > 1000) {
        printf("Inner loop result: %d\n", sum);
    }
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int size) {
    int i, j, k;
    volatile int counter = 0;
    int *matrix = (int*)malloc(size * size * sizeof(int));
    
    if (!matrix) return -1;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i * size + j] = i * j + 1;
        }
    }
    
    /* Complex nested loop with data dependencies */
    for (i = 1; i < size - 1; i++) {
        for (j = 1; j < size - 1; j++) {
            int val = 0;
            /* Inner computation loop */
            for (k = -1; k <= 1; k++) {
                val += matrix[(i + k) * size + j] +
                       matrix[i * size + (j + k)];
            }
            /* Conditional update with side effect */
            if (val % 3 == 0) {
                matrix[i * size + j] = val / 3;
                counter++;
            } else {
                matrix[i * size + j] = val;
                /* Force dependency chain */
                asm volatile ("" : : "r"(val) : "memory");
            }
        }
    }
    
    /* Compute final result */
    int result = 0;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            result ^= matrix[i * size + j];
        }
    }
    
    free(matrix);
    return result + counter;
}

/* Function 3: Switch statement with computed goto for complex CFG */
void test_switch_complex(int mode, int iterations) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    volatile int state = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Computed goto creates complex control flow */
        goto *labels[mode % 4];
        
    label0:
        state += i * 2;
        /* Memory operation */
        asm volatile ("" : : "r"(state) : "memory");
        mode = (mode + 1) % 4;
        continue;
        
    label1:
        state -= i / 2;
        /* Conditional jump */
        if (state > 100) {
            mode = 2;
        } else {
            mode = 3;
        }
        continue;
        
    label2:
        state *= 3;
        /* Function call simulation */
        asm volatile ("nop" : : : "memory");
        mode = 0;
        continue;
        
    label3:
        state = state ^ 0x55AA;
        /* Complex expression */
        state = (state << 3) | (state >> 5);
        mode = 1;
        continue;
    }
    
    /* Use result */
    if (state != 0) {
        printf("Switch state: %d\n", state);
    }
}

/* Function 4: Mixed operations with pointer aliasing */
int test_mixed_ops(int *a, int *b, int n) {
    int i;
    volatile int acc = 0;
    
    /* Create pointer aliasing possibility */
    int *ptr1 = a;
    int *ptr2 = b;
    
    if (n > 10) {
        ptr2 = a + 1;  /* Force aliasing */
    }
    
    /* Loop with mixed operations */
    for (i = 0; i < n; i++) {
        /* Load operations */
        int x = ptr1[i];
        int y = ptr2[i % n];
        
        /* Arithmetic with dependency */
        int z = x * y + i;
        
        /* Conditional store */
        if (z % 7 == 0) {
            ptr1[i] = z;
            acc += z;
        } else {
            ptr2[i % n] = z;
            acc -= z;
        }
        
        /* Barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
    
    return acc;
}

/* Function 5: Recursive-like pattern with tail operations */
int test_tail_operations(int seed, int depth) {
    volatile int result = seed;
    int i;
    
    for (i = 0; i < depth; i++) {
        /* Multiple independent operations */
        int a = result + i;
        int b = result - i;
        int c = result * i;
        
        /* Conditional selection */
        if (i % 2 == 0) {
            result = a ^ b;
            /* Create specific RTL pattern */
            asm volatile ("add %0, %1, %2" : "=r"(result) : "r"(a), "r"(b));
        } else {
            result = b | c;
            asm volatile ("or %0, %1, %2" : "=r"(result) : "r"(b), "r"(c));
        }
        
        /* Loop-carried dependency */
        result = (result << 1) | (result >> 31);
    }
    
    return result;
}

/* Main driver to ensure all code paths are compiled */
int main(int argc, char **argv) {
    int test_size = 50;
    int *array1 = (int*)malloc(test_size * sizeof(int));
    int *array2 = (int*)malloc(test_size * sizeof(int));
    
    if (!array1 || !array2) {
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < test_size; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 7 - 2;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(array1, test_size);
    
    int result1 = test_nested_loops(10);
    printf("Nested loops result: %d\n", result1);
    
    test_switch_complex(argc, 20);
    
    int result2 = test_mixed_ops(array1, array2, test_size);
    printf("Mixed ops result: %d\n", result2);
    
    int result3 = test_tail_operations(42, 15);
    printf("Tail ops result: %d\n", result3);
    
    /* Cleanup */
    free(array1);
    free(array2);
    
    return 0;
}
