/* test-caller-save.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * caller-save.cc optimization pass. It creates scenarios where:
 * 1. Caller-save register spills are required
 * 2. Basic block end pointers need updating when inserting save/restore instructions
 * 3. Multiple live values must be preserved across function calls
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization of these functions */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))func; \
    fp(); \
} while(0)

/* Global volatile variables to prevent dead code elimination */
volatile int global_sink = 0;
volatile int global_counter = 0;

/* Non-inline functions that will be called */
NOINLINE void sink_func1(void) {
    /* Empty function - just a sink for calls */
    global_counter++;
}

NOINLINE void sink_func2(void) {
    /* Another empty function */
    global_counter ^= 1;
}

NOINLINE void sink_func3(int x) {
    /* Function with argument to prevent tail-call optimization */
    global_sink = x;
}

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int live1 = a * 3 + 1;
    int live2 = b ^ 0xABCDEF;
    int live3 = c & 0xFF00FF;
    int live4 = d | 0x123456;
    int live5 = e << 3;
    int live6 = f >> 2;
    int live7 = a + b + c;
    int live8 = d * e * f;
    int live9 = (a ^ b) & (c ^ d);
    int live10 = (e | f) ^ 0xDEADBEEF;
    
    /* Complex condition to create basic block structure */
    if (a > b && c < d) {
        /* More computations to increase register pressure */
        live1 = live1 * 2 + live2;
        live3 = live3 ^ live4;
        live5 = live5 | live6;
        live7 = live7 & live8;
        live9 = live9 + live10;
        
        /* Function call at the end of basic block before return */
        sink_func1();
        
        /* This return makes the call the BB_END before insertion */
        return live1 + live3 + live5 + live7 + live9;
    } else {
        /* Alternative path with different computations */
        live2 = live2 * 3 - live1;
        live4 = live4 ^ live3;
        live6 = live6 | live5;
        live8 = live8 & live7;
        live10 = live10 + live9;
        
        sink_func2();
        return live2 + live4 + live6 + live8 + live10;
    }
}

/* Test 2: Call in switch case with live values */
NOINLINE int test_call_in_switch_case(int selector, int x1, int x2, int x3, 
                                      int x4, int x5, int x6) {
    /* Create many live values */
    int val1 = x1 * x1 + 1;
    int val2 = x2 ^ x3;
    int val3 = x4 & x5;
    int val4 = x6 | 0x1234;
    int val5 = (x1 + x2) * (x3 - x4);
    int val6 = x5 ^ x6 ^ 0xCAFE;
    int val7 = (x1 & x2) | (x3 & x4);
    int val8 = x5 * x6 + x1;
    
    int result = 0;
    
    switch (selector & 0x3) {
        case 0:
            /* Multiple live values across call, then break */
            val1 = val1 * 2;
            val3 = val3 ^ val4;
            val5 = val5 + val6;
            sink_func3(val1);
            /* Call is at end of basic block before break */
            break;
            
        case 1:
            val2 = val2 | val3;
            val4 = val4 & val5;
            val6 = val6 ^ val7;
            sink_func1();
            break;
            
        case 2:
            val3 = val3 + val4;
            val5 = val5 * val6;
            val7 = val7 | val8;
            sink_func2();
            break;
            
        default:
            val4 = val4 ^ val5;
            val6 = val6 & val7;
            val8 = val8 + val1;
            VOLATILE_CALL(sink_func1);
            break;
    }
    
    /* Use all live values after the switch */
    result = val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
    return result;
}

/* Test 3: Call between complex operations with loop-generated values */
NOINLINE int test_call_between_complex_ops(int seed, int iterations) {
    /* Array of values that will be computed in loop */
    int values[16];
    int temp[8];
    
    /* Phase 1: Compute many values in a loop */
    for (int i = 0; i < iterations && i < 16; i++) {
        values[i] = seed * i + (i ^ 0x55);
        values[i] = values[i] * values[i] - i;
    }
    
    /* Intermediate computations creating more live values */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    for (int i = 0; i < 8 && i < iterations; i++) {
        temp[i] = values[i] ^ values[i+8];
        sum1 += temp[i];
        sum2 ^= values[i];
        sum3 |= values[i+8];
        sum4 = sum4 * 3 + temp[i];
    }
    
    /* Multiple independent computations to increase register pressure */
    int prod1 = sum1 * sum2;
    int prod2 = sum3 ^ sum4;
    int diff1 = sum1 - sum2;
    int diff2 = sum3 - sum4;
    int and_result = prod1 & prod2;
    int or_result = diff1 | diff2;
    int xor_result = and_result ^ or_result;
    int shift_result = (sum1 << 3) | (sum2 >> 2);
    
    /* Function call with many values live across it */
    sink_func3(prod1);
    
    /* Phase 2: Use all computed values after the call */
    int final_result = 0;
    for (int i = 0; i < iterations && i < 16; i++) {
        final_result += values[i] * (i + 1);
    }
    
    final_result += prod1 + prod2 + diff1 + diff2;
    final_result ^= and_result | or_result;
    final_result = (final_result * xor_result) ^ shift_result;
    
    return final_result;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int a, int b, int c, int d) {
    int v1 = a * b + c;
    int v2 = b * c + d;
    int v3 = c * d + a;
    int v4 = d * a + b;
    int v5 = (a ^ b) & (c ^ d);
    int v6 = (a | b) ^ (c | d);
    int v7 = v1 + v2 + v3;
    int v8 = v4 * v5 * v6;
    
    if (a > 0) {
        v1 = v1 * 2;
        v3 = v3 ^ v4;
        if (b > 0) {
            v5 = v5 | v6;
            v7 = v7 & v8;
            /* Call at end of inner basic block */
            sink_func1();
            /* Return makes this block end with call */
            return v1 + v3 + v5 + v7;
        } else {
            v2 = v2 * 3;
            v4 = v4 ^ v5;
            sink_func2();
            return v2 + v4 + v6 + v8;
        }
    } else {
        v6 = v6 << 2;
        v8 = v8 >> 1;
        if (c > 0) {
            v1 = v1 & v2;
            v3 = v3 | v4;
            VOLATILE_CALL(sink_func2);
            return v1 + v3 + v6 + v8;
        } else {
            v5 = v5 * v6;
            v7 = v7 ^ v8;
            sink_func3(v5);
            return v5 + v7;
        }
    }
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_at_bb_end(i*2, i*3, i*4, i*5, i*6, i*7);
    }
    
    /* Run test 2 with various selectors */
    for (int i = 0; i < 20; i++) {
        total += test_call_in_switch_case(i, i+10, i+20, i+30, 
                                         i+40, i+50, i+60);
    }
    
    /* Run test 3 with different seeds and iterations */
    for (int i = 1; i <= 8; i++) {
        total += test_call_between_complex_ops(i * 100, i * 2);
    }
    
    /* Run test 4 with various inputs */
    for (int i = -5; i <= 5; i++) {
        total += test_nested_conditionals(i, i+1, i+2, i+3);
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    /* Use result to prevent optimization */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}
