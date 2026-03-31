/* caller-save-test.c - Test program to trigger caller-save insertion logic */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and optimization */
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

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0x55AA55AA;
    int v4 = d + e * f;
    int v5 = (a ^ b) | (c & d);
    int v6 = e * 7 - f;
    int v7 = (a + b + c + d) & 0xFF;
    int v8 = (e << 3) | (f >> 1);
    
    /* Function call at end of basic block before return */
    if (v1 > 0) {
        /* All v1-v8 must be preserved across this call */
        func1();
        /* This return makes the call the last instruction in BB */
        return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    } else {
        /* Alternative path to ensure both branches exist */
        int v9 = v1 * v2;
        int v10 = v3 | v4;
        func2();
        return v9 - v10 + v5 + v6 + v7 + v8;
    }
}

/* Test 2: Call in switch case with break */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int l1 = x * 2;
    int l2 = y + 5;
    int l3 = z ^ 0x12345678;
    int l4 = (x + y) * z;
    int l5 = x | y | z;
    int l6 = (x * y) - z;
    int l7 = (x << 4) | (y << 2) | z;
    int l8 = ~(x ^ y ^ z);
    
    switch (x & 0x3) {
        case 0:
            /* Call at end of case before break */
            func1();
            result = l1 + l2;
            break;
        case 1:
            /* More complex case with multiple live values */
            result = l3 * l4;
            func2();
            /* Call is at end of BB before break */
            break;
        case 2:
            /* Even more live values */
            result = l5 + l6 + l7 + l8;
            func3();
            break;
        default:
            result = l1 - l2 + l3 - l4;
            func4();
            break;
    }
    
    /* Use all live values to ensure they must be preserved */
    return result + l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8;
}

/* Test 3: Call between complex operations with loop */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int i;
    int values[8];
    
    /* Initialize many values in a loop */
    for (i = 0; i < 8; i++) {
        values[i] = seed * (i + 1) + (i * i);
    }
    
    /* More computations creating register pressure */
    int t1 = values[0] * values[1];
    int t2 = values[2] ^ values[3];
    int t3 = values[4] | values[5];
    int t4 = values[6] & values[7];
    int t5 = t1 + t2;
    int t6 = t3 - t4;
    int t7 = (t5 << 2) | (t6 >> 1);
    int t8 = ~(t1 ^ t2 ^ t3 ^ t4);
    
    /* Non-inline call with many live values */
    VOLATILE_CALL(func1);
    
    /* Use all computed values after call */
    int sum = 0;
    for (i = 0; i < 8; i++) {
        sum += values[i];
    }
    
    return sum + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int r1 = a + b;
    int r2 = b * c;
    int r3 = a ^ c;
    int r4 = (a << 3) | (b << 1) | c;
    int r5 = ~(a + b + c);
    int r6 = r1 * r2 - r3;
    int r7 = r4 ^ r5;
    int r8 = r6 & r7;
    
    if (a > 0) {
        if (b > 0) {
            func1();
            /* Call at end of inner if block */
            return r1 + r2 + r3;
        } else {
            int t = r4 + r5;
            func2();
            /* Call at end of else block */
            return t + r6 + r7;
        }
    } else {
        if (c > 0) {
            int u = r7 * r8;
            func3();
            /* Call at end of this block */
            return u + r1 + r2;
        } else {
            func4();
            /* Call at end of final else block */
            return r3 + r4 + r5 + r6 + r7 + r8;
        }
    }
}

/* Test 5: Multiple consecutive calls with live values */
NOINLINE int test_multiple_calls(int x, int y, int z) {
    /* Create a chain of computations with calls in between */
    int v1 = x * 2 + 1;
    int v2 = y << 1;
    int v3 = z ^ 0xDEADBEEF;
    
    func1();  /* v1-v3 must be saved across this call */
    
    int v4 = v1 + v2;
    int v5 = v2 * v3;
    int v6 = v1 | v2 | v3;
    
    func2();  /* v4-v6 must be saved across this call */
    
    int v7 = v4 ^ v5;
    int v8 = v6 + v1;
    int v9 = v2 & v3;
    
    /* Final call at potential BB end */
    if (v7 > 100) {
        func3();
        return v7 + v8 + v9;
    } else {
        func4();
        return v7 - v8 + v9;
    }
}

int main(void) {
    int total = 0;
    int i;
    
    /* Seed for reproducible but varied inputs */
    srand(42);
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 100; i++) {
        int a = rand() % 1000;
        int b = rand() % 1000;
        int c = rand() % 1000;
        int d = rand() % 1000;
        int e = rand() % 1000;
        int f = rand() % 1000;
        
        total += test_call_at_bb_end(a, b, c, d, e, f);
        total += test_call_in_switch_case(a, b, c);
        total += test_call_between_complex_ops(i, a);
        total += test_nested_conditionals(a, b, c);
        total += test_multiple_calls(a, b, c);
        
        /* Prevent loop unrolling from simplifying too much */
        if (total > 1000000) {
            total %= 1000000;
        }
    }
    
    printf("Total: %d\n", total);
    return 0;
}
