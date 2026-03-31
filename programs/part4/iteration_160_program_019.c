/* test-caller-save.c - Test program to trigger caller-save insertion at BB_END */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile function pointer to prevent optimization */
typedef void (*volatile func_ptr_t)(void);

/* Global volatile sink to prevent dead code elimination */
volatile int global_sink;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink = 1; }
NOINLINE void func2(void) { global_sink = 2; }
NOINLINE void func3(void) { global_sink = 3; }
NOINLINE void func4(void) { global_sink = 4; }

/* Test 1: Call at the end of a basic block before return */
NOINLINE int test_call_at_bb_end(int x, int y, int z, int w) {
    /* Create many live values that must survive across the call */
    int a = x * 2 + 1;
    int b = y / 3 - 2;
    int c = z ^ 0xABCD;
    int d = w & 0x1234;
    int e = x + y + z + w;
    int f = x * y - z * w;
    int g = (x << 3) | (y >> 2);
    int h = ~z & 0xFF;
    
    /* Use volatile function pointer to ensure call isn't optimized */
    func_ptr_t fp = func1;
    fp();  /* This call should trigger caller-save for live registers */
    
    /* Use all live values after the call - forces them to be preserved */
    return a + b + c + d + e + f + g + h;
}

/* Test 2: Call in a switch case that ends with break */
NOINLINE int test_call_in_switch_case(int selector, int x, int y, int z) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Many live values in this case */
            int a = x * 3;
            int b = y * 5;
            int c = z * 7;
            int d = a ^ b;
            int e = b | c;
            int f = c & a;
            
            func2();  /* Call at end of basic block before break */
            
            result = a + b + c + d + e + f;
            break;  /* Basic block ends with the call, then break */
        }
        case 1: {
            int a = x + y;
            int b = y + z;
            result = a * b;
            break;
        }
        case 2: {
            int a = x - y;
            int b = y - z;
            result = a * b;
            break;
        }
        default:
            result = x + y + z;
    }
    
    return result;
}

/* Test 3: Call between complex operations with loop-unrolled values */
NOINLINE int test_call_between_complex_ops(int seed) {
    /* Unrolled loop creates many independent values */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    
    /* More operations to increase register pressure */
    int a1 = v1 + v2;
    int a2 = v3 ^ v4;
    int a3 = v5 | v6;
    int a4 = v7 & v8;
    int a5 = v9 - v10;
    
    /* Intermediate computations */
    int b1 = a1 * a2;
    int b2 = a3 + a4;
    int b3 = a5 << 2;
    
    /* Call with many values live across it */
    func3();
    
    /* Use all values after call */
    int r1 = b1 + v1 + v2;
    int r2 = b2 ^ v3 ^ v4;
    int r3 = b3 | v5 | v6;
    int r4 = a4 & v7 & v8;
    int r5 = a5 - v9 - v10;
    
    return r1 + r2 + r3 + r4 + r5;
}

/* Test 4: Call in conditional branch at end of basic block */
NOINLINE int test_call_in_conditional(int x, int y, int cond) {
    /* Create many live values */
    int a = x * x;
    int b = y * y;
    int c = x + y;
    int d = x - y;
    int e = x ^ y;
    int f = x | y;
    int g = x & y;
    
    int result;
    
    if (cond > 0) {
        /* More computations in the if branch */
        int h = a + b;
        int i = c * d;
        int j = e ^ f;
        
        func4();  /* Call at end of basic block before return */
        
        /* This return creates a basic block ending with the call */
        return h + i + j + g;
    } else {
        /* Different computations */
        int h = a - b;
        int i = c / (d ? d : 1);
        result = h + i;
    }
    
    /* Use remaining values */
    return result + a + b + c;
}

/* Test 5: Nested calls with live values */
NOINLINE int test_nested_live_values(int x) {
    /* Chain of computations creating many live values */
    int v1 = x + 1;
    int v2 = v1 * 2;
    int v3 = v2 - 3;
    int v4 = v3 ^ 4;
    int v5 = v4 | 5;
    int v6 = v5 & 6;
    int v7 = v6 << 1;
    int v8 = v7 >> 2;
    int v9 = v8 + 7;
    int v10 = v9 * 8;
    
    /* First call */
    func1();
    
    /* More computations keeping values live */
    int a1 = v1 + v2;
    int a2 = v3 + v4;
    int a3 = v5 + v6;
    
    /* Second call - more values live across this one */
    func2();
    
    /* Final use of all values */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + a1 + a2 + a3;
}

int main(void) {
    int total = 0;
    
    /* Run tests with different inputs to ensure execution */
    total += test_call_at_bb_end(1, 2, 3, 4);
    total += test_call_at_bb_end(5, 6, 7, 8);
    
    total += test_call_in_switch_case(0, 10, 20, 30);
    total += test_call_in_switch_case(1, 40, 50, 60);
    total += test_call_in_switch_case(2, 70, 80, 90);
    
    total += test_call_between_complex_ops(100);
    total += test_call_between_complex_ops(200);
    
    total += test_call_in_conditional(15, 25, 1);
    total += test_call_in_conditional(35, 45, 0);
    
    total += test_nested_live_values(123);
    total += test_nested_live_values(456);
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
