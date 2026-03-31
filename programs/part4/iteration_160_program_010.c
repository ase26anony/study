/* test-caller-save.c
 * This program is designed to trigger the uncovered lines in GCC's caller-save.cc
 * Specifically, it forces insertion of save/restore instructions at call sites
 * where registers need to be preserved across function calls.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and inter-procedural optimization */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile function pointers to prevent optimization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_func_ptr;

/* Global volatile variables to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Function call at the end of a basic block before return
 * This should create a basic block ending with a call instruction
 */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must be preserved across the call */
    int live1 = a * 2 + 1;
    int live2 = b * 3 - 2;
    int live3 = c ^ 0x55AA55AA;
    int live4 = d | 0x12345678;
    int live5 = e & 0xF0F0F0F0;
    int live6 = f + 100;
    
    /* More computations to increase register pressure */
    int live7 = live1 * live2;
    int live8 = live3 + live4;
    int live9 = live5 - live6;
    int live10 = live7 ^ live8;
    
    /* Use volatile function pointer to ensure call isn't optimized away */
    volatile_func_ptr = func1;
    volatile_func_ptr();  /* Call at potential BB end */
    
    /* Use all live values after the call - forces them to be preserved */
    return live1 + live2 + live3 + live4 + live5 + live6 + 
           live7 + live8 + live9 + live10;
}

/* Test 2: Function call in a switch case that ends with break
 * Creates a basic block ending with call then break
 */
NOINLINE int test_call_in_switch_case(int selector, int x1, int x2, int x3, 
                                      int x4, int x5, int x6) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Many live values across the call */
            int v1 = x1 * 2;
            int v2 = x2 + x3;
            int v3 = x4 ^ x5;
            int v4 = x6 << 2;
            int v5 = v1 & v2;
            int v6 = v3 | v4;
            
            volatile_func_ptr = func2;
            volatile_func_ptr();  /* Call before break - end of BB */
            
            result = v1 + v2 + v3 + v4 + v5 + v6;
            break;  /* Basic block ends with call, then break */
        }
        
        case 1: {
            /* Different computation pattern */
            int v1 = x1 + x2;
            int v2 = x3 - x4;
            int v3 = x5 * x6;
            int v4 = v1 ^ v2;
            int v5 = v3 & 0xFF;
            
            volatile_func_ptr = func3;
            volatile_func_ptr();
            
            result = v1 * v2 + v3 - v4 + v5;
            break;
        }
            
        default: {
            /* Even more register pressure */
            int v1 = x1;
            int v2 = x2;
            int v3 = x3;
            int v4 = x4;
            int v5 = x5;
            int v6 = x6;
            int v7 = v1 + v2;
            int v8 = v3 + v4;
            int v9 = v5 + v6;
            int v10 = v7 * v8;
            
            volatile_func_ptr = func4;
            volatile_func_ptr();
            
            result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
            break;
        }
    }
    
    return result;
}

/* Test 3: Complex loop before call, then use all values after call
 * Maximizes register pressure around the call site
 */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    int i;
    /* Array of values that will be computed in loop and used after call */
    int values[12];
    
    /* Compute many different values in a loop */
    for (i = 0; i < 12; i++) {
        values[i] = seed + i * 3;
        /* Do some computation to make values non-trivial */
        values[i] = (values[i] * 13) ^ 0x123456;
        values[i] = values[i] + (i << 4);
    }
    
    /* Additional intermediate computations */
    int sum1 = values[0] + values[1] + values[2];
    int sum2 = values[3] * values[4] - values[5];
    int sum3 = values[6] ^ values[7] | values[8];
    int sum4 = values[9] & values[10] + values[11];
    
    /* More computations to increase register pressure */
    int prod1 = sum1 * sum2;
    int prod2 = sum3 * sum4;
    int xor1 = sum1 ^ sum3;
    int xor2 = sum2 ^ sum4;
    
    /* Call with many live values */
    volatile_func_ptr = func1;
    volatile_func_ptr();
    
    /* Use ALL computed values after the call */
    int result = 0;
    for (i = 0; i < 12; i++) {
        result += values[i];
    }
    
    result += sum1 + sum2 + sum3 + sum4;
    result += prod1 + prod2;
    result += xor1 ^ xor2;
    
    return result;
}

/* Test 4: Nested conditionals with calls at BB ends */
NOINLINE int test_nested_conditionals(int a, int b, int c, int d, int e, int f) {
    int result = 0;
    
    if (a > 0) {
        int v1 = a * 2;
        int v2 = b + c;
        int v3 = d ^ e;
        int v4 = f << 1;
        
        if (b > 0) {
            int v5 = v1 + v2;
            int v6 = v3 * v4;
            int v7 = v5 ^ v6;
            
            volatile_func_ptr = func2;
            volatile_func_ptr();  /* Call at BB end before return */
            
            return v5 + v6 + v7;  /* Return immediately after call */
        } else {
            int v5 = v1 - v2;
            int v6 = v3 | v4;
            
            volatile_func_ptr = func3;
            volatile_func_ptr();  /* Another BB ending with call */
            
            result = v5 * v6;
        }
        
        /* Use remaining values */
        result += v1 + v2 + v3 + v4;
    } else {
        int v1 = a + b;
        int v2 = c * d;
        int v3 = e ^ f;
        int v4 = v1 & v2;
        int v5 = v3 | v4;
        
        volatile_func_ptr = func4;
        volatile_func_ptr();
        
        result = v1 + v2 + v3 + v4 + v5;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    int i;
    
    /* Initialize volatile function pointer */
    volatile_func_ptr = func1;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
    }
    
    /* Run test 2 with different selectors */
    for (i = 0; i < 8; i++) {
        total += test_call_in_switch_case(i, i*2, i*3, i*4, i*5, i*6, i*7);
    }
    
    /* Run test 3 with different seeds */
    for (i = 1; i <= 5; i++) {
        total += test_call_between_complex_ops(3, i*10);
    }
    
    /* Run test 4 with various inputs */
    for (i = 0; i < 6; i++) {
        total += test_nested_conditionals(i, i-1, i+1, i*2, i*3, i*4);
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be non-zero and deterministic)\n");
    
    return total != 0 ? 0 : 1;
}
