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
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at basic block end before return */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across call */
    int a = x + 1;
    int b = y * 2;
    int c = z & 0xFF;
    int d = x ^ y;
    int e = y | z;
    int f = z - x;
    int g = x * y;
    int h = y + z;
    int i = z * 3;
    int j = x << 2;
    
    /* Force register pressure - use all values before call */
    int sum1 = a + b + c + d;
    int sum2 = e + f + g + h;
    int sum3 = i + j + x + y;
    
    /* Call at what should be BB end before conditional return */
    if (x > 0) {
        /* All values still live here */
        func1();
        /* This call is at BB end before return */
        return sum1 + sum2 + sum3 + a;  /* Use 'a' to ensure it stays live */
    } else {
        /* Different path to create another BB ending with call */
        func2();
        return sum1 - sum2 + b;  /* Use 'b' */
    }
}

/* Test 2: Call in switch case with break */
NOINLINE int test_call_in_switch_case(int x, int y) {
    int result = 0;
    
    /* Create many live values */
    int v1 = x * 2;
    int v2 = y + 7;
    int v3 = x ^ y;
    int v4 = y * 3;
    int v5 = x & 0xF;
    int v6 = y | 0xAA;
    int v7 = x + y;
    int v8 = y - x;
    int v9 = x * y;
    int v10 = y << 1;
    
    switch (x % 4) {
        case 0:
            /* Use values before call */
            result = v1 + v2;
            func1();
            /* Call at BB end before break */
            break;
            
        case 1:
            result = v3 + v4 + v5;
            func2();
            /* This should create BB ending with call */
            break;
            
        case 2:
            /* Even more live values */
            result = v6 + v7 + v8 + v9;
            func3();
            break;
            
        default:
            result = v10 + v1 + v2 + v3 + v4;
            func4();
            break;
    }
    
    /* Use all values after switch to keep them live across calls */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test 3: Complex loop with call in middle */
NOINLINE int test_call_between_complex_ops(int iterations) {
    int arr[10];
    int i;
    
    /* Initialize with computations */
    for (i = 0; i < 10; i++) {
        arr[i] = (i * iterations) ^ 0xABCD;
    }
    
    /* Create many live values from array */
    int t1 = arr[0] + arr[1];
    int t2 = arr[1] * arr[2];
    int t3 = arr[2] & arr[3];
    int t4 = arr[3] | arr[4];
    int t5 = arr[4] ^ arr[5];
    int t6 = arr[5] + arr[6];
    int t7 = arr[6] * arr[7];
    int t8 = arr[7] & arr[8];
    int t9 = arr[8] | arr[9];
    int t10 = arr[9] ^ arr[0];
    
    /* Additional computations to increase register pressure */
    int u1 = t1 * 2;
    int u2 = t2 + 7;
    int u3 = t3 ^ 0xFF;
    int u4 = t4 & 0xAA;
    int u5 = t5 | 0x55;
    
    /* Non-inline call with many live values */
    VOLATILE_CALL(func1);
    
    /* Use all values after call */
    int sum = 0;
    sum += t1 + t2 + t3 + t4 + t5;
    sum += t6 + t7 + t8 + t9 + t10;
    sum += u1 + u2 + u3 + u4 + u5;
    
    /* Force conditional that might make call BB end */
    if (sum > 1000) {
        VOLATILE_CALL(func2);
        return sum;
    } else {
        VOLATILE_CALL(func3);
        return sum * 2;
    }
}

/* Test 4: Nested conditionals with calls at ends */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    /* Create many interdependent values */
    int x1 = a + b;
    int x2 = b * c;
    int x3 = c ^ a;
    int x4 = a & b;
    int x5 = b | c;
    int x6 = c - a;
    int x7 = a * 3;
    int x8 = b + 5;
    int x9 = c * 2;
    int x10 = a ^ b ^ c;
    
    int result = 0;
    
    if (a > 0) {
        if (b > 0) {
            result = x1 + x2 + x3;
            func1();  /* Could be BB end */
        } else {
            result = x4 + x5 + x6;
            func2();  /* Could be BB end */
            /* Add another computation to potentially make this not BB end */
            result += x7;
        }
        /* Use more values to keep them live */
        result += x8;
    } else {
        if (c > 0) {
            result = x9 + x10 + x1;
            func3();  /* Could be BB end */
        } else {
            result = x2 + x3 + x4;
            func4();  /* Could be BB end */
        }
        result += x5;
    }
    
    /* Final use of all values */
    return result + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
}

/* Main driver that runs all tests */
int main(void) {
    int total = 0;
    int i;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i*2, i*3);
        total += test_call_at_bb_end(-i, i+1, i*4);
    }
    
    /* Run test 2 */
    for (i = 0; i < 8; i++) {
        total += test_call_in_switch_case(i, i*3);
    }
    
    /* Run test 3 */
    for (i = 1; i <= 5; i++) {
        total += test_call_between_complex_ops(i);
    }
    
    /* Run test 4 */
    for (i = 0; i < 6; i++) {
        total += test_nested_conditionals(i, i-2, i+2);
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be non-zero and deterministic)\n");
    
    /* Store to volatile to prevent optimization */
    global_sink = total;
    
    return total != 0 ? 0 : 1;
}
