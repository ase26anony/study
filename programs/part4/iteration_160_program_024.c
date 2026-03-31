/* test-caller-save.c
 * This program is designed to trigger the uncovered lines in GCC's caller-save.cc
 * Specifically, it forces insertion of save/restore instructions around function calls
 * where those instructions become the new end of a basic block.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and optimization of helper functions */
#define NOINLINE __attribute__((noinline, noclone))

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_sink++; }
NOINLINE void func2(void) { global_sink--; }
NOINLINE void func3(void) { global_sink ^= 1; }
NOINLINE void func4(void) { global_sink |= 0x55; }

/* Test 1: Function call at the end of a basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across the call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0xDEADBEEF;
    int v4 = d | 0x12345678;
    int v5 = e + f * 7;
    int v6 = (a ^ b) & (c | d);
    int v7 = v1 + v2 - v3;
    int v8 = v4 * v5 / (v6 + 1);
    
    /* Force register pressure - more values than caller-saved registers */
    int t1 = v1 + v2;
    int t2 = v3 - v4;
    int t3 = v5 ^ v6;
    int t4 = v7 | v8;
    int t5 = t1 * t2;
    int t6 = t3 + t4;
    
    /* Non-inline call with many live values */
    func1();
    
    /* Use all live values after the call - this extends their liveness */
    int result = t1 + t2 + t3 + t4 + t5 + t6 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    
    /* The call is at the end of a basic block before return */
    return result;
}

/* Test 2: Function call in a switch case that ends with break */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    switch (x & 3) {
        case 0: {
            /* Create live values before the call */
            int a = y * 2 + 1;
            int b = z << 3;
            int c = a ^ b;
            int d = (y + z) * 7;
            int e = a | b | c;
            int f = d & 0xFF;
            
            /* Non-inline call */
            func2();
            
            /* Use values after call */
            result = a + b - c + d + e + f;
            break;  /* Creates basic block ending with the call */
        }
        
        case 1: {
            int a = y + z;
            int b = y - z;
            int c = y * z;
            int d = y ^ z;
            
            func3();
            
            result = a * b + c - d;
            break;
        }
        
        case 2: {
            /* Even more register pressure */
            int v1 = x + y;
            int v2 = x - y;
            int v3 = x * y;
            int v4 = x ^ y;
            int v5 = y + z;
            int v6 = y - z;
            int v7 = x + z;
            int v8 = x - z;
            
            func4();
            
            result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
            break;
        }
        
        default:
            result = x + y + z;
    }
    
    return result;
}

/* Test 3: Complex loop with values live across call */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int i;
    int acc = seed;
    
    /* Compute many values in a loop */
    int vals[8];
    for (i = 0; i < 8; i++) {
        vals[i] = (seed + i) * (i + 1);
        acc ^= vals[i];
    }
    
    /* Additional computations to increase register pressure */
    int t1 = vals[0] + vals[1];
    int t2 = vals[2] - vals[3];
    int t3 = vals[4] * vals[5];
    int t4 = vals[6] | vals[7];
    int t5 = t1 ^ t2;
    int t6 = t3 & t4;
    int t7 = acc << 2;
    int t8 = ~acc;
    
    /* Non-inline call with many live values */
    void (*volatile fp)(void) = func1;
    fp();  /* Volatile function pointer prevents optimization */
    
    /* Use all computed values after the call */
    int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
    for (i = 0; i < 8; i++) {
        result += vals[i];
    }
    
    return result;
}

/* Test 4: Nested conditionals creating multiple BB ends with calls */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int result = 0;
    
    if (a > 0) {
        int x1 = a * 2 + 1;
        int x2 = b << 1;
        int x3 = c ^ 0xABCD;
        int x4 = x1 + x2;
        int x5 = x3 - x1;
        
        if (b > 0) {
            int y1 = x1 * x2;
            int y2 = x3 | x4;
            int y3 = x5 ^ y1;
            int y4 = y2 + y3;
            
            func2();
            
            /* Call at BB end before return */
            return y1 + y2 + y3 + y4;
        } else {
            int z1 = x4 - x5;
            int z2 = x1 ^ x2;
            int z3 = x3 & x4;
            
            func3();
            
            result = z1 + z2 + z3;
        }
    } else {
        int w1 = b * c;
        int w2 = a + b + c;
        int w3 = w1 ^ w2;
        int w4 = (a << 4) | (b << 2) | c;
        
        func4();
        
        result = w1 + w2 + w3 + w4;
    }
    
    return result;
}

/* Test 5: Multiple consecutive calls with overlapping live ranges */
NOINLINE int test_multiple_calls(int a, int b, int c, int d) {
    /* First set of live values */
    int v1 = a + b;
    int v2 = c - d;
    int v3 = a ^ c;
    int v4 = b | d;
    
    func1();
    
    /* Second set overlapping with first */
    int v5 = v1 * v2;
    int v6 = v3 & v4;
    int v7 = v5 + v6;
    int v8 = v1 - v4;
    
    func2();
    
    /* Third set */
    int v9 = v5 ^ v7;
    int v10 = v6 | v8;
    int v11 = v9 * v10;
    int v12 = v7 - v9;
    
    func3();
    
    /* Use all values */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12;
}

int main(void) {
    int total = 0;
    int i;
    
    /* Seed for reproducible but varied inputs */
    srand(42);
    
    /* Run each test multiple times with different inputs */
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
        total += test_multiple_calls(a, b, c, d);
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
