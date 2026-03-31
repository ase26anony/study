/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting save/restore instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining and IPA optimizations */
#define NOINLINE __attribute__((noinline, noipa))

/* Volatile globals to prevent dead code elimination */
volatile int global_counter = 0;
volatile int global_sink = 0;

/* Non-inline functions that will be called */
NOINLINE void func1(void) { global_counter++; }
NOINLINE void func2(int x) { global_sink ^= x; }
NOINLINE void func3(void) { /* empty */ }
NOINLINE int func4(int a, int b) { return a ^ b; }

/* Test 1: Call at basic block end with many live values */
NOINLINE int test_call_at_bb_end(int x, int y, int z) {
    /* Create many live values that must survive across call */
    int a = x * 3 + 1;
    int b = y << 2;
    int c = z & 0xFF;
    int d = x ^ y ^ z;
    int e = x + y + z;
    int f = x * y - z;
    int g = (x << 3) | (y << 2) | z;
    int h = ~(x * y * z);
    
    /* Function call with all values live */
    func1();
    
    /* Use all live values after call - ensures they must be saved */
    return a + b - c + d ^ e | f & g + h;
}

/* Test 2: Call in switch case at basic block end */
NOINLINE int test_call_in_switch_case(int selector, int x, int y) {
    int result = 0;
    
    switch (selector & 3) {
        case 0: {
            /* Many live values in this case */
            int a = x + 1;
            int b = y * 2;
            int c = x ^ y;
            int d = ~x;
            int e = y << 1;
            int f = x * 3;
            
            /* Call at end of basic block before break */
            func2(a);
            
            /* This makes the call the BB_END before break */
            result = a + b + c + d + e + f;
            break;
        }
        case 1: {
            int a = x - y;
            int b = x | y;
            /* Call then immediate return - call is BB_END */
            func3();
            return a ^ b;
        }
        case 2: {
            /* Chain of computations with call in middle */
            int t1 = x * x;
            int t2 = y * y;
            int t3 = t1 + t2;
            int t4 = t1 - t2;
            int t5 = t3 ^ t4;
            
            func1();
            
            int t6 = t5 * 2;
            int t7 = t6 + x;
            result = t7;
            break;
        }
        default:
            result = x + y;
    }
    
    return result;
}

/* Test 3: Complex loop with call and many live values */
NOINLINE int test_call_between_complex_ops(int iterations, int seed) {
    int i;
    int live_values[8];  /* More values than available caller-saved regs */
    
    /* Compute many values before call */
    for (i = 0; i < 8; i++) {
        live_values[i] = seed * (i + 1) + (i * i);
    }
    
    /* Additional computations to increase register pressure */
    int a = live_values[0] ^ live_values[1];
    int b = live_values[2] | live_values[3];
    int c = live_values[4] & live_values[5];
    int d = live_values[6] + live_values[7];
    int e = a * b;
    int f = c - d;
    int g = e ^ f;
    int h = a + b + c + d;
    
    /* Non-inline call with many values live */
    int (*volatile fp)(int, int) = func4;
    int call_result = fp(g, h);
    
    /* Use all values after call */
    int sum = 0;
    for (i = 0; i < 8; i++) {
        sum += live_values[i];
    }
    
    return sum + a + b + c + d + e + f + g + h + call_result;
}

/* Test 4: Nested conditionals with calls at block ends */
NOINLINE int test_nested_conditionals(int x, int y, int z) {
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            /* First branch with many live values */
            int a = x * 2;
            int b = y * 3;
            int c = z * 4;
            int d = a + b;
            int e = b + c;
            int f = c + a;
            
            /* Call at end of this basic block */
            func1();
            
            /* Use values - ensures they're live across call */
            result = d + e + f;
        } else {
            /* Alternative branch, also with call at end */
            int a = x | y;
            int b = y & z;
            int c = x ^ z;
            
            func2(a);
            
            result = a - b + c;
        }
    } else {
        /* Outer else branch */
        int a = ~x;
        int b = ~y;
        int c = ~z;
        
        /* Multiple calls in sequence */
        func3();
        
        int d = a + b;
        int e = b + c;
        
        func1();
        
        result = d * e;
    }
    
    return result;
}

/* Test 5: Call with live values in different scopes */
NOINLINE int test_mixed_scopes(int x) {
    int outer1 = x + 100;
    int outer2 = x * 50;
    
    {
        /* Inner scope with its own values */
        int inner1 = outer1 + 1;
        int inner2 = outer2 - 1;
        int inner3 = inner1 ^ inner2;
        int inner4 = inner1 & inner2;
        
        /* Call with mix of inner and outer values live */
        func2(inner3);
        
        /* Use inner values */
        outer1 = inner3 + inner4;
    }
    
    {
        /* Another inner scope */
        int inner5 = outer1 * 2;
        int inner6 = outer2 / 2;
        
        /* Call at end of basic block */
        func3();
        
        return inner5 + inner6 + outer1 + outer2;
    }
}

/* Main driver that runs all tests */
int main(void) {
    int total = 0;
    int i;
    
    /* Run multiple iterations with different inputs */
    for (i = 0; i < 100; i++) {
        total += test_call_at_bb_end(i, i+1, i+2);
        total += test_call_in_switch_case(i, i*2, i*3);
        total += test_call_between_complex_ops(8, i+10);
        total += test_nested_conditionals(i-50, i, i+50);
        total += test_mixed_scopes(i*7);
    }
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
