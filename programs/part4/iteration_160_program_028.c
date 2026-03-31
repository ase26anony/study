/* Test program to trigger caller-save insertion at basic block end */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noclone, noipa))

/* Volatile globals to prevent optimization */
volatile int global_seed = 12345;
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink++; }
NOINLINE void func2(void) { global_sink--; }
NOINLINE void func3(int x) { global_sink += x; }
NOINLINE void func4(void) { /* empty */ }

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0xABCD;
    int v4 = d + e * f;
    int v5 = (a & b) | (c & d);
    int v6 = e * 7 - f;
    int v7 = (a + b + c + d) & 0xFF;
    int v8 = (e << 3) | (f & 0xF);
    
    /* Use volatile function pointer to ensure call isn't optimized */
    void (*volatile fp)(void) = func1;
    
    /* Complex condition that creates basic block ending with call */
    if ((a + b + c) > 1000) {
        /* All these values are live across the call */
        fp();  /* This call should be at BB end */
        
        /* Return uses all live values - forces them to survive */
        return v1 + v2 - v3 + v4 - v5 + v6 - v7 + v8;
    } else {
        /* Different computation to create control flow */
        return v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8;
    }
}

/* Test 2: Call in switch case at basic block end */
NOINLINE int test_call_in_switch_case(int x, int a, int b, int c, int d) {
    int result = 0;
    
    /* Create many live values */
    int l1 = a * x + 1;
    int l2 = b * x + 2;
    int l3 = c * x + 3;
    int l4 = d * x + 4;
    int l5 = (a & b) * x;
    int l6 = (c | d) * x;
    int l7 = (a ^ b ^ c ^ d) * x;
    int l8 = x * x * x;
    
    switch (x & 0x7) {
        case 0:
            result = l1 + l2;
            func2();  /* Call at BB end before break */
            break;
        case 1:
            result = l3 - l4;
            /* Multiple calls to increase pressure */
            func3(l5);
            break;
        case 2:
            result = l5 * l6;
            func1();
            break;
        case 3:
            result = l7 / (l8 ? l8 : 1);
            func4();
            break;
        default:
            /* More live value usage */
            result = (l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8) & 0xFFFF;
            func2();
            break;
    }
    
    /* Use all live values to keep them alive */
    return result + (l1 & 1) + (l2 & 2) + (l3 & 4) + (l4 & 8);
}

/* Test 3: Call between complex operations with loop */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int i;
    int values[16];
    
    /* Initialize array with complex computations */
    for (i = 0; i < 16; i++) {
        values[i] = (seed * i) ^ (seed << (i & 0xF));
    }
    
    /* Compute many intermediate values in registers */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int prod1 = 1, prod2 = 1;
    
    for (i = 0; i < 8; i++) {
        sum1 += values[i] * 2;
        sum2 += values[i + 8] * 3;
        prod1 *= (values[i] & 0xFF) + 1;
        prod2 *= (values[i + 8] & 0x7F) + 1;
    }
    
    /* More intermediate values */
    int t1 = sum1 ^ sum2;
    int t2 = prod1 | prod2;
    int t3 = (sum1 << 3) & (sum2 >> 2);
    int t4 = (prod1 * 7) % (prod2 + 1);
    int t5 = t1 + t2 + t3 + t4;
    int t6 = (t1 * t2) - (t3 * t4);
    int t7 = (t5 ^ t6) & 0xFFFFFF;
    int t8 = (t5 + t6) * 2;
    
    /* Volatile function call - all t1-t8 must survive */
    void (*volatile fp2)(void) = func4;
    fp2();
    
    /* Use all computed values after call */
    int result = t1 + t2 * 2 + t3 / 4 + t4 * 3 + t5 - t6 + t7 * t8;
    
    /* More computations to use array values */
    for (i = 0; i < 16; i++) {
        result += (values[i] & 1) ? values[i] : -values[i];
    }
    
    return result;
}

/* Test 4: Nested condition with calls at block ends */
NOINLINE int test_nested_conditions(int a, int b, int c, int d) {
    int x1 = a * b + c;
    int x2 = b * c + d;
    int x3 = c * d + a;
    int x4 = d * a + b;
    int x5 = (a ^ b) * (c ^ d);
    int x6 = (a & b) | (c & d);
    int x7 = (a | b) & (c | d);
    int x8 = ~(a * b * c * d) & 0xFFFF;
    
    int result = 0;
    
    if (a > 0) {
        if (b > 0) {
            func1();
            result = x1 + x2;  /* Call at BB end in inner block */
        } else {
            result = x3 - x4;
            func2();  /* Call at BB end before block exit */
        }
        /* More live values used */
        result += x5;
    } else {
        if (c > 0) {
            result = x6 * x7;
            func3(result);  /* Call with argument */
        } else {
            func4();
            result = x8 / (a ? a : 1);
        }
        result -= x1;
    }
    
    /* Force all x values to be live as long as possible */
    return result + (x1 & 1) + (x2 & 2) + (x3 & 4) + (x4 & 8) +
           (x5 & 16) + (x6 & 32) + (x7 & 64) + (x8 & 128);
}

/* Main driver */
int main(void) {
    int total = 0;
    int i;
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_in_switch_case(i, i*2, i*3, i*4, i*5);
        total += test_call_between_complex_ops(i % 16, i * 137);
        total += test_nested_conditions(i, i+100, i+200, i+300);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 10 == 0) {
            global_seed = total & 0xFF;
        }
    }
    
    printf("Total: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
