/* caller-save-test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;

/* Non-inline functions that will force caller-save decisions */
__attribute__((noinline, noipa)) void non_inline_func1(void) {
    global_counter++;
}

__attribute__((noinline, noipa)) void non_inline_func2(int x) {
    global_sink = x;
}

__attribute__((noinline, noipa)) void non_inline_func3(void) {
    /* Empty function - just a call target */
}

/* Test 1: Function call at end of basic block before return */
__attribute__((noinline, noipa)) 
int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must survive across call */
    int live1 = a * 2 + 1;
    int live2 = b ^ 0x55AA55AA;
    int live3 = c + d * 3;
    int live4 = (e << 4) | (f & 0xF);
    int live5 = a + b + c + d;
    int live6 = (a * b) - (c * d);
    int live7 = e ^ f ^ 0x12345678;
    int live8 = (a << 2) | (b >> 2);
    
    /* Force register pressure - use all values before call */
    int temp1 = live1 + live2;
    int temp2 = live3 - live4;
    int temp3 = live5 * live6;
    int temp4 = live7 & live8;
    
    /* Call at what should be BB_END in one branch */
    if (temp1 > temp2) {
        /* More computations to ensure values stay in registers */
        live1 = temp3 + live1;
        live2 = temp4 ^ live2;
        live3 = live3 * 2;
        live4 = live4 | 0xFF;
        
        /* Critical: call as last instruction before return in this branch */
        non_inline_func1();
        
        /* This return makes the call the BB_END */
        return live1 + live2 + live3 + live4;
    } else {
        /* Alternative path without call */
        return temp1 + temp2 + temp3 + temp4;
    }
}

/* Test 2: Function call in switch case with break */
__attribute__((noinline, noipa))
int test_call_in_switch_case(int selector, int x1, int x2, int x3, int x4) {
    int result = 0;
    
    /* Create live values before switch */
    int val1 = x1 * x1 + 1;
    int val2 = x2 | x3;
    int val3 = (x4 << 3) ^ 0xDEADBEEF;
    int val4 = x1 + x2 + x3 + x4;
    int val5 = x1 * x2 * x3;
    int val6 = ~(x4) & 0x7FFFFFFF;
    
    switch (selector & 0x3) {
        case 0:
            /* Use values to keep them live */
            val1 = val1 + val2;
            val2 = val2 ^ val3;
            /* Call then break - call should be BB_END before break */
            non_inline_func2(val1);
            break;
            
        case 1:
            val3 = val3 * 2;
            val4 = val4 | val5;
            result = val3 + val4;
            break;
            
        case 2:
            /* Another call site */
            val5 = val5 + val6;
            val6 = val6 ^ val1;
            non_inline_func3();
            result = val5 - val6;
            break;
            
        default:
            result = val1 + val2 + val3 + val4 + val5 + val6;
            break;
    }
    
    /* Use all values after switch to ensure liveness across call */
    return result + val1 + val2 + val3 + val4 + val5 + val6;
}

/* Test 3: Complex loop with many live values across call */
__attribute__((noinline, noipa))
int test_call_between_complex_ops(int iterations, int seed) {
    int i;
    /* Many accumulator variables - all must be in registers */
    int acc1 = seed, acc2 = seed ^ 0x55555555;
    int acc3 = seed * 3, acc4 = seed + 0x12345678;
    int acc5 = ~seed, acc6 = seed << 1;
    int acc7 = seed >> 1, acc8 = seed | 0xF0F0F0F0;
    
    /* Loop creates register pressure */
    for (i = 0; i < iterations && i < 10; i++) {
        /* Modify all accumulators */
        acc1 = acc1 + i;
        acc2 = acc2 ^ (i * 2);
        acc3 = acc3 * (i + 1);
        acc4 = acc4 | (i << 4);
        acc5 = acc5 - i;
        acc6 = acc6 & ~i;
        acc7 = acc7 + (i * i);
        acc8 = acc8 ^ (acc1 + i);
    }
    
    /* Call with all accumulators live */
    non_inline_func1();
    
    /* Use all accumulators after call */
    int sum = acc1 + acc2 + acc3 + acc4;
    int product = acc5 * acc6;
    int xor_result = acc7 ^ acc8;
    
    return sum + product + xor_result;
}

/* Test 4: Multiple calls in different basic blocks */
__attribute__((noinline, noipa))
int test_multiple_calls_bb_end(int a, int b, int cond1, int cond2) {
    /* Create many live values */
    int x1 = a + b, x2 = a * b;
    int x3 = a ^ b, x4 = a - b;
    int x5 = (a << 4), x6 = (b >> 2);
    int x7 = a | b, x8 = a & b;
    
    /* Nested conditionals creating multiple BB_END opportunities */
    if (cond1) {
        x1 = x1 * 2;
        x2 = x2 + x3;
        if (cond2) {
            x3 = x3 ^ x4;
            x4 = x4 | x5;
            /* Call at end of inner if block */
            non_inline_func2(x1);
            /* Return makes this BB_END */
            return x1 + x2 + x3 + x4;
        } else {
            x5 = x5 << 1;
            x6 = x6 >> 1;
            /* Another call at end of else block */
            non_inline_func3();
            return x5 + x6 + x7 + x8;
        }
    } else {
        x7 = x7 & 0xFF;
        x8 = x8 | 0xAA;
        /* Call at end of outer else block */
        non_inline_func1();
        return x7 * x8;
    }
}

/* Test 5: Call through volatile function pointer */
__attribute__((noinline, noipa))
int test_volatile_call(int a, int b, int c) {
    /* Volatile function pointer ensures call isn't optimized */
    void (*volatile fp)(void) = non_inline_func3;
    
    /* Many live values */
    int v1 = a * 3, v2 = b + 5;
    int v3 = c ^ 0x99, v4 = a + b + c;
    int v5 = (a << b) & 0xFF, v6 = (c >> 2) | 0x11;
    int v7 = a * b * c, v8 = ~(a + b);
    
    /* Use values before call */
    int t1 = v1 + v2 + v3;
    int t2 = v4 * v5 - v6;
    
    /* Volatile call - compiler can't analyze it */
    fp();
    
    /* Use all values after call */
    return t1 + t2 + v7 + v8 + v1 + v2 + v3 + v4 + v5 + v6;
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use command line args or defaults for variability */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run test 1 multiple times with different inputs */
    total += test_call_at_bb_end(base, base+1, base+2, base+3, base+4, base+5);
    total += test_call_at_bb_end(base+1, base+2, base+3, base+4, base+5, base+6);
    
    /* Test 2 with switch */
    total += test_call_in_switch_case(base & 3, base, base+10, base+20, base+30);
    total += test_call_in_switch_case((base+1) & 3, base+5, base+15, base+25, base+35);
    
    /* Test 3 with loop */
    total += test_call_between_complex_ops(5, base);
    total += test_call_between_complex_ops(8, base+100);
    
    /* Test 4 with nested conditionals */
    total += test_multiple_calls_bb_end(base, base+50, 1, 0);
    total += test_multiple_calls_bb_end(base+10, base+60, 0, 1);
    
    /* Test 5 with volatile call */
    total += test_volatile_call(base, base+7, base+14);
    
    printf("Total result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
