/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile int g_volatile_array[100];

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int n, int* arr) {
    int i, j;
    volatile int local_vol = 0;
    
    /* Outer loop with data dependency */
    for (i = 0; i < n; i++) {
        /* Inner loop with conditional */
        for (j = 0; j < 10; j++) {
            /* Create data dependency chain */
            local_vol = arr[i] + j;
            
            /* Conditional store with side effect */
            if (local_vol > 100) {
                arr[i] = local_vol - 50;
                /* Inline asm to create scheduling barrier */
                asm volatile ("" : : "r"(local_vol) : "memory");
            } else {
                arr[i] = local_vol + 25;
            }
            
            /* Another asm to create unschedulable dependency */
            asm volatile ("# dependency barrier %0" : : "r"(arr[i]));
        }
        
        /* Function call to create control flow complexity */
        g_volatile_counter += i;
    }
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int rows, int cols, int matrix[][10]) {
    int i, j, k;
    int sum = 0;
    volatile int temp;
    
    /* Triple nested loop for scheduling complexity */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            /* Data dependency across iterations */
            temp = matrix[i][j];
            
            for (k = 0; k < 5; k++) {
                /* Complex arithmetic with dependencies */
                temp = temp * 3 + k;
                
                /* Conditional with unpredictable branch */
                if ((temp & 1) == 0) {
                    sum += temp;
                    /* Memory barrier */
                    asm volatile ("" : : : "memory");
                } else {
                    sum -= temp / 2;
                }
                
                /* Another scheduling constraint */
                g_volatile_array[k] = temp;
            }
            
            matrix[i][j] = temp;
        }
    }
    
    return sum;
}

/* Function 3: Switch statement with computed goto-like behavior */
int test_switch_complex(int mode, int iterations) {
    int i, result = 0;
    volatile int state = 0;
    
    /* Loop with switch inside */
    for (i = 0; i < iterations; i++) {
        /* Complex switch with multiple cases */
        switch (mode) {
            case 0:
                result += i * 2;
                /* Inline asm for RTL generation */
                asm volatile ("# case 0 computation %0" : "+r"(result));
                state = result % 256;
                break;
                
            case 1:
                result -= i * 3;
                asm volatile ("# case 1 computation %0" : "+r"(result));
                state = (result + i) & 0xFF;
                break;
                
            case 2:
                result ^= i;
                asm volatile ("# case 2 computation %0" : "+r"(result));
                state = result | 0x80;
                break;
                
            case 3:
                result = (result << 3) | (i & 7);
                asm volatile ("# case 3 computation %0" : "+r"(result));
                state = ~result;
                break;
                
            default:
                result = result * 11 + i;
                asm volatile ("# default computation %0" : "+r"(result));
                state = 0;
                break;
        }
        
        /* Memory operation with dependency */
        g_volatile_array[state % 100] = result;
        
        /* Change mode to create control flow variability */
        mode = (mode + 1) % 5;
    }
    
    return result;
}

/* Function 4: Mixed control flow with pointer chasing */
int test_mixed_control_flow(int* data, int size) {
    int i, *ptr = data;
    volatile int accumulator = 0;
    
    /* Loop with pointer arithmetic and conditionals */
    for (i = 0; i < size; i++) {
        /* Unpredictable branch */
        if (ptr < data + size - 1) {
            /* Pointer chasing with dependency */
            int diff = *ptr - *(ptr + 1);
            accumulator += diff;
            
            /* Inline asm for scheduling complexity */
            asm volatile ("# pointer diff %0" : : "r"(diff));
            
            ptr += (diff > 0) ? 1 : 2;
        } else {
            accumulator -= *ptr;
            ptr = data;
        }
        
        /* Another conditional with side effect */
        if (accumulator > 1000) {
            accumulator = accumulator % 1000;
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Store to volatile to prevent dead code elimination */
        g_volatile_counter = accumulator;
    }
    
    return accumulator;
}

/* Function 5: Recursive-like pattern using loop */
int test_pseudo_recursive(int n, int depth) {
    int i, j;
    volatile int stack[10];
    int result = 0;
    
    /* Simulate recursive pattern with manual stack */
    for (i = 0; i < depth; i++) {
        stack[i] = n + i;
        
        /* Inner computation loop */
        for (j = 0; j < n; j++) {
            /* Data-dependent computation */
            int val = stack[i] * j;
            
            /* Conditional with multiple paths */
            if (val % 3 == 0) {
                result += val;
                asm volatile ("# mod 3 path %0" : : "r"(val));
            } else if (val % 3 == 1) {
                result -= val / 2;
                asm volatile ("# mod 1 path %0" : : "r"(val));
            } else {
                result ^= val;
                asm volatile ("# mod 2 path %0" : : "r"(val));
            }
            
            /* Update stack with dependency */
            stack[i] = (stack[i] + result) & 0xFF;
        }
        
        /* Store to volatile array */
        g_volatile_array[i % 100] = stack[i];
    }
    
    return result;
}

/* Main driver function */
int main() {
    int array1[50];
    int matrix[5][10];
    int data_array[100];
    int i, result = 0;
    
    /* Initialize arrays */
    for (i = 0; i < 50; i++) {
        array1[i] = i * 3 + 7;
    }
    
    for (i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    for (i = 0; i < 100; i++) {
        data_array[i] = (i * 17) % 123;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(20, array1);
    result += test_nested_loops(5, 10, matrix);
    result += test_switch_complex(0, 25);
    result += test_mixed_control_flow(data_array, 100);
    result += test_pseudo_recursive(8, 5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
