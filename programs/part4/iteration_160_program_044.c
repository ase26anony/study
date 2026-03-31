/* test-caller-save.c
 * Test program to trigger caller-save register spill insertion
 * at basic block boundaries, specifically targeting uncovered
 * lines in caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))(func); \
    fp(); \
} while(0)

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c & 0xFF;
    int v4 = d | 0x1234;
    int v5 = e ^ 0xABCD;
    int v6 = f + 100;
    int v7 = a + b + c;
    int v8 = d - e - f;
    int v9 = (a * b) / (c + 1);
    int v10 = (d << 3) | (e >> 2);
    
    /* Function call at end of basic block before return */
    if (a > 0) {
        /* All these values must be saved across the call */
        func1();
        /* This return makes the call the BB_END */
        return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    } else {
        /* Alternative path to ensure both branches exist */
        int v11 = a * b * c;
        int v12 = d + e + f;
        int v13 = (a << 4) & 0xFF;
        int v14 = b ^ c ^ d;
        func2();
        return v11 + v12 + v13 + v14;
    }
}

/* Test 2: Call in switch case with break */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int l1 = x * 2;
    int l2 = y + 5;
    int l3 = z & 0x0F;
    int l4 = x ^ y ^ z;
    int l5 = (x << 3) | (y >> 1);
    int l6 = z * 3 + 7;
    int l7 = x + y * 2;
    int l8 = z - x - y;
    
    switch (x & 0x3) {
        case 0:
            /* Call at end of case before break - creates BB ending with call */
            func1();
            result = l1 + l2;
            break;
        case 1:
            /* More live values across call */
            int t1 = l3 * l4;
            int t2 = l5 ^ l6;
            func2();
            result = t1 + t2 + l7;
            break;
        case 2:
            /* Even more register pressure */
            int u1 = l1 * l2 * l3;
            int u2 = l4 | l5 | l6;
            int u3 = l7 & l8;
            VOLATILE_CALL(func3);  /* Volatile call through pointer */
            result = u1 + u2 + u3;
            break;
        default:
            /* Complex computation before call */
            for (int i = 0; i < 3; i++) {
                l1 += i;
                l2 -= i;
                l3 ^= i;
            }
            func4();
            result = l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
            break;
    }
    
    return result;
}

/* Test 3: Call between complex operations with loop */
NOINLINE int test_call_between_complex_ops(int seed) {
    /* Create array of values computed in loop */
    int values[12];
    int sum = 0;
    
    /* Compute many values before call */
    for (int i = 0; i < 12; i++) {
        values[i] = seed * i + (i << 2);
        values[i] ^= 0x12345678;
        values[i] = (values[i] * 3) / (seed + 1);
    }
    
    /* Additional live variables */
    int a = seed * 2;
    int b = seed + 100;
    int c = seed ^ 0xFF;
    int d = seed << 4;
    int e = seed >> 2;
    int f = seed | 0xABCD;
    int g = seed & 0x7777;
    int h = seed - 50;
    int j = seed * seed;
    int k = seed + seed * 2;
    
    /* Non-inline call with many live values */
    func1();
    
    /* Use all values after call */
    for (int i = 0; i < 12; i++) {
        sum += values[i];
    }
    
    sum += a + b + c + d + e + f + g + h + j + k;
    
    /* Conditional to create basic block structure */
    if (sum > 1000) {
        int extra1 = a * b;
        int extra2 = c ^ d;
        int extra3 = e | f;
        func2();
        return sum + extra1 + extra2 + extra3;
    } else {
        int extra4 = g & h;
        int extra5 = j * k;
        VOLATILE_CALL(func3);
        return sum - extra4 - extra5;
    }
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int p, int q, int r) {
    /* Create many live values */
    int x1 = p * q;
    int x2 = q * r;
    int x3 = r * p;
    int x4 = p + q + r;
    int x5 = p ^ q ^ r;
    int x6 = (p << 2) | (q >> 1);
    int x7 = r & 0x7F;
    int x8 = p % (q + 1);
    int x9 = q * 3 - r;
    int x10 = (r << 3) & 0xFF;
    
    /* Complex conditional structure */
    if (p > q) {
        if (q > r) {
            /* Call at end of inner if block */
            func1();
            return x1 + x2 + x3;
        } else {
            int y1 = x4 * x5;
            int y2 = x6 ^ x7;
            /* Call before return at block end */
            func2();
            return y1 + y2;
        }
    } else {
        if (p > r) {
            int z1 = x8 + x9;
            int z2 = x10 * 2;
            /* Volatile call at block end */
            VOLATILE_CALL(func3);
            return z1 - z2;
        } else {
            /* Multiple calls in same block */
            int w1 = x1 * x2 * x3;
            func1();
            int w2 = w1 + x4 + x5;
            func2();
            int w3 = w2 ^ x6 ^ x7;
            func3();
            /* Final call at actual block end before return */
            func4();
            return w3 + x8 + x9 + x10;
        }
    }
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use command line arguments or random seeds for variability */
    int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    int seed3 = argc > 3 ? atoi(argv[3]) : 13579;
    int seed4 = argc > 4 ? atoi(argv[4]) : 24680;
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        result += test_call_at_bb_end(seed1 + i, seed2 - i, seed3 * i, 
                                     seed4 / (i + 1), i * 10, i * 20);
        result += test_call_in_switch_case(seed2 + i, seed3 - i, seed4 * i);
        result += test_call_between_complex_ops(seed3 + i * 7);
        result += test_nested_conditionals(seed4 - i, seed1 * i, seed2 + i * 3);
    }
    
    /* Print result to prevent optimization */
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
