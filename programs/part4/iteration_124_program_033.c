/* Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inline helper functions to generate call RTL */
static int __attribute__((noinline)) helper_mul(int a, int b) {
    return a * b;
}

static float __attribute__((noinline)) helper_fmul(float a, float b) {
    return a * b;
}

/* Function with complex loop body to create ILP opportunities */
__attribute__((noinline, optimize("O3")))
int test_complex_loop(int* arr, int n, float* farr) {
    int int_sum = 0;
    float float_sum = 0.0f;
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 0; i < n; i++) {
        /* Mixed integer and floating point operations */
        int val = arr[i];
        float fval = farr[i];
        
        /* Data-dependent computations creating ILP */
        int product = val * i;
        int_sum += product;
        
        float fproduct = fval * (float)i;
        float_sum += fproduct;
        
        /* Conditional move/ternary operation */
        int max_val = (val > int_sum) ? val : int_sum;
        
        /* Built-in function for complex RTL */
        int popcnt = __builtin_popcount(val);
        
        /* Memory access pattern */
        arr[i] = max_val + popcnt;
        
        /* Inline assembly as scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Call to non-inline functions */
        int_sum += helper_mul(val, i);
        float_sum += helper_fmul(fval, (float)i);
        
        /* Branch to create control flow */
        if (i % 3 == 0) {
            int_sum -= popcnt;
            barrier = int_sum; /* Use volatile to prevent dead code */
        } else if (i % 5 == 0) {
            float_sum *= 1.01f;
        }
        
        /* More mixed operations */
        double dval = (double)val * 0.5;
        int_sum += (int)dval;
    }
    
    return int_sum + (int)float_sum;
}

/* Function with nested loops for outer loop pipelining */
__attribute__((noinline, optimize("O3")))
int test_nested_loops(int size) {
    int total = 0;
    int* matrix = (int*)malloc(size * size * sizeof(int));
    
    if (!matrix) return 0;
    
    /* Initialize */
    for (int i = 0; i < size * size; i++) {
        matrix[i] = i % 256;
    }
    
    /* Nested loops for outer-loop pipelining */
    for (int i = 0; i < size; i++) {
        int row_sum = 0;
        for (int j = 0; j < size; j++) {
            int idx = i * size + j;
            int val = matrix[idx];
            
            /* Complex expression with multiple dependencies */
            row_sum += val * (i + j) - (i * j);
            
            /* Conditional store */
            matrix[idx] = (val > 128) ? val - 128 : val + 128;
            
            /* Another scheduling barrier */
            asm volatile ("" : : : "memory");
        }
        total += row_sum;
        
        /* Branch with side effect */
        if (i % 7 == 0) {
            total = __builtin_bswap32(total); /* Generate interesting RTL */
        }
    }
    
    free(matrix);
    return total;
}

/* Function with pointer chasing to create memory dependencies */
__attribute__((noinline, optimize("O3")))
int test_linked_list(int iterations) {
    struct Node {
        int value;
        struct Node* next;
        float fvalue;
    };
    
    /* Create a small linked list */
    struct Node nodes[10];
    for (int i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].fvalue = (float)i * 1.5f;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[9].value = 90;
    nodes[9].fvalue = 13.5f;
    nodes[9].next = &nodes[0]; /* Circular */
    
    struct Node* current = &nodes[0];
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < iterations; i++) {
        /* Pointer chasing creates memory dependencies */
        sum += current->value;
        fsum += current->fvalue;
        
        /* Mixed computation */
        int temp = current->value * i;
        sum += (temp > 1000) ? temp / 2 : temp * 2;
        
        /* Floating point operation */
        fsum = fsum * 1.001f + (float)current->value;
        
        current = current->next;
        
        /* Periodic barrier */
        if (i % 4 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return sum + (int)fsum;
}

/* Main driver that calls all test functions */
int main(int argc, char** argv) {
    int n = 100;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 100;
    }
    
    /* Allocate and initialize arrays */
    int* int_arr = (int*)malloc(n * sizeof(int));
    float* float_arr = (float*)malloc(n * sizeof(float));
    
    if (!int_arr || !float_arr) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < n; i++) {
        int_arr[i] = (i * 37) % 7919; /* Prime modulus for variety */
        float_arr[i] = (float)i * 0.73f;
    }
    
    /* Run test functions */
    int result1 = test_complex_loop(int_arr, n, float_arr);
    printf("Test 1 result: %d\n", result1);
    
    int result2 = test_nested_loops(50);
    printf("Test 2 result: %d\n", result2);
    
    int result3 = test_linked_list(n * 2);
    printf("Test 3 result: %d\n", result3);
    
    /* Final computation using all results */
    int final_result = result1 + result2 + result3;
    printf("Final result: %d\n", final_result);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    
    return 0;
}
