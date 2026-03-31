/* test-caller-save.c - Test program to trigger caller-save insertion at BB_END */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile function pointer to prevent optimization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fp;

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that get called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * b + 1;
    int v2 = b * c - 2;
    int v3 = c * d + 3;
    int v4 = d * e - 4;
    int v5 = e * f + 5;
    int v6 = f * a - 6;
    
    /* Additional computations to increase register pressure */
    int t1 = v1 ^ v2;
    int t2 = v3 | v4;
    int t3 = v5 & v6;
    int t4 = v1 + v3 + v5;
    int t5 = v2 + v4 + v6;
    
    /* Call at what could be BB_END if this is in a conditional */
    if (a > 0) {
        /* All these values are live across the call */
        func1();
        
        /* This return makes the call the last instruction in BB */
        return t1 + t2 + t3 + t4 + t5 + v1 + v2 + v3 + v4 + v5 + v6;
    } else {
        /* Different path to create control flow */
        int r1 = v1 * 2;
        int r2 = v2 / 3;
        func2();
        return r1 + r2;
    }
}

/* Test 2: Call in switch case that ends with break */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    /* Create live values */
    int a = x * y + z;
    int b = y * z + x;
    int c = z * x + y;
    int d = a ^ b ^ c;
    int e = (a & b) | c;
    int f = (b & c) | a;
    int g = (c & a) | b;
    
    int result = 0;
    
    switch (x & 0x3) {
        case 0:
            /* Many live values across call */
            func1();
            result = a + b;
            break;  /* Call is at BB_END before break */
            
        case 1:
            /* Different set of live values */
            int t1 = d + e;
            int t2 = f + g;
            func2();
            result = t1 * t2;
            break;
            
        case 2:
            /* Even more live values */
            int u1 = a * d;
            int u2 = b * e;
            int u3 = c * f;
            int u4 = d * g;
            func3();
            result = u1 + u2 + u3 + u4;
            break;
            
        default:
            /* Call through volatile pointer */
            volatile_fp = func4;
            volatile_fp();
            result = a + b + c + d + e + f + g;
            break;
    }
    
    return result;
}

/* Test 3: Complex loop before call, then use all values after */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    /* Array of values computed in loop, all live across call */
    int vals[16];
    int i;
    
    /* Compute many values in loop - creates register pressure */
    for (i = 0; i < 16; i++) {
        vals[i] = seed * (i + 1);
        seed = seed * 1103515245 + 12345;
    }
    
    /* More computations to ensure values stay in registers */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    sum1 = vals[0] + vals[1] + vals[2] + vals[3];
    sum2 = vals[4] + vals[5] + vals[6] + vals[7];
    sum3 = vals[8] + vals[9] + vals[10] + vals[11];
    sum4 = vals[12] + vals[13] + vals[14] + vals[15];
    
    /* Intermediate computations */
    int prod1 = sum1 * sum2;
    int prod2 = sum3 * sum4;
    int diff1 = sum1 - sum2;
    int diff2 = sum3 - sum4;
    
    /* Non-inline call with many live values */
    func1();
    
    /* Use all computed values after call */
    int result = prod1 + prod2 + diff1 + diff2;
    for (i = 0; i < 16; i++) {
        result += vals[i];
    }
    
    return result;
}

/* Test 4: Nested conditionals with calls at BB ends */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int x = a * a + b * b;
    int y = b * b + c * c;
    int z = c * c + a * a;
    
    int r1 = x ^ y;
    int r2 = y ^ z;
    int r3 = z ^ x;
    int r4 = r1 * r2;
    int r5 = r2 * r3;
    int r6 = r3 * r1;
    
    if (a > b) {
        if (b > c) {
            /* Call at end of inner if block */
            func1();
            return r1 + r2 + r3;
        } else {
            int t1 = r4 + r5;
            int t2 = r5 + r6;
            /* Call at end of else block */
            func2();
            return t1 * t2;
        }
    } else {
        if (a > c) {
            int u1 = r4 ^ r5;
            int u2 = r5 ^ r6;
            /* Call at end of inner else-if */
            func3();
            return u1 + u2;
        } else {
            /* Call through volatile pointer at BB_END */
            volatile_fp = func4;
            volatile_fp();
            return r1 + r2 + r3 + r4 + r5 + r6;
        }
    }
}

/* Test 5: Multiple consecutive calls with live values */
NOINLINE int test_multiple_calls(int a, int b, int c, int d, int e, int f) {
    /* Compute many independent values */
    int v1 = a + b;
    int v2 = b + c;
    int v3 = c + d;
    int v4 = d + e;
    int v5 = e + f;
    int v6 = f + a;
    
    int w1 = v1 * v2;
    int w2 = v3 * v4;
    int w3 = v5 * v6;
    
    /* First call - some values might be spilled/restored */
    func1();
    
    /* Use values, then compute more */
    int x1 = w1 + v1;
    int x2 = w2 + v3;
    int x3 = w3 + v5;
    
    /* Second call - different spill decisions */
    func2();
    
    /* More computations */
    int y1 = x1 * x2;
    int y2 = x2 * x3;
    int y3 = x3 * x1;
    
    /* Third call at potential BB_END in conditional */
    if (y1 > y2) {
        func3();
        return y1 + y2 + y3;
    } else {
        volatile_fp = func4;
        volatile_fp();
        return y1 * y2 * y3;
    }
}

int main(void) {
    int total = 0;
    int i;
    
    /* Initialize volatile function pointer */
    volatile_fp = func1;
    
    printf("Running caller-save BB_END insertion tests...\n");
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_in_switch_case(i, i*2, i*3);
        total += test_call_between_complex_ops(i % 8, i);
        total += test_nested_conditionals(i, i+10, i+20);
        total += test_multiple_calls(i, i+1, i+2, i+3, i+4, i+5);
    }
    
    printf("Total result: %d\n", total);
    printf("(This value is meaningless; the important part is the compilation)\n");
    
    return 0;
}
