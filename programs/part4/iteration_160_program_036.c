/* caller_save_test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets the instruction insertion logic at lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))func; \
    fp(); \
} while(0)

/* Global volatile variables to prevent dead code elimination */
volatile int global_sink = 0;
volatile int global_counter = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(void) { global_counter += 2; }
NOINLINE void func3(void) { global_counter += 3; }
NOINLINE void func4(void) { global_counter += 4; }

/* Test 1: Function call at the end of a basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live values that must survive across the call */
    int live1 = a * b + c;
    int live2 = d ^ e ^ f;
    int live3 = g & h;
    int live4 = a + b + c + d;
    int live5 = e * f * g;
    int live6 = h << 2;
    int live7 = (a + b) * (c + d);
    int live8 = (e | f) & (g | h);
    
    /* Complex computation to increase register pressure */
    int temp1 = live1 * live2;
    int temp2 = live3 + live4;
    int temp3 = live5 ^ live6;
    int temp4 = live7 & live8;
    
    /* Call at what could be BB_END if this is the last instruction before return */
    if (temp1 > temp2) {
        /* Multiple live values across the call */
        int sum_before = live1 + live2 + live3 + live4;
        VOLATILE_CALL(func1);  /* Non-inline call with many live registers */
        
        /* Use all live values after the call - forces caller-save */
        int result = sum_before + live5 + live6 + live7 + live8;
        return result;  /* Call was at BB_END, inserted save/restore becomes new BB_END */
    } else {
        /* Alternative path also with call at BB_END */
        int diff_before = live5 - live6 - live7;
        VOLATILE_CALL(func2);
        return diff_before + live8;  /* Call at BB_END */
    }
}

/* Test 2: Function call in switch case with break */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int v1 = x * x;
    int v2 = y * y;
    int v3 = z * z;
    int v4 = x + y + z;
    int v5 = x ^ y ^ z;
    int v6 = (x << 3) | (y << 2) | (z << 1);
    int v7 = ~x & ~y & ~z;
    int v8 = x * y * z;
    
    switch (x & 0x3) {
        case 0:
            /* Multiple live values, then call, then break */
            int sum = v1 + v2 + v3;
            VOLATILE_CALL(func1);  /* Call in middle of basic block */
            result = sum + v4;  /* Use live values after call */
            break;  /* Creates BB_END at the call if not already */
            
        case 1:
            /* Different pattern with call at potential BB_END */
            int prod = v5 * v6;
            VOLATILE_CALL(func2);
            result = prod + v7;  /* Call was at BB_END before break */
            break;
            
        case 2:
            /* Even more live values */
            int t1 = v1 + v3 + v5;
            int t2 = v2 + v4 + v6;
            VOLATILE_CALL(func3);
            result = t1 * t2 + v7 + v8;
            break;
            
        default:
            /* Call as last instruction before return from switch */
            int mix = (v1 & v2) | (v3 & v4) | (v5 & v6);
            VOLATILE_CALL(func4);
            result = mix ^ v7 ^ v8;
            break;
    }
    
    return result;
}

/* Test 3: Complex loop with many live values across call */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    /* Unrolled loop to create many live values */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (int i = 0; i < iterations && i < 10; i++) {
        /* Many independent computations creating register pressure */
        int t1 = a * i + b;
        int t2 = c ^ i ^ d;
        int t3 = e & i & f;
        int t4 = g | i | h;
        int t5 = a + b + c + i;
        int t6 = d * e * f * i;
        int t7 = (g << i) & h;
        int t8 = ~a & ~b & i;
        
        /* Update accumulators - all t values must be kept live */
        acc1 += t1 + t2;
        acc2 += t3 + t4;
        acc3 += t5 + t6;
        acc4 += t7 + t8;
        
        /* Rotate values to create data dependencies */
        a = b; b = c; c = d; d = e;
        e = f; f = g; g = h; h = t1 & 0xFF;
    }
    
    /* Non-inline call with many live values */
    VOLATILE_CALL(func1);
    
    /* Use all accumulators after the call */
    return acc1 + acc2 * 2 + acc3 * 3 + acc4 * 4;
}

/* Test 4: Nested conditionals with calls at BB_END */
NOINLINE int test_nested_conditionals(int x, int y, int z) {
    /* Create register pressure */
    int r1 = x * y;
    int r2 = y * z;
    int r3 = z * x;
    int r4 = x + y + z;
    int r5 = x ^ y ^ z;
    int r6 = (x << y) | z;
    int r7 = ~x & y & ~z;
    int r8 = x * 2 + y * 3 + z * 4;
    
    if (x > 0) {
        if (y > 0) {
            int sum = r1 + r2 + r3;
            VOLATILE_CALL(func1);  /* Call in nested block */
            if (z > 0) {
                /* Call at BB_END before return */
                int prod = r4 * r5;
                VOLATILE_CALL(func2);
                return prod + sum;  /* BB_END was at call */
            } else {
                /* Another call at BB_END */
                int diff = r6 - r7;
                VOLATILE_CALL(func3);
                return diff + r8;  /* BB_END was at call */
            }
        } else {
            /* Call as last instruction in this branch */
            int mix = (r1 & r2) | (r3 & r4);
            VOLATILE_CALL(func4);
            return mix;  /* BB_END was at call */
        }
    }
    
    /* Default path also with call */
    VOLATILE_CALL(func1);
    return r5 + r6 + r7 + r8;
}

/* Test 5: Multiple consecutive calls with live values */
NOINLINE int test_multiple_calls(int a, int b, int c) {
    /* Many live values that must survive across multiple calls */
    int v1 = a + b;
    int v2 = b + c;
    int v3 = c + a;
    int v4 = a * b;
    int v5 = b * c;
    int v6 = c * a;
    int v7 = a ^ b ^ c;
    int v8 = ~a & ~b & ~c;
    
    /* First call - some values might be spilled/restored */
    VOLATILE_CALL(func1);
    
    /* Use values, then create more */
    int t1 = v1 + v2 + v3;
    int t2 = v4 * v5 * v6;
    
    /* Second call - different register pressure */
    VOLATILE_CALL(func2);
    
    /* More computations */
    int t3 = v7 | v8;
    int t4 = (v1 << 2) & (v2 << 1);
    
    /* Third call - potentially at BB_END if followed by return */
    VOLATILE_CALL(func3);
    
    return t1 + t2 + t3 + t4;
}

int main(void) {
    int total = 0;
    
    /* Seed for reproducible but varied inputs */
    int seed = 42;
    
    printf("Testing caller-save optimization paths...\n");
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 5; i++) {
        total += test_call_at_bb_end(
            seed + i, seed + i + 1, seed + i + 2,
            seed + i + 3, seed + i + 4, seed + i + 5,
            seed + i + 6, seed + i + 7
        );
        
        total += test_call_in_switch_case(
            seed + i * 2, seed + i * 3, seed + i * 5
        );
        
        total += test_call_between_complex_ops(
            3 + (i % 3), seed + i * 7
        );
        
        total += test_nested_conditionals(
            seed - i, seed + i * 11, seed + i * 13
        );
        
        total += test_multiple_calls(
            seed + i * 17, seed + i * 19, seed + i * 23
        );
        
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Store result to volatile to prevent optimization */
    global_sink = total;
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return (total > 0) ? 0 : 1;
}
