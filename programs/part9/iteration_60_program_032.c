/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int temp = arr[i];
        
        /* Conditional store with dependency chain */
        if (temp > 100) {
            arr[i] = temp * 2 + i;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
        } else {
            arr[i] = temp / 2 - i;
        }
        
        /* Nested loop for more scheduling complexity */
        for (j = 0; j < 5; j++) {
            sum += arr[i] + j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use the result to prevent optimization */
    if (sum > 1000) {
        printf("Sum: %d\n", sum);
    }
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int size) {
    int i, j, k;
    volatile int result = 0;
    int *matrix = (int*)malloc(size * size * sizeof(int));
    
    if (!matrix) return -1;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i * size + j] = i * j + (i ^ j);
        }
    }
    
    /* Complex nested loop with data dependencies */
    for (i = 1; i < size - 1; i++) {
        for (j = 1; j < size - 1; j++) {
            int sum = 0;
            for (k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    sum += matrix[(i + k) * size + (j + l)];
                    /* Create dependency chain */
                    asm volatile ("" : "+r"(sum) : : "memory");
                }
            }
            matrix[i * size + j] = sum / 9;
            result += matrix[i * size + j];
        }
    }
    
    free(matrix);
    return result;
}

/* Function 3: Switch statement with computed goto for complex CFG */
void test_switch_complex(int mode, int iterations) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    volatile int counter = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int selector = (mode + i) % 5;
        
        /* Computed goto creates complex control flow */
        goto *labels[selector];
        
    case0:
        counter += i * 2;
        /* Memory operation with barrier */
        asm volatile ("" : : "r"(counter) : "memory");
        continue;
        
    case1:
        counter -= i / 2;
        /* Create artificial dependency */
        asm volatile ("nop" : : : "cc");
        continue;
        
    case2:
        counter = counter * 3 + 1;
        /* Force spill/reload */
        asm volatile ("" : "+r"(counter) : : );
        continue;
        
    case3:
        counter = ~counter;
        /* Complex operation */
        asm volatile ("" : : "r"(i), "r"(counter) : );
        continue;
        
    default_case:
        counter = (counter << 2) | (counter >> 30);
        /* Serializing operation */
        asm volatile ("" : : : "memory");
        continue;
    }
    
    /* Use result */
    if (counter > 100) {
        printf("Counter: %d\n", counter);
    }
}

/* Function 4: Mixed operations with pointer aliasing */
int test_mixed_ops(int *a, int *b, int n) {
    int i;
    volatile int acc = 0;
    
    /* Create potential pointer aliasing */
    int *ptr1 = a;
    int *ptr2 = b;
    
    for (i = 0; i < n; i++) {
        /* Memory operations with possible aliasing */
        *ptr1 = *ptr2 + i;
        
        /* Arithmetic with dependency */
        int t1 = *ptr1 * 3;
        int t2 = t1 / (i + 1);
        int t3 = t2 ^ t1;
        
        /* Conditional update */
        if (t3 & 1) {
            *ptr2 = t3;
            asm volatile ("" : : "r"(t3) : "memory");
        } else {
            *ptr2 = ~t3;
        }
        
        acc += *ptr1 + *ptr2;
        
        /* Pointer arithmetic */
        ptr1++;
        ptr2++;
        
        /* Barrier every few iterations */
        if (i % 8 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return acc;
}

/* Function 5: Recursive-like pattern with loops */
void test_recursive_pattern(int depth, int width) {
    int i, j;
    volatile int stack[32];
    int top = 0;
    
    for (i = 0; i < depth; i++) {
        /* Push state */
        stack[top++] = i;
        
        /* Inner processing */
        for (j = 0; j < width; j++) {
            int val = stack[top - 1] * j;
            
            /* Complex conditional */
            if ((val & 3) == 0) {
                stack[top - 1] = val + j;
                asm volatile ("" : "+r"(val) : : );
            } else if ((val & 3) == 1) {
                stack[top - 1] = val - j;
                /* Force register pressure */
                asm volatile ("" : : "r"(i), "r"(j), "r"(val) : );
            } else {
                stack[top - 1] = val ^ j;
            }
            
            /* Periodic memory sync */
            if (j % 4 == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        /* Pop if condition met */
        if (stack[top - 1] > 100) {
            top--;
        }
    }
}

/* Main driver */
int main(int argc, char **argv) {
    int test_array[100];
    int i, result;
    
    /* Initialize with random-ish data */
    srand(time(NULL));
    for (i = 0; i < 100; i++) {
        test_array[i] = rand() % 200;
    }
    
    /* Call all test functions to ensure compilation */
    test_inner_loop(test_array, 100);
    
    result = test_nested_loops(20);
    if (result < 0) {
        fprintf(stderr, "Memory allocation failed\n");
    }
    
    test_switch_complex(2, 50);
    
    int test_array2[50];
    for (i = 0; i < 50; i++) {
        test_array2[i] = i * 3;
    }
    result = test_mixed_ops(test_array, test_array2, 50);
    
    test_recursive_pattern(10, 15);
    
    /* Use results to prevent dead code elimination */
    printf("Final check: %d\n", test_array[0] + result);
    
    return 0;
}
