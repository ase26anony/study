/* test-caller-save.c - Program to trigger caller-save optimization edge cases */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of these functions */
#define NOINLINE __attribute__((noinline, noclone))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))func; \
    fp(); \
} while(0)

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int x, int y, int z, int w) {
    /* Create many live values that must survive across call */
    int a = x * 3 + 1;
    int b = y << 2;
    int c = z & 0xFF;
    int d = w ^ 0xAAAA;
    int e = x + y + z + w;
    int f = (x * y) | (z << 1);
    int g = ~(x ^ y);
    int h = (z * w) >> 2;
    
    /* Force register pressure - all values must be live across call */
    if (x > 0) {
        /* This call is at the end of a basic block before return */
        func1();
        /* All 8 values must be preserved across the call */
        return a + b - c + d - e + f - g + h;
    } else {
        /* Different path to create control flow */
        func2();
        return a - b + c - d + e - f + g - h;
    }
}

/* Test 2: Call in switch case with complex live values */
NOINLINE int test_call_in_switch_case(int selector, int v1, int v2, int v3) {
    int result = 0;
    
    /* Create many live values before switch */
    int live1 = v1 * 2 + 1;
    int live2 = v2 << 3;
    int live3 = v3 & 0x5555;
    int live4 = ~v1;
    int live5 = v2 * v3;
    int live6 = (v1 ^ v2) | v3;
    int live7 = v1 + v2 + v3;
    int live8 = (v1 << 2) | (v2 >> 1);
    
    switch (selector & 3) {
        case 0:
            /* This call is at end of basic block before break */
            func1();
            result = live1 + live2;
            break;
        case 1:
            /* Multiple calls with live values */
            func2();
            result = live3 - live4;
            break;
        case 2:
            /* Complex computation then call at block end */
            live5 = live5 * 2;
            live6 = live6 ^ 0xFF;
            func3();
            result = live5 + live6 + live7;
            break;
        default:
            /* Call with all live values used after */
            func4();
            result = live1 + live2 + live3 + live4 + 
                    live5 + live6 + live7 + live8;
            break;
    }
    
    /* Use all live values to ensure they must be preserved */
    return result + (live1 & 1) + (live2 & 2) + (live3 & 4) + (live4 & 8);
}

/* Test 3: Call between complex operations with loop-generated values */
NOINLINE int test_call_between_complex_ops(int base, int iterations) {
    int i;
    /* Array of values that will be live across call */
    int vals[12];
    
    /* Generate many distinct values in registers */
    for (i = 0; i < 12; i++) {
        vals[i] = (base + i) * (i + 1);
        vals[i] = vals[i] ^ (vals[i] >> 3);
    }
    
    /* Force all values to be in registers before call */
    int sum1 = 0, sum2 = 0;
    for (i = 0; i < 6; i++) {
        sum1 += vals[i] * (i + 1);
        sum2 += vals[i + 6] * (i + 2);
    }
    
    /* Non-inline call with many live values */
    VOLATILE_CALL(func1);
    
    /* Use all values after call - forcing caller-save decisions */
    int total = 0;
    for (i = 0; i < 12; i++) {
        total += vals[i];
        if (i % 2) total -= vals[i] >> 1;
        else total ^= vals[i];
    }
    
    return total + sum1 - sum2;
}

/* Test 4: Nested calls with live values at each level */
NOINLINE int helper1(int a, int b) {
    int x = a * 3;
    int y = b << 1;
    func2();  /* Call in helper */
    return x - y;
}

NOINLINE int helper2(int a, int b, int c) {
    int x = a + b;
    int y = c * 2;
    int z = x ^ y;
    func3();  /* Another call */
    return z + (x & y);
}

NOINLINE int test_nested_calls(int p1, int p2, int p3) {
    /* Create live values that span multiple calls */
    int v1 = p1 * p2;
    int v2 = p2 + p3;
    int v3 = p1 ^ p3;
    int v4 = (p1 << 3) | (p2 >> 1);
    int v5 = ~p1;
    int v6 = p2 * p3;
    
    /* First call with some values live */
    int r1 = helper1(v1, v2);
    
    /* More computations */
    v3 = v3 + r1;
    v4 = v4 ^ v1;
    
    /* Second call with different values live */
    int r2 = helper2(v3, v4, v5);
    
    /* Final call at block end before return */
    if (r1 > r2) {
        func4();
        return v1 + v2 + v3 + v4 + v5 + v6 + r1;
    } else {
        func1();
        return v1 - v2 + v3 - v4 + v5 - v6 + r2;
    }
}

/* Test 5: Call with live values in conditional at block end */
NOINLINE int test_conditional_block_end(int cond, int a, int b, int c) {
    /* Create many live values */
    int x1 = a * 2 + 1;
    int x2 = b << 2;
    int x3 = c & 0xFF;
    int x4 = a ^ b ^ c;
    int x5 = (a + b) * c;
    int x6 = ~(a | b | c);
    int x7 = (a << 1) | (b << 2) | (c << 3);
    int x8 = (a * b) + (b * c) + (c * a);
    
    if (cond) {
        /* Call at end of this basic block before return */
        func1();
        return x1 + x2 + x3 + x4;
    } else if (cond & 1) {
        /* Different path, different call */
        func2();
        return x5 - x6 + x7;
    } else {
        /* Final path with all values used */
        func3();
        return x1 - x2 + x3 - x4 + x5 - x6 + x7 - x8;
    }
}

int main(void) {
    int total = 0;
    int i;
    
    /* Seed for reproducible but varied inputs */
    srand(42);
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 100; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        int d = rand() % 100;
        
        total += test_call_at_bb_end(a, b, c, d);
        total += test_call_in_switch_case(i % 4, a, b, c);
        total += test_call_between_complex_ops(a, 3 + (i % 5));
        total += test_nested_calls(a, b, c);
        total += test_conditional_block_end(i % 3, a, b, c);
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be non-zero and consistent for given seed)\n");
    
    return total != 0 ? 0 : 1;
}
