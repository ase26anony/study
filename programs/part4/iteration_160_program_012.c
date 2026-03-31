/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will force caller-save decisions */
__attribute__((noinline, noipa)) void non_inline_func1(void) {
    /* Empty function - just a call target */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) void non_inline_func2(int x) {
    /* Function with argument to prevent tail-call optimization */
    global_sink += x;
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) int non_inline_func3(int a, int b) {
    /* Function that returns a value */
    asm volatile("" : : : "memory");
    return a ^ b;
}

/* Test 1: Function call at the end of a basic block before return */
__attribute__((noinline, noipa)) int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across the call */
    int a = x * 3 + 1;
    int b = y / 2 - 5;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = (x + y) * (z - 1);
    int f = x | y | z;
    int g = ~x + ~y;
    int h = (x << 2) | (y >> 3);
    
    /* Function call placed at end of basic block before return */
    if (x > 0) {
        /* All these values must be saved/restored around the call */
        non_inline_func1();
        
        /* This return makes the call the BB_END before insertion */
        return a + b + c + d + e + f + g + h;
    } else {
        /* Alternative path to ensure both branches are compiled */
        int i = x * y * z;
        int j = (x + 1) * (y + 1) * (z + 1);
        non_inline_func2(i);
        return j - i;
    }
}

/* Test 2: Function call in a switch case that ends with break */
__attribute__((noinline, noipa)) int test_call_in_switch_case(int selector, 
                                                             int v1, int v2, 
                                                             int v3, int v4) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Create many live values across the call */
            int t1 = v1 * v2 + 123;
            int t2 = v3 ^ v4 ^ 0xABCD;
            int t3 = (v1 << 3) | (v2 >> 1);
            int t4 = v3 + v4 * 2;
            int t5 = ~v1 & ~v2;
            int t6 = v3 | v4 | 0xFF;
            
            /* Call at what could be BB_END before insertion */
            non_inline_func2(t1);
            
            /* Use all live values after call */
            result = t1 + t2 + t3 + t4 + t5 + t6;
            break;  /* Creates a basic block ending with the call */
        }
        
        case 1: {
            int u1 = v1 + v2;
            int u2 = v3 - v4;
            non_inline_func1();
            result = u1 * u2;
            break;
        }
            
        case 2: {
            /* More register pressure */
            int w1 = v1 * 2, w2 = v2 * 3, w3 = v3 * 4, w4 = v4 * 5;
            int w5 = w1 ^ w2, w6 = w3 & w4, w7 = w5 | w6;
            non_inline_func3(w1, w2);
            result = w3 + w4 + w5 + w6 + w7;
            break;
        }
            
        default:
            result = v1 + v2 + v3 + v4;
            non_inline_func1();
            break;
    }
    
    return result;
}

/* Test 3: Complex operations with loop before call */
__attribute__((noinline, noipa)) int test_call_between_complex_ops(int iterations, 
                                                                  int seed1, 
                                                                  int seed2) {
    /* Unroll a small loop to create many live values */
    int accum[8] = {0};
    
    /* Manual loop unrolling to increase register pressure */
    for (int i = 0; i < iterations && i < 8; i++) {
        accum[i] = seed1 * (i + 1) + seed2 * (8 - i);
    }
    
    /* Create many derived values that must live across the call */
    int sum1 = accum[0] + accum[1] + accum[2];
    int sum2 = accum[3] ^ accum[4] ^ accum[5];
    int sum3 = (accum[6] << 2) | (accum[7] >> 2);
    int prod1 = accum[0] * accum[1];
    int prod2 = accum[2] * accum[3];
    int diff1 = accum[4] - accum[5];
    int diff2 = accum[6] - accum[7];
    int xor1 = accum[0] ^ accum[7];
    int xor2 = accum[1] ^ accum[6];
    
    /* Volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = non_inline_func1;
    fp();
    
    /* Use all values after the call */
    return sum1 + sum2 + sum3 + prod1 + prod2 + diff1 + diff2 + xor1 + xor2;
}

/* Test 4: Nested condition with call at BB end */
__attribute__((noinline, noipa)) int test_nested_condition_bb_end(int a, int b, 
                                                                 int c, int d) {
    int result = 0;
    
    if (a > b) {
        if (c > d) {
            /* Create register pressure */
            int x1 = a * c + 100;
            int x2 = b * d - 50;
            int x3 = (a ^ c) & (b ^ d);
            int x4 = (a << c) | (b >> d);
            int x5 = ~a + ~b;
            int x6 = c * 3 + d * 7;
            int x7 = (a & b) | (c & d);
            int x8 = (a + b) ^ (c + d);
            
            /* Call at potential BB_END */
            non_inline_func2(x1);
            
            /* Return immediately after call */
            return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
        } else {
            int y1 = a + b + c + d;
            non_inline_func1();
            result = y1 * 2;
        }
    } else {
        int z1 = a - b - c - d;
        non_inline_func3(z1, a);
        result = z1;
    }
    
    return result;
}

/* Test 5: Multiple calls in same function with overlapping live ranges */
__attribute__((noinline, noipa)) int test_multiple_calls_live_ranges(int p1, int p2,
                                                                    int p3, int p4) {
    /* Values that must survive across multiple calls */
    int live1 = p1 * p2 + 111;
    int live2 = p3 ^ p4 ^ 0xDEAD;
    int live3 = (p1 << p2) | (p3 >> 1);
    int live4 = ~p1 & ~p2 & ~p3;
    int live5 = p4 * 3 + p1 * 7;
    int live6 = (p2 + p3) * (p4 - p1);
    
    /* First call - some values might be spilled/restored */
    non_inline_func2(live1);
    
    /* Intermediate use keeps values live */
    int mid1 = live2 + live3;
    int mid2 = live4 ^ live5;
    
    /* More values created between calls */
    int live7 = live6 * 2 + mid1;
    int live8 = live1 ^ live2 ^ mid2;
    int live9 = (live3 << 2) | (live4 >> 2);
    int live10 = live5 + live6 + live7;
    
    /* Second call - different register pressure */
    non_inline_func3(live7, live8);
    
    /* Final computation uses all values */
    return live1 + live2 + live3 + live4 + live5 + 
           live6 + live7 + live8 + live9 + live10 + mid1 + mid2;
}

int main(void) {
    int total = 0;
    
    /* Seed values to ensure varied execution paths */
    int seed = 12345;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run each test multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int val1 = seed + i * 17;
        int val2 = seed - i * 23;
        int val3 = seed ^ (i * 31);
        int val4 = seed | (i * 47);
        
        total += test_call_at_bb_end(val1, val2, val3);
        total += test_call_in_switch_case(i, val1, val2, val3, val4);
        total += test_call_between_complex_ops(8, val1, val2);
        total += test_nested_condition_bb_end(val1, val2, val3, val4);
        total += test_multiple_calls_live_ranges(val1, val2, val3, val4);
        
        /* Modify seed to explore different paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be consistent across runs)\n");
    
    /* Store in volatile global to ensure all computations are used */
    global_sink = total;
    
    return 0;
}
