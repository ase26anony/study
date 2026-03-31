/* caller_save_test.c
 * Test program to trigger specific uncovered lines in GCC's caller-save.cc
 * Lines 905-913: Inserting instructions at end of basic blocks
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and optimization */
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

/* Test 1: Call at end of basic block before return */
NOINLINE int test_call_at_bb_end(int a, int b, int c, int d, int e, int f) {
    /* Create many live values that must be preserved across call */
    int live1 = a * b + c;
    int live2 = b * c + d;
    int live3 = c * d + e;
    int live4 = d * e + f;
    int live5 = e * f + a;
    int live6 = f * a + b;
    int live7 = (a ^ b) | (c & d);
    int live8 = (b ^ c) | (d & e);
    
    /* Complex condition to create basic block structure */
    if (a > b && c < d) {
        /* More computations to increase register pressure */
        live1 = live1 * 2 + live2;
        live2 = live2 * 3 + live3;
        live3 = live3 * 4 + live4;
        live4 = live4 * 5 + live5;
        
        /* Function call at end of basic block before return */
        func1();
        
        /* This return makes the call the last instruction in BB */
        return live1 + live2 + live3 + live4 + live5 + live6 + live7 + live8;
    } else {
        /* Alternative path with different computations */
        live5 = live5 * 6 + live6;
        live6 = live6 * 7 + live7;
        live7 = live7 * 8 + live8;
        live8 = live8 * 9 + live1;
        
        /* Another call at BB end */
        func2();
        
        return live5 + live6 + live7 + live8 + live1 + live2 + live3 + live4;
    }
}

/* Test 2: Call in switch case at BB end */
NOINLINE int test_call_in_switch_case(int x, int y, int z) {
    int result = 0;
    
    /* Create many live values */
    int val1 = x * y;
    int val2 = y * z;
    int val3 = z * x;
    int val4 = x + y + z;
    int val5 = x ^ y ^ z;
    int val6 = (x << 3) | (y << 2) | (z << 1);
    int val7 = ~x & ~y & ~z;
    int val8 = x * 11 + y * 13 + z * 17;
    
    switch (x % 4) {
        case 0:
            /* More computations */
            val1 = val1 + val2;
            val2 = val2 + val3;
            val3 = val3 + val4;
            /* Call at end of case BB before break */
            func1();
            result = val1 + val2 + val3;
            break;
            
        case 1:
            val4 = val4 * val5;
            val5 = val5 * val6;
            val6 = val6 * val7;
            /* Another call at BB end */
            func2();
            result = val4 + val5 + val6;
            break;
            
        case 2:
            val7 = val7 ^ val8;
            val8 = val8 ^ val1;
            val1 = val1 ^ val2;
            /* Volatile function pointer call */
            VOLATILE_CALL(func3);
            result = val7 + val8 + val1;
            break;
            
        default:
            val2 = val2 | val3;
            val3 = val3 | val4;
            val4 = val4 | val5;
            func4();
            result = val2 + val3 + val4;
            break;
    }
    
    /* Use all values to ensure they're live across calls */
    return result + val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8;
}

/* Test 3: Complex loop with call in middle */
NOINLINE int test_call_between_complex_ops(int n, int seed) {
    int i;
    /* Many accumulator variables */
    int acc1 = seed, acc2 = seed * 2, acc3 = seed * 3;
    int acc4 = seed * 4, acc5 = seed * 5, acc6 = seed * 6;
    int acc7 = seed * 7, acc8 = seed * 8, acc9 = seed * 9;
    int acc10 = seed * 10;
    
    /* Loop creates many live values */
    for (i = 0; i < n; i++) {
        acc1 = acc1 * 3 + i;
        acc2 = acc2 * 5 + i;
        acc3 = acc3 * 7 + i;
        acc4 = acc4 * 11 + i;
        acc5 = acc5 * 13 + i;
        
        /* Every 8 iterations, make a call with all values live */
        if ((i & 7) == 7) {
            /* All acc variables are live across this call */
            func1();
            
            /* More computations after call */
            acc6 = acc6 * 17 + acc1;
            acc7 = acc7 * 19 + acc2;
            acc8 = acc8 * 23 + acc3;
            acc9 = acc9 * 29 + acc4;
            acc10 = acc10 * 31 + acc5;
        } else {
            /* Different computations in other iterations */
            acc6 = acc6 ^ acc1;
            acc7 = acc7 ^ acc2;
            acc8 = acc8 ^ acc3;
            acc9 = acc9 ^ acc4;
            acc10 = acc10 ^ acc5;
        }
    }
    
    /* Use all accumulators to ensure they're live */
    return acc1 + acc2 + acc3 + acc4 + acc5 + 
           acc6 + acc7 + acc8 + acc9 + acc10;
}

/* Test 4: Nested conditionals with calls at BB ends */
NOINLINE int test_nested_conditionals(int a, int b, int c, int d) {
    int x1 = a + b, x2 = b + c, x3 = c + d, x4 = d + a;
    int y1 = a * b, y2 = b * c, y3 = c * d, y4 = d * a;
    int z1 = a ^ b, z2 = b ^ c, z3 = c ^ d, z4 = d ^ a;
    
    if (a > 0) {
        x1 = x1 * 2;
        x2 = x2 * 3;
        if (b > 0) {
            y1 = y1 + x1;
            y2 = y2 + x2;
            /* Call at end of inner BB */
            func1();
            /* Return makes previous call the BB end */
            return x1 + x2 + y1 + y2 + z1;
        } else {
            y3 = y3 + x3;
            y4 = y4 + x4;
            func2();
            return x3 + x4 + y3 + y4 + z2;
        }
    } else {
        x3 = x3 * 4;
        x4 = x4 * 5;
        if (c > 0) {
            z1 = z1 | y1;
            z2 = z2 | y2;
            VOLATILE_CALL(func3);
            return x1 + x2 + z1 + z2 + y3;
        } else {
            z3 = z3 | y3;
            z4 = z4 | y4;
            func4();
            return x3 + x4 + z3 + z4 + y4;
        }
    }
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int i, total = 0;
    
    /* Use command line args or defaults for variability */
    int base = argc > 1 ? atoi(argv[1]) : 12345;
    
    printf("Running caller-save edge case tests...\n");
    
    /* Run multiple iterations to increase coverage chance */
    for (i = 0; i < 10; i++) {
        int iter = base + i;
        
        total += test_call_at_bb_end(iter, iter+1, iter+2, iter+3, iter+4, iter+5);
        total += test_call_in_switch_case(iter, iter*2, iter*3);
        total += test_call_between_complex_ops(iter % 20 + 5, iter);
        total += test_nested_conditionals(iter, -iter, iter/2, -iter/2);
        
        /* Prevent loop unrolling from simplifying too much */
        if (total > 1000000) total = total % 1000000;
    }
    
    printf("Total result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return total != 0 ? 0 : 1;
}
