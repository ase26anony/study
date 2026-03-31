/* test-caller-save.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_CALL(func) do { \
    void (*volatile fp)(void) = (void (*)(void))(func); \
    fp(); \
} while(0)

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink++; }
NOINLINE void func2(void) { global_sink--; }
NOINLINE void func3(void) { global_sink ^= 1; }
NOINLINE void func4(void) { global_sink |= 0x55; }

/* Test 1: Call at basic block end before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int live1 = a * 3 + 1;
    int live2 = b << 2;
    int live3 = c ^ 0xABCD;
    int live4 = d | 0x1234;
    int live5 = e + f;
    int live6 = a - b;
    int live7 = c * d;
    int live8 = e ^ f;
    
    /* Force register pressure - use all values in computation */
    int sum1 = live1 + live2;
    int sum2 = live3 + live4;
    int sum3 = live5 + live6;
    int sum4 = live7 + live8;
    
    /* Complex condition to create basic block boundary */
    if (a > b) {
        /* More computations to increase register pressure */
        int tmp1 = sum1 * 2;
        int tmp2 = sum2 / 3;
        int tmp3 = sum3 << 1;
        int tmp4 = sum4 >> 2;
        
        /* Non-inline call at the end of basic block before return */
        func1();
        
        /* This return makes the call the last instruction in BB */
        return tmp1 + tmp2 + tmp3 + tmp4;
    } else {
        /* Alternative path with different computations */
        int tmp1 = sum1 ^ sum2;
        int tmp2 = sum3 & sum4;
        int tmp3 = live1 | live3;
        int tmp4 = live5 ^ live7;
        
        /* Another call at BB end */
        func2();
        
        return tmp1 - tmp2 + tmp3 - tmp4;
    }
}

/* Test 2: Call in switch case with break */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int v1 = x * 17;
    int v2 = y + 42;
    int v3 = z ^ 0xFF;
    int v4 = x & y;
    int v5 = z | 0xAA;
    int v6 = x + y + z;
    int v7 = x * y * z;
    int v8 = (x << 3) | (y << 2) | z;
    
    /* Switch creates multiple basic blocks */
    switch (x % 4) {
        case 0:
            /* Use all live values before call */
            result = v1 + v2 - v3;
            /* Call at end of case before break */
            func3();
            break;
            
        case 1:
            result = v4 * v5 / 2;
            /* Multiple calls in sequence */
            func1();
            func2();
            break;
            
        case 2:
            /* Complex computation with many temporaries */
            int t1 = v6 + v7;
            int t2 = v8 ^ v1;
            int t3 = v2 & v3;
            int t4 = v4 | v5;
            result = t1 + t2 + t3 + t4;
            func4();
            break;
            
        default:
            result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
            VOLATILE_CALL(func1);  /* Volatile call */
            break;
    }
    
    /* Use all values again after switch to keep them live */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Test 3: Call between complex operations with loop */
NOINLINE int test_call_between_complex_ops(int seed, int iterations) {
    int arr[8];
    int i;
    
    /* Initialize array with complex computations */
    for (i = 0; i < 8; i++) {
        arr[i] = seed * (i + 1);
        arr[i] ^= (arr[i] << 3);
        arr[i] += i * 17;
    }
    
    /* More live values from array elements */
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Loop creates register pressure */
    for (i = 0; i < iterations; i++) {
        /* Use all array elements in different ways */
        sum1 += arr[0] + arr[1];
        sum2 += arr[2] * arr[3];
        sum3 ^= arr[4] | arr[5];
        sum4 &= arr[6] ^ arr[7];
        
        /* Modify array elements to prevent optimization */
        arr[i % 8] += i;
    }
    
    /* Create additional live values */
    int live1 = sum1 * 3;
    int live2 = sum2 >> 2;
    int live3 = sum3 ^ 0xDEAD;
    int live4 = sum4 | 0xBEEF;
    int live5 = arr[0] + arr[7];
    int live6 = arr[1] * arr[6];
    int live7 = arr[2] & arr[5];
    int live8 = arr[3] | arr[4];
    
    /* Non-inline call with many live values */
    func1();
    
    /* Use all live values after call */
    int result = live1 + live2 + live3 + live4;
    result += live5 - live6 + live7 ^ live8;
    
    /* Use array elements again */
    for (i = 0; i < 8; i++) {
        result += arr[i];
    }
    
    return result;
}

/* Test 4: Nested calls with register pressure */
NOINLINE int test_nested_calls(int a, int b, int c) {
    /* Create many interdependent values */
    int x1 = a + b;
    int x2 = b + c;
    int x3 = a + c;
    int x4 = a * b;
    int x5 = b * c;
    int x6 = a * c;
    int x7 = x1 ^ x2;
    int x8 = x3 & x4;
    int x9 = x5 | x6;
    int x10 = x7 + x8;
    
    /* First call */
    func2();
    
    /* More computations between calls */
    int y1 = x10 * 2;
    int y2 = x9 >> 1;
    int y3 = x8 ^ 0x55;
    int y4 = x7 & 0xAA;
    
    /* Second call - different function */
    func3();
    
    /* Final computations using all values */
    int z1 = y1 + y2;
    int z2 = y3 - y4;
    int z3 = x1 * x2;
    int z4 = x3 / (x4 ? x4 : 1);
    
    /* Conditional that creates BB boundary */
    if (z1 > z2) {
        int final1 = z3 + z4;
        /* Call at BB end before return */
        func4();
        return final1;
    } else {
        int final2 = z3 - z4;
        /* Another call at BB end */
        VOLATILE_CALL(func1);
        return final2;
    }
}

/* Main driver */
int main(void) {
    int total = 0;
    int i;
    
    printf("Testing caller-save edge cases...\n");
    
    /* Run multiple tests with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5);
        total += test_call_in_switch_case(i, i*2, i*3);
        total += test_call_between_complex_ops(i, 5);
        total += test_nested_calls(i, i+10, i+20);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
