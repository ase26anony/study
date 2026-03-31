/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting save/restore instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from removing our calls */
#define NOINLINE __attribute__((noinline, noipa))
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

/* ============================================
 * Test 1: Call at basic block end before return
 * Forces BB_END update when inserting save/restore
 * ============================================ */
NOINLINE int test_call_at_bb_end(int x, int y, int z, int w) {
    /* Create many live values that must survive across call */
    int a = x + 1;
    int b = y * 2;
    int c = z & 0xFF;
    int d = w ^ 0x55;
    int e = a + b;
    int f = c - d;
    int g = e * f;
    int h = (a << 2) | (b >> 1);
    int i = c ^ d ^ e;
    int j = f + g + h;
    
    /* This call is at the end of a basic block before return */
    if (x > y) {
        /* All these values are live across the call */
        func1();
        /* Call is last instruction before return in this BB */
        return a + b + c + d + e + f + g + h + i + j;
    } else {
        /* Alternative path with different live values */
        int k = x * y * z;
        int l = w + 100;
        int m = k / (l + 1);
        int n = m << 3;
        int o = n ^ k;
        int p = l - m;
        
        func2();
        /* Another call at BB end before return */
        return k + l + m + n + o + p;
    }
}

/* ============================================
 * Test 2: Call in switch case with break
 * Creates basic block ending with call instruction
 * ============================================ */
NOINLINE int test_call_in_switch_case(int selector, int x, int y, int z) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Many live values across call */
            int v1 = x + y;
            int v2 = x * y;
            int v3 = z << 2;
            int v4 = v1 ^ v2;
            int v5 = v3 + v4;
            int v6 = v5 * 2;
            int v7 = v6 - v1;
            int v8 = v7 & 0xFF;
            
            func3();  /* Call at end of case BB before break */
            
            result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
            break;
        }
        case 1: {
            /* Different set of live values */
            int w1 = x ^ y;
            int w2 = z * 3;
            int w3 = w1 + w2;
            int w4 = w3 << 1;
            int w5 = w4 - x;
            int w6 = w5 & y;
            int w7 = w6 | z;
            int w8 = w7 * 2;
            
            VOLATILE_CALL(func4);  /* Volatile call at BB end */
            
            result = w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8;
            break;
        }
        default: {
            int d1 = x * z;
            int d2 = y + 100;
            int d3 = d1 ^ d2;
            int d4 = d3 << 1;
            result = d4;
            break;
        }
    }
    
    return result;
}

/* ============================================
 * Test 3: Call between complex operations
 * High register pressure with loop computations
 * ============================================ */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    /* Compute many values in a way that creates register pressure */
    int vals[16];
    int sum = 0;
    
    /* Pre-call computations - all must stay live */
    for (int i = 0; i < 8; i++) {
        vals[i] = (seed + i) * (i + 1);
        vals[i] ^= (vals[i] << 3);
        vals[i] += i * 17;
    }
    
    /* Additional independent computations */
    int a = seed * 3;
    int b = seed + 777;
    int c = a ^ b;
    int d = c << 2;
    int e = d / 3;
    int f = e + 12345;
    int g = f & 0xFFFF;
    int h = g * 2;
    
    /* Critical call with many live values */
    if (iterations > 0) {
        func1();
    } else {
        func2();
    }
    
    /* Post-call use of all pre-call values */
    for (int i = 0; i < 8; i++) {
        sum += vals[i];
    }
    
    sum += a + b + c + d + e + f + g + h;
    
    /* More computations after call */
    int i = sum * 2;
    int j = i ^ seed;
    int k = j + 1000;
    int l = k & 0xFF;
    
    return sum + i + j + k + l;
}

/* ============================================
 * Test 4: Nested conditionals with calls at BB ends
 * ============================================ */
NOINLINE int test_nested_conditionals(int a, int b, int c, int d) {
    int result = 0;
    
    if (a > 0) {
        int x1 = a * 2;
        int x2 = b + 10;
        int x3 = c ^ d;
        int x4 = x1 + x2;
        int x5 = x3 * x4;
        int x6 = x5 & 0xFF;
        
        if (b > 0) {
            int y1 = x1 * x2;
            int y2 = x3 + x4;
            int y3 = x5 ^ x6;
            int y4 = y1 + y2;
            int y5 = y3 * y4;
            
            func2();  /* Call at inner BB end */
            
            result = y1 + y2 + y3 + y4 + y5;
        } else {
            int z1 = x1 ^ x2;
            int z2 = x3 * x4;
            int z3 = x5 + x6;
            int z4 = z1 & z2;
            int z5 = z3 | z4;
            
            func3();  /* Call at inner BB end */
            
            result = z1 + z2 + z3 + z4 + z5;
        }
    } else {
        int w1 = b * c;
        int w2 = d + 100;
        int w3 = w1 ^ w2;
        int w4 = w3 << 1;
        
        func1();  /* Call at outer BB end */
        
        result = w1 + w2 + w3 + w4;
    }
    
    return result;
}

/* ============================================
 * Main driver that calls all tests
 * ============================================ */
int main(void) {
    int total = 0;
    
    /* Seed computations with varying inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i*2, i*3, i*4);
        total += test_call_in_switch_case(i, i+1, i+2, i+3);
        total += test_call_between_complex_ops(i % 3, i * 100);
        total += test_nested_conditionals(i-5, i, i+10, i*2);
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be non-zero: %d)\n", total);
    
    return total != 0 ? 0 : 1;
}
