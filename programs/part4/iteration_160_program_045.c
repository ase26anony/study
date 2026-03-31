/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets instruction insertion at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile global to prevent dead code elimination */
volatile int global_sink = 0;

/* Non-inline functions that will force caller-save decisions */
__attribute__((noinline, noipa)) void func1(void) {
    /* Empty function - just a call target */
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) void func2(int x) {
    /* Use parameter to prevent optimization */
    global_sink += x;
    asm volatile("" : : : "memory");
}

__attribute__((noinline, noipa)) void func3(void) {
    /* Another call target */
    asm volatile("" : : : "memory");
}

/* Test 1: Call at end of basic block before return */
__attribute__((noinline, noipa)) 
int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int v1 = a * 3 + 1;
    int v2 = b << 2;
    int v3 = c ^ 0x55AA55AA;
    int v4 = d + e * 2;
    int v5 = f & 0x0F0F0F0F;
    int v6 = (a + b) * (c - d);
    int v7 = e | f;
    int v8 = (a ^ b) + (c ^ d);
    
    /* Function call with all values live */
    func1();
    
    /* Use all values after call - forces them to be saved/restored */
    return v1 + v2 - v3 + v4 * v5 + v6 / 7 + v7 ^ v8;
}

/* Test 2: Call in switch case at basic block end */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    switch (x % 4) {
        case 0: {
            /* Many live values in this case */
            int l1 = y * 2 + 1;
            int l2 = z << 3;
            int l3 = y ^ z;
            int l4 = (y + z) * 2;
            int l5 = y & 0xFF;
            int l6 = z | 0xAA;
            
            /* Call at end of basic block before break */
            func2(l1);
            
            /* Use values after call */
            result = l1 + l2 - l3 + l4 * l5 + l6;
            break;
        }
        case 1: {
            int m1 = y + z;
            int m2 = y - z;
            func3();
            result = m1 * m2;
            break;
        }
        case 2: {
            int n1 = y * y;
            int n2 = z * z;
            func1();
            result = n1 + n2;
            break;
        }
        default: {
            int o1 = y / 2;
            int o2 = z / 3;
            func2(o1);
            result = o1 - o2;
            break;
        }
    }
    
    return result;
}

/* Test 3: Call between complex operations with loop-generated values */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int seed) {
    /* Generate many values in a loop that must live across call */
    int values[12];
    
    /* Compute many independent values */
    for (int i = 0; i < 12; i++) {
        values[i] = seed * (i + 1) + (i * i);
    }
    
    /* Additional computations to increase register pressure */
    int a = values[0] * 3 + values[1];
    int b = values[2] << values[3] % 8;
    int c = values[4] ^ values[5];
    int d = values[6] + values[7] * 2;
    int e = values[8] & values[9];
    int f = values[10] | values[11];
    int g = a + b - c;
    int h = d * e / 7;
    int i = f ^ 0x12345678;
    int j = (a ^ b) + (c ^ d);
    int k = e << 2;
    int l = g * h + i;
    
    /* Critical call with many live values */
    void (*volatile fp)(void) = func1;
    fp();  /* Volatile function pointer call */
    
    /* Use all computed values after the call */
    return a + b - c + d * e + f / 3 + g ^ h + i - j + k * l;
}

/* Test 4: Nested conditionals creating multiple BB ends with calls */
__attribute__((noinline, noipa))
int test_nested_conditionals(int x, int y, int z, int w) {
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            /* Many live values in this path */
            int t1 = x * y + 1;
            int t2 = z << 2;
            int t3 = w ^ 0x55;
            int t4 = (x + y) * z;
            int t5 = w & 0x0F;
            int t6 = t1 + t2;
            int t7 = t3 * t4;
            
            /* Call at BB end before return */
            func2(t1);
            
            /* This return makes the call the BB end */
            return t5 + t6 - t7;
        } else {
            int u1 = x + z;
            int u2 = y * w;
            int u3 = u1 ^ u2;
            func3();
            result = u3;
        }
    } else {
        int v1 = y + w;
        int v2 = z * 2;
        func1();
        result = v1 - v2;
    }
    
    return result;
}

/* Test 5: Call in loop with live values across iteration */
__attribute__((noinline, noipa))
int test_call_in_loop(int iterations) {
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Values computed in loop that must survive call */
        int a = i * 3 + 1;
        int b = (i + 1) << 2;
        int c = i ^ 0xAA;
        int d = a + b * 2;
        int e = c & 0x0F;
        
        /* Call in middle of loop body */
        if (i % 3 == 0) {
            func1();
        } else if (i % 3 == 1) {
            func2(a);
        } else {
            func3();
        }
        
        /* Use values after call */
        acc += a + b - c + d * e;
    }
    
    return acc;
}

int main(void) {
    int total = 0;
    
    /* Run all tests with varying inputs */
    total += test_call_at_bb_end(1, 2, 3, 4, 5, 6);
    total += test_call_in_switch_case(10, 20, 30);
    total += test_call_between_complex_ops(42);
    total += test_nested_conditionals(5, -3, 7, 9);
    total += test_call_in_loop(8);
    
    /* Also test with different inputs to explore different paths */
    total += test_call_at_bb_end(7, 8, 9, 10, 11, 12);
    total += test_call_in_switch_case(3, 40, 50);
    total += test_call_between_complex_ops(100);
    total += test_nested_conditionals(-2, 6, 8, 10);
    total += test_call_in_loop(5);
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}
