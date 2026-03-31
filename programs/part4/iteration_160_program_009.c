/* caller_save_test.c - Test program to trigger GCC caller-save optimization */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile function pointer to prevent optimization */
typedef void (*volatile_func_t)(void);

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int live1 = a * b + c;
    int live2 = b * c - d;
    int live3 = c * d ^ e;
    int live4 = d * e | f;
    int live5 = e * f & a;
    int live6 = f * a + b;
    int live7 = a ^ b ^ c;
    int live8 = b | c | d;
    
    /* Additional computations to increase register pressure */
    int tmp1 = live1 * 2;
    int tmp2 = live2 / 3;
    int tmp3 = live3 << 2;
    int tmp4 = live4 >> 1;
    
    /* Call at the end of basic block before return */
    if (a > 0) {
        /* All these values must be saved/restored */
        int sum = live1 + live2 + live3 + live4;
        int prod = live5 * live6 * live7 * live8;
        
        /* Non-inline call with volatile function pointer */
        volatile_func_t fp = func1;
        fp();
        
        /* This return makes the call the last instruction in BB */
        return sum + prod + tmp1 + tmp2 + tmp3 + tmp4;
    } else {
        /* Different path to create control flow */
        return live1 + live2;
    }
}

/* Test 2: Call in switch case with live values */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int v1 = x * 31;
    int v2 = y * 47;
    int v3 = z * 73;
    int v4 = x ^ y ^ z;
    int v5 = x | y | z;
    int v6 = x & y & z;
    int v7 = x + y + z;
    int v8 = x - y - z;
    
    switch (x % 4) {
        case 0:
            /* Use all live values before call */
            result = v1 + v2 + v3;
            /* Non-inline call */
            func2();
            /* Call is at end of basic block before break */
            break;
            
        case 1:
            result = v4 * v5;
            func3();
            break;
            
        case 2:
            /* More complex computation with all values */
            result = v1 * v2 + v3 * v4 - v5 * v6 + v7 * v8;
            /* Call through volatile pointer */
            volatile_func_t fp = func4;
            fp();
            break;
            
        default:
            result = v6 + v7 + v8;
            break;
    }
    
    /* Use all values after switch to extend liveness */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Test 3: Call between complex operations with loop */
NOINLINE int test_call_between_complex_ops(int seed) {
    int values[8];
    int i;
    
    /* Compute many values in a loop */
    for (i = 0; i < 8; i++) {
        values[i] = seed * (i + 1);
        values[i] ^= (values[i] << 3);
        values[i] += i * 17;
    }
    
    /* Additional computations to increase register pressure */
    int a = values[0] * values[1];
    int b = values[2] ^ values[3];
    int c = values[4] | values[5];
    int d = values[6] & values[7];
    int e = a + b + c + d;
    int f = a * b - c * d;
    int g = (a ^ b) | (c ^ d);
    int h = (a & b) + (c & d);
    
    /* Non-inline call with many live values */
    volatile_func_t fp = func1;
    fp();
    
    /* Use all computed values after call */
    int sum = 0;
    for (i = 0; i < 8; i++) {
        sum += values[i];
    }
    
    return sum + a + b + c + d + e + f + g + h;
}

/* Test 4: Nested calls with live values */
NOINLINE int test_nested_calls(int p, int q, int r) {
    /* Create many interdependent values */
    int x1 = p * q + r;
    int x2 = q * r + p;
    int x3 = r * p + q;
    int x4 = x1 ^ x2 ^ x3;
    int x5 = x1 | x2 | x3;
    int x6 = x1 & x2 & x3;
    
    /* First call */
    func2();
    
    /* More computations between calls */
    int y1 = x4 * x5;
    int y2 = x5 * x6;
    int y3 = x6 * x4;
    
    /* Second call */
    volatile_func_t fp = func3;
    fp();
    
    /* Final computation using all values */
    return x1 + x2 + x3 + x4 + x5 + x6 + y1 + y2 + y3;
}

/* Test 5: Call in both branches of if-else */
NOINLINE int test_call_in_both_branches(int cond, int a, int b, int c) {
    /* Many live values */
    int v1 = a * 2;
    int v2 = b * 3;
    int v3 = c * 5;
    int v4 = a ^ b;
    int v5 = b ^ c;
    int v6 = c ^ a;
    int v7 = v1 + v2 + v3;
    int v8 = v4 * v5 * v6;
    
    if (cond > 0) {
        /* Call at end of this basic block before return */
        int result = v1 + v2 + v3 + v4;
        func1();
        return result + v7;
    } else {
        /* Different call, still at end of basic block */
        int result = v5 + v6 + v7 + v8;
        func2();
        return result + v1;
    }
}

int main(void) {
    int total = 0;
    int i;
    
    /* Seed for reproducible but varied inputs */
    srand(42);
    
    /* Run tests multiple times with different inputs */
    for (i = 0; i < 100; i++) {
        int a = rand() % 100;
        int b = rand() % 100;
        int c = rand() % 100;
        int d = rand() % 100;
        int e = rand() % 100;
        int f = rand() % 100;
        
        total += test_call_at_bb_end(a, b, c, d, e, f);
        total += test_call_in_switch_case(a, b, c);
        total += test_call_between_complex_ops(a);
        total += test_nested_calls(a, b, c);
        total += test_call_in_both_branches(a, b, c, d);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
