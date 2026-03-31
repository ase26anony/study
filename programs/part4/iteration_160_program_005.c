/* Test program to trigger caller-save insertion at basic block end */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization of calls */
#define NOINLINE __attribute__((noinline,noipa))

/* Volatile sink to prevent dead code elimination */
static volatile int global_sink;

/* Non-inline functions that will force caller-save decisions */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int x, int y, int z, int w) {
    /* Create many live values that must be preserved across call */
    int a = x * 2 + 1;
    int b = y * 3 - 2;
    int c = z << 1;
    int d = w ^ 0x5555;
    int e = x + y + z + w;
    int f = (x * y) | (z & w);
    int g = ~(x ^ y);
    int h = (z << 2) | (w >> 2);
    
    /* Function call with many live registers */
    func1();
    
    /* Use all live values after call - forces them to be preserved */
    return a + b + c + d + e + f + g + h;
}

/* Test 2: Call in switch case that ends with break */
NOINLINE int test_call_in_switch_case(int selector, int x, int y, int z) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Many live values before call */
            int a = x + 1;
            int b = y * 2;
            int c = z & 0xFF;
            int d = x ^ y ^ z;
            int e = (x << 3) | (y >> 1);
            int f = ~z;
            
            /* Call at what could be end of basic block */
            func2();
            
            /* Use values after call */
            result = a + b - c + d + e + f;
            break;  /* Creates basic block ending with call */
        }
        case 1: {
            int a = x * y;
            int b = z + 100;
            func3();
            result = a - b;
            break;
        }
        default: {
            result = x + y + z;
            break;
        }
    }
    
    return result;
}

/* Test 3: Complex control flow with call at block end */
NOINLINE int test_complex_control_flow(int cond, int x, int y) {
    int a = x * 3;
    int b = y * 5;
    int c = x + y;
    int d = x ^ y;
    int e = ~x;
    int f = y << 2;
    
    if (cond > 0) {
        /* More live values in this branch */
        int g = a * 2;
        int h = b + 10;
        int i = c ^ d;
        
        /* Call with many live values */
        func4();
        
        /* This could make the call the last insn before return */
        return g + h + i + e + f;
    } else {
        int j = a + b;
        int k = c * d;
        func1();
        return j - k + e - f;
    }
}

/* Test 4: Loop with values live across call at end */
NOINLINE int test_loop_and_call(int iterations, int seed) {
    int acc = seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Compute many values in registers */
        int a = acc + i;
        int b = acc * i;
        int c = acc ^ i;
        int d = ~acc;
        int e = acc << (i & 3);
        int f = acc >> 1;
        int g = i * 3;
        int h = i + 5;
        
        /* Call in loop body - values must be preserved */
        func2();
        
        /* Use all values - forces preservation across call */
        acc = a + b - c + d + e + f + g + h;
        
        if (acc & 1) {
            /* Early return with call at block end */
            func3();
            return acc;  /* Call is last instruction before return */
        }
    }
    
    func1();
    return acc;
}

/* Test 5: Nested calls with register pressure */
NOINLINE int test_nested_pressure(int x, int y, int z) {
    /* Volatile function pointer to prevent optimization */
    void (*volatile fp1)(void) = func1;
    void (*volatile fp2)(void) = func2;
    
    /* Extreme register pressure */
    int v1 = x + 1;
    int v2 = y * 2;
    int v3 = z & 0xFF;
    int v4 = x ^ y;
    int v5 = y ^ z;
    int v6 = z ^ x;
    int v7 = v1 + v2;
    int v8 = v3 * v4;
    int v9 = v5 | v6;
    int v10 = ~v7;
    int v11 = v8 << 1;
    int v12 = v9 >> 2;
    int v13 = v10 ^ v11;
    int v14 = v12 + v13;
    int v15 = v14 * 3;
    
    /* First call */
    fp1();
    
    /* Intermediate use keeps values live */
    int sum1 = v1 + v2 + v3 + v4 + v5 + v6;
    
    /* More computations */
    int w1 = sum1 + v7;
    int w2 = v8 - v9;
    int w3 = v10 & v11;
    int w4 = v12 | v13;
    int w5 = v14 ^ v15;
    int w6 = w1 * w2;
    int w7 = w3 + w4;
    int w8 = w5 - w6;
    int w9 = w7 ^ w8;
    
    /* Second call */
    fp2();
    
    /* Final use of all values */
    return sum1 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + v15;
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use command line args or defaults for variability */
    int x = argc > 1 ? atoi(argv[1]) : 12345;
    int y = argc > 2 ? atoi(argv[2]) : 67890;
    int z = argc > 3 ? atoi(argv[3]) : 54321;
    int w = argc > 4 ? atoi(argv[4]) : 98765;
    
    /* Run all tests to trigger different caller-save scenarios */
    result += test_call_at_bb_end(x, y, z, w);
    result += test_call_in_switch_case(x & 3, y, z, w);
    result += test_complex_control_flow(x > y, z, w);
    result += test_loop_and_call(5, x);
    result += test_nested_pressure(x, y, z);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
