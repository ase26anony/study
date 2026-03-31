/* test-caller-save.c - Program to trigger specific caller-save optimization paths */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations */
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

/* Test 1: Function call at the end of a basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live values that must survive across the call */
    int v1 = a + b;
    int v2 = c * d;
    int v3 = e ^ f;
    int v4 = g & h;
    int v5 = a - c;
    int v6 = b + d;
    int v7 = e | f;
    int v8 = g ^ h;
    
    /* More computations to increase register pressure */
    int v9 = v1 * v2;
    int v10 = v3 + v4;
    int v11 = v5 ^ v6;
    int v12 = v7 & v8;
    
    /* Call at what should be BB_END before return */
    func1();
    
    /* Use all live values after call - forces them to be saved/restored */
    return v1 + v2 - v3 + v4 * v5 - v6 + v7 ^ v8 + v9 - v10 + v11 * v12;
}

/* Test 2: Function call in a switch case that ends with break */
NOINLINE int test_call_in_switch_case(int x, int a, int b, int c, int d, int e, int f) {
    int result = 0;
    
    switch (x & 3) {
        case 0: {
            /* Create many live values in this case */
            int v1 = a * a;
            int v2 = b + b;
            int v3 = c ^ 0xFF;
            int v4 = d | 0x0F;
            int v5 = e << 2;
            int v6 = f >> 1;
            int v7 = v1 + v2;
            int v8 = v3 - v4;
            
            /* Call that should be at BB_END before break */
            func2();
            
            /* Use values after call */
            result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
            break;  /* Creates basic block ending with the call */
        }
        case 1: {
            int v1 = a + 1;
            int v2 = b + 2;
            func3();
            result = v1 * v2;
            break;
        }
        case 2: {
            int v1 = c * 3;
            int v2 = d / 2;
            func4();
            result = v1 - v2;
            break;
        }
        default:
            result = e + f;
    }
    
    return result;
}

/* Test 3: Complex loop before call, then use of all values after call */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int i;
    /* Array of values that will be computed in loop and used after call */
    int vals[12];
    
    /* Compute many different values in a loop */
    for (i = 0; i < 12; i++) {
        /* Different computations for each slot to prevent optimization */
        switch (i % 4) {
            case 0: vals[i] = (seed + i) * 3; break;
            case 1: vals[i] = (seed ^ i) + 7; break;
            case 2: vals[i] = (seed | i) - 5; break;
            case 3: vals[i] = (seed & i) * 2; break;
        }
    }
    
    /* More intermediate computations */
    int t1 = vals[0] + vals[1];
    int t2 = vals[2] * vals[3];
    int t3 = vals[4] ^ vals[5];
    int t4 = vals[6] | vals[7];
    int t5 = vals[8] & vals[9];
    int t6 = vals[10] - vals[11];
    
    /* Call with many values live */
    VOLATILE_CALL(func1);
    
    /* Use all computed values after the call */
    int sum = 0;
    for (i = 0; i < 12; i++) {
        sum += vals[i];
    }
    sum += t1 + t2 + t3 + t4 + t5 + t6;
    
    return sum;
}

/* Test 4: Nested condition with call at end of each branch */
NOINLINE int test_nested_conditions(int cond1, int cond2, int a, int b, int c, int d, int e, int f) {
    int result = 0;
    
    if (cond1) {
        int v1 = a + b;
        int v2 = c * d;
        int v3 = v1 ^ v2;
        int v4 = a & c;
        int v5 = b | d;
        
        if (cond2) {
            int v6 = v3 + 10;
            int v7 = v4 * 2;
            int v8 = v5 ^ 0xAA;
            
            /* Call at BB_END before return */
            func3();
            
            return v6 + v7 + v8;  /* Return immediately after call */
        } else {
            int v6 = v3 - 5;
            int v7 = v4 / 2;
            int v8 = v5 & 0x55;
            
            /* Another call at BB_END */
            func4();
            
            return v6 * v7 - v8;  /* Return immediately after call */
        }
    } else {
        int v1 = e + f;
        int v2 = a * b;
        int v3 = c ^ d;
        int v4 = v1 & v2;
        int v5 = v3 | 0xFF;
        
        /* Call at BB_END before return */
        VOLATILE_CALL(func2);
        
        return v4 + v5;  /* Return immediately after call */
    }
}

/* Test 5: Multiple consecutive calls with live values between them */
NOINLINE int test_multiple_calls(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* First set of live values */
    int v1 = a + 1;
    int v2 = b * 2;
    int v3 = c ^ 3;
    int v4 = d | 4;
    
    /* First call - v1-v4 must be saved */
    func1();
    
    /* Use values, then compute more */
    int v5 = v1 + v2;
    int v6 = v3 - v4;
    int v7 = v5 * v6;
    int v8 = a + b + c + d;
    
    /* Second call - v5-v8 must be saved */
    func2();
    
    /* More computations */
    int v9 = v7 + v8;
    int v10 = v5 ^ v6;
    int v11 = v1 & v2;
    int v12 = v3 | v4;
    
    /* Third call - v9-v12 must be saved */
    func3();
    
    /* Final use of all values */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12;
}

int main(void) {
    int total = 0;
    int i;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run each test multiple times with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
        total += test_call_in_switch_case(i, i*2, i*3, i*4, i*5, i*6, i*7);
        total += test_call_between_complex_ops(i % 10, i);
        total += test_nested_conditions(i & 1, i & 2, i, i+1, i+2, i+3, i+4, i+5);
        total += test_multiple_calls(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be non-zero and deterministic)\n");
    
    return total != 0 ? 0 : 1;
}
