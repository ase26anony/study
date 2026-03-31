/* test-caller-save.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noclone, noipa))

/* Volatile globals to prevent dead code elimination */
volatile int global_counter = 0;
volatile int global_sink = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(int x) { global_sink = x; }
NOINLINE void func3(void) { /* empty */ }
NOINLINE int func4(int a, int b) { return a ^ b; }

/* Test 1: Call at basic block end before return */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across call */
    int a = x * 3 + 1;
    int b = y << 2;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = a + b;
    int f = c - d;
    int g = (x * y) | z;
    int h = (y * z) & 0xFFFF;
    
    /* Function call with all values live */
    func1();
    
    /* Use all values after call - forces them to be saved/restored */
    return a + b - c + d * e - f / (g + 1) + h;
}

/* Test 2: Call in switch case at basic block end */
NOINLINE int test_call_in_switch_case(int selector, int x, int y) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Many live values in this case */
            int v1 = x + 1;
            int v2 = y * 2;
            int v3 = v1 ^ v2;
            int v4 = (x << 3) | (y & 0xF);
            int v5 = v1 + v2 + v3 + v4;
            
            /* Call at end of basic block before break */
            func2(v5);
            
            /* This call makes the basic block end with the call */
            break;  /* Basic block ends with call, then break */
        }
        case 1: {
            int v6 = x * y;
            int v7 = x + y;
            int v8 = x - y;
            int v9 = x ^ y;
            
            func3();
            
            result = v6 + v7 + v8 + v9;
            break;
        }
        case 2: {
            /* More register pressure */
            int t1 = x * 3;
            int t2 = y * 5;
            int t3 = t1 + t2;
            int t4 = t1 - t2;
            int t5 = t1 ^ t2;
            int t6 = t3 * t4;
            int t7 = t5 | t6;
            
            func1();
            
            result = t7;
            break;
        }
        default:
            result = x + y;
    }
    
    return result;
}

/* Test 3: Complex loop creating register pressure, then call */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    /* Create many accumulator variables */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Unrolled loop to create many live values */
    for (int i = 0; i < iterations && i < 8; i++) {
        int val = seed + i * 17;
        
        /* Different computations for each accumulator */
        acc1 += val * 2;
        acc2 += val ^ 0x55;
        acc3 += val << (i & 3);
        acc4 += val >> 1;
        acc5 |= val;
        acc6 &= val | 1;
        acc7 ^= val * 3;
        acc8 = acc8 * 13 + val;
    }
    
    /* All 8 accumulators are live across this call */
    func3();
    
    /* Use all accumulators after call */
    return acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int a, int b, int c) {
    int result = 0;
    
    if (a > 0) {
        int x1 = a * 2 + 1;
        int x2 = b * 3 - 1;
        int x3 = c ^ 0xFF;
        int x4 = x1 + x2 + x3;
        
        if (b > 0) {
            int y1 = x1 * x2;
            int y2 = x3 << 2;
            int y3 = x4 & 0xFFFF;
            int y4 = y1 ^ y2 ^ y3;
            
            /* Call at end of inner if block */
            func2(y4);
            
            /* Basic block ends with call */
            result = y4;
        } else {
            int z1 = x1 | x2;
            int z2 = x3 & x4;
            int z3 = z1 * z2;
            
            func1();
            
            result = z3;
        }
    } else {
        int w1 = b + c;
        int w2 = b * c;
        int w3 = w1 ^ w2;
        int w4 = w3 * 7;
        
        func3();
        
        result = w4;
    }
    
    return result;
}

/* Test 5: Multiple consecutive calls with live values between them */
NOINLINE int test_multiple_calls(int x, int y) {
    /* First set of live values */
    int a = x + y;
    int b = x * y;
    int c = x ^ y;
    int d = x - y;
    
    /* First call - a,b,c,d must be saved */
    int r1 = func4(a, b);
    
    /* New values become live */
    int e = r1 * 2;
    int f = c + d;
    int g = a ^ r1;
    int h = b & 0xFF;
    
    /* Second call - e,f,g,h must be saved, plus maybe others */
    func2(e + f);
    
    /* Use all values */
    return a + b + c + d + e + f + g + h + r1;
}

/* Test 6: Call as last instruction before return in multiple paths */
NOINLINE int test_call_before_return(int cond1, int cond2, int x) {
    /* Force many live values */
    int v1 = x * 11;
    int v2 = x + 22;
    int v3 = x ^ 33;
    int v4 = x << 2;
    int v5 = x >> 1;
    
    if (cond1) {
        int t1 = v1 + v2;
        int t2 = v3 * v4;
        int t3 = t1 ^ t2;
        
        /* Call at end of if block before return */
        func1();
        
        return t3 + v5;  /* v5 is also live */
    } else if (cond2) {
        int u1 = v2 + v3;
        int u2 = v4 | v5;
        int u3 = u1 * u2;
        
        func2(u3);
        
        return u3 + v1;  /* v1 is live across call */
    } else {
        int w1 = v1 * v3;
        int w2 = v2 & v4;
        int w3 = w1 + w2;
        
        func3();
        
        return w3 + v5;  /* v5 is live across call */
    }
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_call_at_bb_end(i, i*2, i*3);
    }
    
    /* Run test 2 */
    total += test_call_in_switch_case(0, 100, 200);
    total += test_call_in_switch_case(1, 150, 250);
    total += test_call_in_switch_case(2, 200, 300);
    
    /* Run test 3 */
    total += test_call_between_complex_ops(8, 42);
    total += test_call_between_complex_ops(4, 123);
    
    /* Run test 4 */
    total += test_nested_conditionals(10, 20, 30);
    total += test_nested_conditionals(-5, 15, 25);
    
    /* Run test 5 */
    total += test_multiple_calls(7, 13);
    total += test_multiple_calls(23, 47);
    
    /* Run test 6 */
    total += test_call_before_return(1, 0, 99);
    total += test_call_before_return(0, 1, 88);
    total += test_call_before_return(0, 0, 77);
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
