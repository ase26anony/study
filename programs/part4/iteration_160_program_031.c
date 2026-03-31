/* test-caller-save.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noclone))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))func; \
    fp(); \
} while(0)

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void sink_func1(void) { global_sink = 1; }
NOINLINE void sink_func2(void) { global_sink = 2; }
NOINLINE void sink_func3(void) { global_sink = 3; }
NOINLINE void sink_func4(void) { global_sink = 4; }

/* Test 1: Call at basic block end before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live values that must survive across call */
    int live1 = a * b + c;
    int live2 = d ^ e | f;
    int live3 = g << 2;
    int live4 = h >> 1;
    int live5 = a + b + c + d;
    int live6 = e * f * g;
    int live7 = h & 0xFF;
    int live8 = (a ^ b) & (c ^ d);
    
    /* Use all values in computation to ensure they're live */
    int sum_before = live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    
    /* Function call with many live registers */
    sink_func1();
    
    /* Use all live values after call - forces caller-save */
    int sum_after = live1 - live2 + live3 - live4 + live5 - live6 + live7 - live8;
    
    /* This return makes the call the last instruction in its basic block */
    return sum_before + sum_after;
}

/* Test 2: Call in switch case (creates basic block ending with call) */
NOINLINE int test_call_in_switch_case(int x, int a, int b, int c, int d, int e) {
    int result = 0;
    
    /* Create live values */
    int val1 = a * x + 1;
    int val2 = b * x + 2;
    int val3 = c * x + 3;
    int val4 = d * x + 4;
    int val5 = e * x + 5;
    
    switch (x & 3) {
        case 0: {
            /* Many live values across call */
            int tmp1 = val1 ^ val2;
            int tmp2 = val3 | val4;
            sink_func2();  /* Call at end of basic block before break */
            result = tmp1 + tmp2 + val5;
            break;  /* Basic block ends with call, then break */
        }
        case 1: {
            int tmp1 = val1 & val2;
            int tmp2 = val3 ^ val4;
            /* Another call site */
            VOLATILE_CALL(sink_func3);
            result = tmp1 - tmp2 + val5;
            break;
        }
        case 2: {
            /* Even more live values */
            int tmp1 = val1 + val2 + val3;
            int tmp2 = val4 * val5;
            int tmp3 = (val1 << 2) | (val2 >> 1);
            sink_func4();
            result = tmp1 * tmp2 - tmp3;
            break;
        }
        default:
            result = val1 + val2 + val3 + val4 + val5;
    }
    
    return result;
}

/* Test 3: Complex loop with values live across call */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int i;
    int accum[8] = {0};  /* Multiple accumulators */
    
    /* Initialize with seed values */
    for (i = 0; i < 8; i++) {
        accum[i] = seed + i * 7;
    }
    
    /* Loop creates many live values */
    for (i = 0; i < n; i++) {
        /* Update all accumulators - creates register pressure */
        accum[0] = accum[0] * 3 + i;
        accum[1] = accum[1] ^ (accum[0] & 0xFF);
        accum[2] = accum[2] + accum[1] * 2;
        accum[3] = accum[3] | (accum[2] << 1);
        accum[4] = accum[4] - accum[3] / 2;
        accum[5] = accum[5] & (accum[4] | 0x5555);
        accum[6] = accum[6] * accum[5] + 1;
        accum[7] = accum[7] ^ accum[6];
        
        /* Function call with many live values */
        if (i % 4 == 0) {
            sink_func1();  /* Call in middle of loop with live values */
        }
        
        /* More operations using all accumulators */
        accum[0] = accum[0] + accum[7];
        accum[1] = accum[1] - accum[6];
        accum[2] = accum[2] * accum[5];
        accum[3] = accum[3] | accum[4];
        accum[4] = accum[4] ^ accum[3];
        accum[5] = accum[5] & accum[2];
        accum[6] = accum[6] + accum[1];
        accum[7] = accum[7] - accum[0];
    }
    
    /* Final computation using all accumulators */
    int result = 0;
    for (i = 0; i < 8; i++) {
        result += accum[i];
    }
    
    /* One more call at the end */
    sink_func2();
    
    return result;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int a, int b, int c, int d) {
    int x1 = a * b + 123;
    int x2 = c * d - 456;
    int x3 = (a ^ c) | (b ^ d);
    int x4 = (a << 3) + (b >> 2);
    int x5 = c * 7 + d * 11;
    int x6 = (a & b) ^ (c & d);
    int x7 = x1 + x2 + x3;
    int x8 = x4 * x5 - x6;
    
    if (a > b) {
        if (c > d) {
            /* Many live values, call at end of inner block */
            int tmp = x1 + x2 + x3 + x4;
            sink_func3();
            return tmp + x5 + x6;  /* Return right after call */
        } else {
            int tmp = x7 * x8;
            VOLATILE_CALL(sink_func4);
            return tmp - x1;  /* Another call at block end */
        }
    } else {
        if (c < d) {
            int tmp = x2 ^ x3 ^ x4;
            sink_func1();
            return tmp | x5;  /* Call then return */
        }
    }
    
    /* Default path with one more call */
    sink_func2();
    return x7 + x8;
}

/* Test 5: Multiple calls in same function with overlapping live ranges */
NOINLINE int test_multiple_calls(int a, int b, int c, int d, int e, int f) {
    /* Phase 1: Compute values */
    int v1 = a + b;
    int v2 = c * d;
    int v3 = e ^ f;
    int v4 = (a << b) | (c >> d);
    int v5 = v1 + v2 + v3;
    
    /* First call with v1-v5 live */
    sink_func1();
    
    /* Phase 2: More computations */
    int v6 = v4 * v5 + 1;
    int v7 = v2 ^ v3 & v4;
    int v8 = v1 | v6;
    int v9 = v5 - v7 + v8;
    
    /* Second call with v6-v9 and some older values live */
    sink_func2();
    
    /* Phase 3: Even more values */
    int v10 = v6 * v7 / 2;
    int v11 = v8 ^ v9;
    int v12 = v10 + v11 + v4;
    
    /* Third call at potential block end */
    if (v12 > 1000) {
        int tmp = v9 + v10 + v11;
        sink_func3();
        return tmp;  /* Call then return */
    }
    
    /* Final call */
    VOLATILE_CALL(sink_func4);
    return v12;
}

int main(void) {
    int total = 0;
    int i;
    
    /* Seed for reproducibility */
    srand(42);
    
    /* Run all tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        int d = rand() % 100;
        int e = rand() % 100;
        int f = rand() % 100;
        int g = rand() % 100;
        int h = rand() % 100;
        int n = 5 + (rand() % 10);
        
        total += test_call_at_bb_end(a, b, c, d, e, f, g, h);
        total += test_call_in_switch_case(i, a, b, c, d, e);
        total += test_call_between_complex_ops(n, i);
        total += test_nested_conditionals(a, b, c, d);
        total += test_multiple_calls(a, b, c, d, e, f);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
