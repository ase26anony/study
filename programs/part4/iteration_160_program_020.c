/* test-caller-save.c - Test program to trigger caller-save insertion logic */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations */
#define NOINLINE __attribute__((noinline, noclone))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (func); \
    fp(); \
} while(0)

/* Non-inline functions to force actual calls */
NOINLINE void func1(void) { asm volatile("" : : : "memory"); }
NOINLINE void func2(void) { asm volatile("" : : : "memory"); }
NOINLINE void func3(void) { asm volatile("" : : : "memory"); }

/* Global volatile to prevent dead code elimination */
volatile int global_sink;

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0xABCD;
    int v4 = d | 0x1234;
    int v5 = e + f * 7;
    int v6 = a ^ b ^ c;
    int v7 = d - e + f;
    int v8 = (a * b) / (c + 1);
    
    /* All these values are live across the call */
    int sum_before = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    
    /* Function call with many live registers */
    func1();
    
    /* Use all live values after call - forces caller-save */
    int result = v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8;
    
    /* Conditional that makes the call the last instruction in BB */
    if (sum_before > 1000) {
        /* More computations to increase register pressure */
        int t1 = v1 * v2;
        int t2 = v3 / (v4 + 1);
        int t3 = v5 ^ v6;
        int t4 = v7 & v8;
        
        /* Call at the end of basic block before return */
        func2();
        /* This call should be BB_END, and inserted save/restore 
           should become new BB_END */
        return t1 + t2 + t3 + t4 + result;
    } else {
        /* Alternative path */
        return result + sum_before;
    }
}

/* Test 2: Call in switch case */
NOINLINE int test_call_in_switch_case(int x, int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0xFF;
    int d = seed << 3;
    int e = seed - 100;
    int f = seed | 0xAA;
    
    int result = 0;
    
    switch (x % 4) {
        case 0:
            /* Many live values across call */
            a = a * 2 + 1;
            b = b ^ a;
            c = c | b;
            d = d + c;
            e = e - d;
            f = f * e;
            
            /* Call in the middle of case */
            func1();
            
            /* Use values after call */
            result = a + b + c + d + e + f;
            break;  /* Call is before break, so it's at BB end */
            
        case 1:
            result = a - b + c - d;
            break;
            
        case 2:
            /* Another call site */
            a = a + b;
            b = b + c;
            c = c + d;
            func3();
            result = a * b * c;
            break;
            
        default:
            result = seed;
    }
    
    return result;
}

/* Test 3: Complex loop with call */
NOINLINE int test_call_between_complex_ops(int iterations) {
    /* Create array of values in registers */
    int vals[10];
    int i, j;
    
    /* Initialize with complex computations */
    for (i = 0; i < 10; i++) {
        vals[i] = (i * iterations) ^ 0x12345678;
        for (j = 0; j < 3; j++) {
            vals[i] = (vals[i] << 3) | (vals[i] >> 29);  /* rotate */
            vals[i] ^= (j * 0x11111111);
        }
    }
    
    /* More live values */
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Pre-call computations creating many live values */
    for (i = 0; i < 5; i++) {
        sum1 += vals[i] * 2;
        sum2 += vals[i + 5] ^ 0xAA;
        sum3 += vals[i] & vals[9 - i];
    }
    
    /* Intermediate values that must survive */
    int t1 = sum1 * 3;
    int t2 = sum2 << 1;
    int t3 = sum3 / 2;
    int t4 = sum1 ^ sum2;
    int t5 = sum2 | sum3;
    int t6 = sum3 & sum1;
    
    /* Critical call with many live values */
    func1();
    
    /* Use all values after call */
    int result = t1 + t2 + t3 + t4 + t5 + t6;
    
    /* More computations to ensure values are used */
    for (i = 0; i < 10; i++) {
        result += vals[i] * (i + 1);
    }
    
    /* Store to volatile to prevent elimination */
    global_sink = result;
    
    return result;
}

/* Test 4: Nested conditionals with calls at BB ends */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int x = a * 3;
    int y = b + 7;
    int z = c ^ 0xFF;
    
    int result = 0;
    
    if (a > 0) {
        int t1 = x * y;
        int t2 = y * z;
        int t3 = z * x;
        
        if (b > 0) {
            int u1 = t1 + t2;
            int u2 = t2 + t3;
            int u3 = t3 + t1;
            
            /* Call at end of inner if block */
            func2();
            /* This should be BB_END before insertion */
            result = u1 + u2 + u3;
        } else {
            result = t1 - t2 + t3;
        }
        
        /* Another call in outer block */
        func1();
        result += x + y + z;
    } else {
        result = a + b + c;
    }
    
    return result;
}

/* Main driver */
int main(void) {
    int total = 0;
    int i;
    
    printf("Testing caller-save insertion scenarios...\n");
    
    /* Run multiple tests with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_in_switch_case(i, i * 7 + 3);
        total += test_call_between_complex_ops(i % 10 + 5);
        total += test_nested_conditionals(i, i-50, i*2);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
